#!/usr/bin/env python3
"""
NadaLisp Jupyter Kernel - Test Script
"""
import os
import sys
import unittest
from jupyter_client.manager import start_new_kernel

class TestNadaKernel(unittest.TestCase):
    """Test the NadaLisp Jupyter kernel"""
    
    @classmethod
    def setUpClass(cls):
        """Start the kernel and set up the client"""
        cls.kernel_name = "nada"
        try:
            cls.km, cls.kc = start_new_kernel(kernel_name=cls.kernel_name)
        except:
            print("ERROR: Could not start the NadaLisp kernel.")
            print("Make sure you've installed the kernel with 'python -m ipykernel install --name nada --display-name NadaLisp'")
            sys.exit(1)
    
    @classmethod
    def tearDownClass(cls):
        """Shut down the kernel"""
        cls.kc.stop_channels()
        cls.km.shutdown_kernel()
    
    def execute_code(self, code):
        """Execute code in the kernel and return the result"""
        msg_id = self.kc.execute(code)
        reply = self.kc.get_shell_msg(timeout=10)
        return reply['content']
    
    def test_simple_expression(self):
        """Test a simple arithmetic expression"""
        result = self.execute_code("(+ 2 3)")
        self.assertEqual(result['status'], 'ok')
    
    def test_define_and_use(self):
        """Test defining a variable and using it"""
        # Define a variable
        result1 = self.execute_code("(define x 42)")
        self.assertEqual(result1['status'], 'ok')
        
        # Use the variable
        result2 = self.execute_code("x")
        self.assertEqual(result2['status'], 'ok')
    
    def test_function_definition(self):
        """Test defining and calling a function"""
        # Define a function
        define_code = """
        (define (square x)
          (* x x))
        """
        result1 = self.execute_code(define_code)
        self.assertEqual(result1['status'], 'ok')
        
        # Call the function
        result2 = self.execute_code("(square 5)")
        self.assertEqual(result2['status'], 'ok')

if __name__ == "__main__":
    unittest.main()
