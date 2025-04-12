#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Launcher
"""
from ipykernel.kernelapp import IPKernelApp
from kernel import NadaKernel

if __name__ == "__main__":
    IPKernelApp.launch_instance(kernel_class=NadaKernel)
