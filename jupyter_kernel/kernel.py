#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Implementation
"""
import sys
import json
import re
import ctypes
import tempfile
import threading
import time
import traceback
import io
import os
from contextlib import redirect_stdout
from ipykernel.kernelbase import Kernel
from ctypes import cdll, CDLL, c_void_p, c_int, c_char_p, c_buffer

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
            # Split code into separate expressions
            expressions = self.split_expressions(code)
            self.log.info(f"Split code into {len(expressions)} expressions")
            
            # Execute each expression and keep only the final result
            final_result = None

            for idx, expr in enumerate(expressions):
                self.log.info(f"Executing expression {idx+1}/{len(expressions)}: {expr}")
                
                # Convert Python string to C string
                c_code = expr.encode('utf-8')
                
                # Use a thread-based timeout instead of signals
                result_ptr = [None]
                error = [None]
                
                def evaluation_thread():
                    try:
                        # Clear the Jupyter output buffer before evaluation
                        self.lib.nada_jupyter_clear_buffer()
                        
                        try:
                            # Parse and evaluate the input code
                            self.log.info(f"Calling nada_parse_eval_multi for: {expr}")
                            value_ptr = self.lib.nada_parse_eval_multi(c_code, self.env)
                            self.log.info(f"Result pointer: {value_ptr}")
                            
                            # Check if it's an error
                            if value_ptr:
                                self.lib.nada_is_error.argtypes = [c_void_p]
                                self.lib.nada_is_error.restype = c_int
                                
                                if self.lib.nada_is_error(value_ptr):
                                    # Get the error message
                                    string_ptr = self.lib.nada_value_to_string(value_ptr)
                                    error_msg = ctypes.string_at(string_ptr).decode('utf-8')
                                    
                                    # Free resources
                                    if hasattr(self.lib, 'nada_free_string'):
                                        self.lib.nada_free_string(string_ptr)
                                    self.lib.nada_free(value_ptr)
                                    
                                    # Store error
                                    error[0] = RuntimeError(error_msg)
                                    return
                                
                            # Get captured output from buffer
                            output_ptr = self.lib.nada_jupyter_get_buffer()
                            if output_ptr:
                                output_text = ctypes.string_at(output_ptr).decode('utf-8')
                                
                                # Get the output type
                                output_type = self.lib.nada_jupyter_get_output_type()
                                
                                # Send captured output to frontend with appropriate MIME type
                                if output_text:
                                    if output_type == 1:  # NADA_OUTPUT_MARKDOWN
                                        self.send_response(self.iopub_socket, 'display_data', {
                                            'data': {
                                                'text/markdown': output_text
                                            },
                                            'metadata': {}
                                        })
                                    elif output_type == 2:  # NADA_OUTPUT_HTML
                                        self.send_response(self.iopub_socket, 'display_data', {
                                            'data': {
                                                'text/html': output_text
                                            },
                                            'metadata': {}
                                        })
                                    else:  # Default to plain text
                                        self.send_response(self.iopub_socket, 'stream', {
                                            'name': 'stdout',
                                            'text': output_text
                                        })
                            
                            # Convert result to string
                            if value_ptr:
                                string_ptr = self.lib.nada_value_to_string(value_ptr)
                                if string_ptr:
                                    result_ptr[0] = ctypes.string_at(string_ptr).decode('utf-8')
                                    self.log.info(f"Got result: {result_ptr[0]}")
                                    
                                    # Free the string if needed
                                    if hasattr(self.lib, 'nada_free_string'):
                                        self.lib.nada_free_string(string_ptr)
                                
                                # Free the result value
                                self.lib.nada_free(value_ptr)
                            
                        except Exception as e:
                            error[0] = e
                            self.log.error(f"Exception in evaluation: {str(e)}")
                            self.log.error(traceback.format_exc())
                            
                    except Exception as e:
                        error[0] = e
                        self.log.error(f"Exception in evaluation thread: {str(e)}")
                        self.log.error(traceback.format_exc())
                
                # Start the evaluation thread...
                thread = threading.Thread(target=evaluation_thread)
                thread.daemon = True
                thread.start()
                
                # Wait for the thread to finish with timeout
                timeout_seconds = 10
                thread.join(timeout_seconds)
                
                # Check if the thread is still alive (timeout occurred)
                if thread.is_alive():
                    error_content = {
                        'ename': 'Timeout Error',
                        'evalue': f"NadaLisp evaluation timed out after {timeout_seconds} seconds",
                        'traceback': [f"Execution timed out after {timeout_seconds} seconds. The NadaLisp interpreter may be stuck."],
                    }
                    self.log.error(f"Timeout after {timeout_seconds} seconds")
                    self.send_response(self.iopub_socket, 'error', error_content)
                    return {
                        'status': 'error',
                        'execution_count': self.execution_count,
                        'ename': 'Timeout Error', 
                        'evalue': f"Execution timed out after {timeout_seconds} seconds",
                        'traceback': [f"The NadaLisp interpreter may be stuck or the C library may have crashed"],
                    }
                
                # Check if there was an error in the thread
                if error[0] is not None:
                    error_content = {
                        'ename': type(error[0]).__name__,
                        'evalue': str(error[0]),
                        'traceback': [str(error[0])],
                    }
                    self.send_response(self.iopub_socket, 'error', error_content)
                    return {
                        'status': 'error',
                        'execution_count': self.execution_count,
                        'ename': type(error[0]).__name__,
                        'evalue': str(error[0]),
                        'traceback': [str(error[0])],
                    }
                
            # Return the final result...
            self.log.info(f"Processing result_ptr: {result_ptr[0]}")
            if result_ptr[0]:
                if not silent:
                    stream_content = {'name': 'stdout', 'text': str(result_ptr[0])}
                    self.send_response(self.iopub_socket, 'stream', stream_content)
                
            return {
                'status': 'ok',
                'execution_count': self.execution_count,
                'payload': [],
                'user_expressions': {},
            }
            
        except Exception as e:
            self.log.error(f"Error in do_execute: {str(e)}")
            self.log.error(traceback.format_exc())
            error_content = {
                'ename': type(e).__name__,
                'evalue': str(e),
                'traceback': traceback.format_exc().split('\n'),
            }
            self.send_response(self.iopub_socket, 'error', error_content)
            return {
                'status': 'error',
                'execution_count': self.execution_count,
                'ename': type(e).__name__,
                'evalue': str(e),
                'traceback': traceback.format_exc().split('\n'),
            }

    def split_expressions(self, code):
        """Split code into separate scheme expressions based on balanced parentheses"""
        expressions = []
        current_expr = ""
        paren_count = 0
        in_string = False
        
        # Skip initial whitespace
        code = code.strip()
        if not code:
            return []
        
        for i, char in enumerate(code):
            # Handle string literals
            if char == '"' and (i == 0 or code[i-1] != '\\'):
                in_string = not in_string
                
            if not in_string:
                if char == '(':
                    paren_count += 1
                elif char == ')':
                    paren_count -= 1
                    
            current_expr += char
            
            # When we've found a complete top-level expression
            if paren_count == 0 and current_expr.strip() and not in_string:
                # If expression starts with (, we have a complete S-expression
                if current_expr.strip()[0] == '(':
                    expressions.append(current_expr.strip())
                    current_expr = ""
                # Otherwise, it might be a non-parenthesized expression
                elif i == len(code) - 1 or code[i+1].isspace():
                    expressions.append(current_expr.strip())
                    current_expr = ""
        
        # Add any remaining expression
        if current_expr.strip():
            expressions.append(current_expr.strip())
            
        return expressions
    
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
