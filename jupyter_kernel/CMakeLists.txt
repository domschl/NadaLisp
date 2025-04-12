# CMakeLists.txt for NadaLisp Jupyter kernel

# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Set up kernel installation directory
set(KERNEL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/kernel)
set(KERNEL_DIR ${KERNEL_INSTALL_DIR}/nada_kernel)

# Create venv for kernel development (using uv)
add_custom_command(
    OUTPUT ${KERNEL_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${KERNEL_INSTALL_DIR}
    COMMENT "Creating kernel installation directory"
)

# Create a target for installing the Python kernel
add_custom_target(
    jupyter_kernel ALL
    DEPENDS ${KERNEL_DIR}
    COMMENT "Setting up NadaLisp Jupyter kernel"
)

# Copy kernel files
file(GLOB KERNEL_PYTHON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
add_custom_command(
    TARGET jupyter_kernel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} ${KERNEL_DIR}
    COMMENT "Copying kernel files"
)

# Copy pyproject.toml to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml
    ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml
    COPYONLY
)

# Set up Python virtualenv with uv and install dependencies
add_custom_target(
    setup_venv
    COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && uv venv
    COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && uv pip install jupyter_client ipykernel jupyterlab
    COMMAND ${CMAKE_COMMAND} -E echo "Virtual environment set up at ${CMAKE_CURRENT_BINARY_DIR}/.venv"
    COMMENT "Setting up Python virtual environment with uv"
)

# Target to install the kernel
add_custom_target(
    install_kernel
    COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${CMAKE_CURRENT_BINARY_DIR}/.venv/bin/python ${CMAKE_CURRENT_SOURCE_DIR}/install_kernel.py --user
    COMMENT "Installing NadaLisp kernel using custom installer"
    DEPENDS setup_venv jupyter_kernel
)

# Run JupyterLab target
add_custom_target(
    run_jupyterlab
    COMMAND ${CMAKE_COMMAND} -E echo "Starting JupyterLab..."
    COMMAND ${CMAKE_COMMAND} -E env PATH=${CMAKE_CURRENT_BINARY_DIR}/.venv/bin:$ENV{PATH} ${CMAKE_CURRENT_BINARY_DIR}/.venv/bin/jupyter lab
    DEPENDS install_kernel
    COMMENT "Running JupyterLab with NadaLisp kernel"
)

# Add test for kernel
add_test(
    NAME kernel_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/.venv/bin/python ${CMAKE_CURRENT_SOURCE_DIR}/test_kernel.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install kernel
install(
    DIRECTORY ${KERNEL_DIR}/
    DESTINATION share/jupyter/kernels/nada
)
