;; tests/memory_tests/test_scripts/recursion.lisp
(define factorial
  (lambda (n)
    (if (= n 0)
        1
        (* n (factorial (- n 1))))))

(factorial 5)
(factorial 10)