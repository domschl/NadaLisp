#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Implementation
"""
import sys
import json
import traceback
import os
from ipykernel.kernelbase import Kernel
from ctypes import CDLL, c_void_p, c_int, c_char_p

# The Kernel base class already provides a self.log logger

class NadaKernel(Kernel):
    implementation = 'nada'
    implementation_version = '0.1'
    language = 'scheme'  # NadaLisp is a Scheme implementation
    language_version = '0.1'
    language_info = {
        'name': 'nada',
        'mimetype': 'text/x-scheme',
        'file_extension': '.scm',
        'pygments_lexer': 'scheme',
    }
    banner = "NadaLisp - A Scheme implementation for Jupyter"
    
    def __init__(self, **kwargs):
        super(NadaKernel, self).__init__(**kwargs)
        self.log.info("NadaLisp kernel starting up")
        self._load_nada_library()
        self._init_nada_env()
        
    def _load_nada_library(self):
        """Load the NadaLisp shared library"""
        # Determine the appropriate library extension based on OS
        if sys.platform.startswith('darwin'):
            lib_ext = '.dylib'
        elif sys.platform.startswith('linux'):
            lib_ext = '.so'
        elif sys.platform.startswith('win'):
            lib_ext = '.dll'
        else:
            lib_ext = '.so'  # Default to .so for unknown platforms
            
        lib_name = f"libnada_shared{lib_ext}"
        self.log.info(f"Looking for library with extension: {lib_ext}")
        
        # Get the directory of the current file
        current_dir = os.path.dirname(os.path.abspath(__file__))
        
        # Try to find the library in common locations
        lib_paths = [
            # Standard locations
            os.path.join(current_dir, lib_name),
            os.path.expanduser(f"~/Codeberg/NadaLisp/build/lib/{lib_name}"),
            f"/usr/local/lib/{lib_name}",
            f"/usr/lib/{lib_name}",
            
            # GitHub Actions specific paths
            os.path.join(os.path.dirname(current_dir), "build", "lib", lib_name),
            os.path.join(os.path.dirname(current_dir), "lib", lib_name),
            
            # More build directory possibilities
            os.path.abspath(os.path.join(current_dir, "..", "build", "lib", lib_name)),
            os.path.abspath(os.path.join(current_dir, "..", "lib", lib_name)),
            os.path.abspath(os.path.join(current_dir, "..", "..", "lib", lib_name)),
            os.path.abspath(os.path.join(current_dir, "..", "..", "build", "lib", lib_name))
        ]
        
        # Environment variable override (useful for CI)
        if 'NADA_LIBRARY_PATH' in os.environ:
            lib_path = os.environ['NADA_LIBRARY_PATH']
            self.log.info(f"Using library path from environment: {lib_path}")
            lib_paths.insert(0, lib_path)
        
        self.log.info(f"Searching for NadaLisp library in: {lib_paths}")
        
        for lib_path in lib_paths:
            self.log.info(f"Checking if {lib_path} exists: {os.path.exists(lib_path)}")
            if os.path.exists(lib_path):
                try:
                    self.log.info(f"Found library at {lib_path}, attempting to load...")
                    self.lib = CDLL(lib_path)
                    self.log.info(f"Successfully loaded NadaLisp library from {lib_path}")
                    break
                except Exception as e:
                    self.log.error(f"Failed to load library from {lib_path}: {e}")
        else:
            # Do a directory listing to help debug
            self.log.error("Library not found. Listing potential directories:")
            for path in lib_paths:
                dir_path = os.path.dirname(path)
                if os.path.exists(dir_path):
                    self.log.error(f"Contents of {dir_path}:")
                    for file in os.listdir(dir_path):
                        self.log.error(f"  {file}")
                else:
                    self.log.error(f"Directory {dir_path} does not exist")
                    
            error_msg = "Could not find the NadaLisp shared library. Searched paths: " + ", ".join(lib_paths)
            self.log.error(error_msg)
            raise RuntimeError(error_msg)
        
        # Define function prototypes - using actual exported function names from the library
        self.lib.nada_create_standard_env.restype = c_void_p
        self.lib.nada_cleanup_env.argtypes = [c_void_p]
        
        # Add function prototype for loading standard libraries
        self.lib.nada_load_libraries.argtypes = [c_void_p]
        self.lib.nada_load_libraries.restype = None
        
        # Update the nada_eval prototype to correctly match its signature
        self.lib.nada_eval.argtypes = [c_void_p, c_void_p]
        self.lib.nada_eval.restype = c_void_p
        
        # Add nada_parse_eval_multi to process multiple expressions at once
        self.lib.nada_parse_eval_multi.argtypes = [c_char_p, c_void_p]
        self.lib.nada_parse_eval_multi.restype = c_void_p
        
        # Existing prototypes
        self.lib.nada_parse.argtypes = [c_char_p]
        self.lib.nada_parse.restype = c_void_p
        
        # Add nada_value_to_string to get string representation of NadaValue
        self.lib.nada_value_to_string.argtypes = [c_void_p]
        self.lib.nada_value_to_string.restype = c_char_p

        # Add to the function prototype definitions
        self.lib.nada_free.argtypes = [c_void_p]
        self.lib.nada_free.restype = None

        # Add if your library has a separate function for freeing strings
        if hasattr(self.lib, 'nada_free_string'):
            self.lib.nada_free_string.argtypes = [c_char_p]
            self.lib.nada_free_string.restype = None
        
        # Add new Jupyter output function prototypes
        self.lib.nada_jupyter_use_output.argtypes = []
        self.lib.nada_jupyter_use_output.restype = None
        
        self.lib.nada_jupyter_get_buffer.argtypes = []
        self.lib.nada_jupyter_get_buffer.restype = c_char_p
        
        self.lib.nada_jupyter_clear_buffer.argtypes = []
        self.lib.nada_jupyter_clear_buffer.restype = None
        
        self.lib.nada_jupyter_cleanup.argtypes = []
        self.lib.nada_jupyter_cleanup.restype = None

        # Add these function prototypes
        self.lib.nada_jupyter_get_output_type.argtypes = []
        self.lib.nada_jupyter_get_output_type.restype = c_int

        # Add function prototype for checking if a value is an error
        self.lib.nada_is_error.argtypes = [c_void_p]
        self.lib.nada_is_error.restype = c_int

        self.log.info("Function prototypes defined successfully")
        
    def _init_nada_env(self):
        """Initialize the NadaLisp environment"""
        self.log.info("Initializing NadaLisp environment...")
        self.env = self.lib.nada_create_standard_env()
        if not self.env:
            error_msg = "Failed to initialize NadaLisp environment"
            self.log.error(error_msg)
            raise RuntimeError(error_msg)
            
        # Initialize Jupyter output
        self.lib.nada_jupyter_use_output()
        self.log.info("NadaLisp Jupyter output initialized")
        
        # Load standard libraries (just like the REPL does)
        self.log.info("Loading standard libraries...")
        self.lib.nada_load_libraries(self.env)
        self.log.info("Standard libraries loaded")
        
        self.log.info(f"NadaLisp environment initialized, handle: {self.env}")
        self.log.info("NadaLisp environment initialized")
        
    def do_execute(self, code, silent, store_history=True, user_expressions=None, allow_stdin=False):
        """Execute the user's code"""
        if not code.strip():
            return {
                'status': 'ok',
                'execution_count': self.execution_count,
                'payload': [],
                'user_expressions': {},
            }
        
        try:
            # Convert Python string to C string
            c_code = code.encode('utf-8')
            value_ptr = None
            result_str = "nil"
            error_occurred = False
            error_msg = ""
            
            # Clear the Jupyter output buffer before evaluation
            self.lib.nada_jupyter_clear_buffer()
            
            try:
                # Parse and evaluate all input code at once
                self.log.info(f"Calling nada_parse_eval_multi for all expressions")
                value_ptr = self.lib.nada_parse_eval_multi(c_code, self.env)
                self.log.info(f"Result pointer: {value_ptr}")
                
                # Check if the result is an error
                if value_ptr and self.lib.nada_is_error(value_ptr):
                    # It's an error - get the error message and set error flag
                    error_occurred = True
                    result_str = self.lib.nada_value_to_string(value_ptr).decode('utf-8', errors='replace')
                    
                    # Don't add "Error: " if it already exists
                    if result_str.startswith("Error:"):
                        error_msg = result_str
                    else:
                        error_msg = f"Error: {result_str}"
                        
                    self.log.error(f"Evaluation error: {result_str}, is_error={self.lib.nada_is_error(value_ptr)}")
                else:
                    # Convert the result to a string
                    if value_ptr:
                        result_str = self.lib.nada_value_to_string(value_ptr).decode('utf-8', errors='replace')
            finally:
                # Always free the value pointer to prevent memory leaks
                if value_ptr:
                    self.lib.nada_free(value_ptr)
            
            # Handle errors or display results
            if error_occurred:
                self.log.error(f"Sending error to client: {error_msg}")
                
                # First send to stderr so it appears in the output area
                self.send_response(self.iopub_socket, 'stream', {
                    'name': 'stderr',
                    'text': error_msg
                })
                
                # Then return error status with traceback for the error area
                return {
                    'status': 'error',
                    'execution_count': self.execution_count,
                    'ename': 'NadaLispError',
                    'evalue': result_str,
                    'traceback': [error_msg],
                }
            else:
                # Only prepare JSON for non-error results
                payload = {
                    'data': {'text/plain': result_str},
                    'metadata': {},
                    'execution_count': self.execution_count,
                }
                
                # Send the output to the client
                self.send_response(self.iopub_socket, 'execute_result', payload)
                
                return {
                    'status': 'ok',
                    'execution_count': self.execution_count,
                    'payload': [],
                    'user_expressions': {},
                }
        
        except Exception as e:
            self.log.error(f"Exception in do_execute: {str(e)}")
            self.log.error(traceback.format_exc())
            return {
                'status': 'error',
                'execution_count': self.execution_count,
                'ename': 'Exception',
                'evalue': str(e),
                'traceback': traceback.format_exc().splitlines(),
            }

    def do_shutdown(self, restart):
        """Clean up resources when the kernel is shut down"""
        if hasattr(self, 'env') and self.env:
            self.log.info("Releasing NadaLisp environment...")
            
            # Clean up Jupyter output system
            self.lib.nada_jupyter_cleanup()
            self.log.info("NadaLisp Jupyter output cleanup complete")
            
            self.lib.nada_cleanup_env(self.env)
            self.log.info("NadaLisp environment released")
        return {'status': 'ok', 'restart': restart}
