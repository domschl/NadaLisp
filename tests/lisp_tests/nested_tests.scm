(define double (lambda (x) (* 2 x)))
(define quad (lambda (x) (double (double x))))

(define-test "double-test" 
  (assert-equal (double 5) 10))
(define-test "quad-test"
  (assert-equal (quad 5) 20))
(define-test "nested-test"
  (assert-equal (quad (double 5)) 40))
