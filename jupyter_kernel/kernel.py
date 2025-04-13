#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Implementation
"""
import os
import sys
import json
import re
import ctypes
import tempfile
import threading
import time
import traceback
from ipykernel.kernelbase import Kernel
from ctypes import c_char_p, c_void_p, c_int, CDLL, POINTER, byref, Structure

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
        
        self.log.info("Function prototypes defined successfully")
        
    def _init_nada_env(self):
        """Initialize the NadaLisp environment"""
        self.log.info("Initializing NadaLisp environment...")
        self.env = self.lib.nada_create_standard_env()
        if not self.env:
            error_msg = "Failed to initialize NadaLisp environment"
            self.log.error(error_msg)
            raise RuntimeError(error_msg)
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
            # Split code into separate expressions by detecting top-level parentheses
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
                        # Parse and evaluate the input code directly with nada_parse_eval_multi
                        self.log.info(f"Parsing and evaluating expression: {expr}")
                        value_ptr = self.lib.nada_parse_eval_multi(c_code, self.env)
                        
                        self.log.info(f"nada_parse_eval_multi returned: {value_ptr}")
                        
                        if not value_ptr:
                            error[0] = RuntimeError("Evaluation returned NULL")
                            self.log.error("Evaluation returned NULL")
                            return
                            
                        # Convert to string only for the last expression
                        if idx == len(expressions) - 1:
                            self.log.info("Converting result to string...")
                            string_ptr = self.lib.nada_value_to_string(value_ptr)
                            
                            # Free the result value
                            self.lib.nada_free(value_ptr)
                            
                            if not string_ptr:
                                error[0] = RuntimeError("Failed to convert result to string")
                                self.log.error("Failed to convert result to string")
                                return
                                
                            # Convert C string result to Python string
                            result_str = ctypes.string_at(string_ptr).decode('utf-8')
                            self.log.info(f"Result string: {result_str}")
                            
                            # Free the string if needed
                            if hasattr(self.lib, 'nada_free_string'):
                                self.lib.nada_free_string(string_ptr)
                            
                            # Store the result string
                            result_ptr[0] = result_str.strip()
                        else:
                            # Just free the intermediate result without converting to string
                            self.lib.nada_free(value_ptr)
                            
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
            self.lib.nada_cleanup_env(self.env)
            self.log.info("NadaLisp environment released")
            self.log.info("NadaLisp environment released")
        return {'status': 'ok', 'restart': restart}
