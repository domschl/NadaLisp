#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Installer
"""
import os
import sys
import json
import argparse
from jupyter_client.kernelspec import KernelSpecManager

def install_kernel(user=True, prefix=None):
    """Install the NadaLisp kernel"""
    
    # Get the directory of this script
    kernel_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Create the kernel spec
    kernel_spec = {
        "argv": [
            sys.executable,
            os.path.join(kernel_dir, "nada_kernel.py"),
            "-f",
            "{connection_file}"
        ],
        "display_name": "NadaLisp",
        "language": "scheme",
        "name": "nada",
        "metadata": {
            "debugger": False
        }
    }
    
    # Create a temporary directory to write the kernel spec
    import tempfile
    import shutil
    temp_dir = tempfile.mkdtemp()
    
    try:
        # Write the kernel.json file
        with open(os.path.join(temp_dir, "kernel.json"), "w") as f:
            json.dump(kernel_spec, f, indent=2)
        
        # Copy the Python files
        for file in ["kernel.py", "nada_kernel.py"]:
            shutil.copy(os.path.join(kernel_dir, file), os.path.join(temp_dir, file))
        
        # Install the kernel
        kernel_spec_manager = KernelSpecManager()
        dest = kernel_spec_manager.install_kernel_spec(
            temp_dir, "nada", user=user, prefix=prefix
        )
        
        print(f"NadaLisp kernel installed in {dest}")
        return 0
    finally:
        # Clean up the temporary directory
        shutil.rmtree(temp_dir)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Install the NadaLisp kernel")
    parser.add_argument("--user", action="store_true", help="Install for current user only")
    parser.add_argument("--prefix", help="Installation prefix")
    
    args = parser.parse_args()
    sys.exit(install_kernel(user=args.user, prefix=args.prefix))
