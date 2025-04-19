[![CMake Test](https://github.com/domschl/NadaLisp/actions/workflows/cmake-test.yml/badge.svg)](https://github.com/domschl/NadaLisp/actions/workflows/cmake-test.yml)

# NadaLisp Project

It's Scheme implementation implemented by Copilot using Claude Sonnet 3.7 Thinking and all of it's collegues. Human did only little hand-holding and debris removal...

Since the language is in flux, have a look at `tests/lisp_tests` for the current language elements. `nadalib` contains a standard library.

## Build

Requires: `cmake`, `ninja`, `readline`

### One time preparation

```bash
git clone https://github.com/domschl/NadaLisp
cd NadaLisp
mkdir build
```

### (Re-)build

In `NadaLisp/build`:

```bash
cmake -G Ninja ..
ninja
```

### Execute

```bash
NadaLisp/build/repl/NadaLisp
```

## Tests

In `NadaLisp/build`:

```bash
ctest -L LispTests         # Just functional tests
ctest -L MemoryTests       # All memory tests
ctest -L LispMemoryTests   # Memory tests for regular Lisp files only
ctest -L JupyterKernel     # Test Jupyter kernel only
```

### Known problems

- nested function definitions with recursion in the nested definition cause leaks.

## Jupyter Kernel for NadaLisp

NadaLisp comes with a Jupyter kernel that allows you to use NadaLisp in Jupyter notebooks.

### Prerequisites

- Python 3.8 or higher
- uv (Python package manager)

### Building and Installing the Kernel

In `NadaLisp/build`:

```bash
# Build the NadaLisp project including the Jupyter kernel
cmake -G Ninja ..
ninja

# Set up the Python virtual environment and install required packages
ninja setup_venv

# Install the NadaLisp kernel
ninja install_kernel
```

### Running JupyterLab with NadaLisp

After installing the kernel, you can start JupyterLab:

```bash
ninja run_jupyterlab
```

This will open JupyterLab in your browser. To create a new NadaLisp notebook:

1. Click on the "+" button in the file browser to open the launcher
2. Select "NadaLisp" from the notebook options
3. You can now write and execute NadaLisp code in the notebook

### Example Usage

In a notebook cell, you can write NadaLisp code like:

```scheme
(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(factorial 5)
```

Execute the cell to see the result.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.
