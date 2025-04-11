; Tests focused on special forms and closures, 2nd part

; ----- Lambda and Closure Tests -----

(define-test "lambda-closure"
  (define make-adder
    (lambda (n)
      (lambda (x) (+ x n))))
  (define add5 (make-adder 5))
  (assert-equal (add5 10) 15)
  (assert-equal ((make-adder 2) 10) 12))

