(define-test "add-fractions"
  (assert-equal (+ 1/3 2/3) 1))

(define-test "add-mixed"
  (assert-equal (+ 1/2 3/2) 2))

(define-test "subtract-fractions"
  (assert-equal (- 5/6 1/6) 4/6))

(define-test "subtract-mixed"
  (assert-equal (- 3 1/3) 8/3))

(define-test "multiply-fractions"
  (assert-equal (* 2/3 3/4) 1/2))

(define-test "multiply-mixed"
  (assert-equal (* 2 1/2) 1))

(define-test "divide-fractions"
  (assert-equal (/ 2/3 1/6) 4))

(define-test "divide-mixed"
  (assert-equal (/ 3 1/2) 6))

(define-test "complex-fraction-expr"
  (assert-equal (/ (+ 1/3 1/6) (* 3/4 2/3)) 1))

(define-test "gcd-calculation"
  (assert-equal (/ 75 125) 3/5))

(define-test "large-rationals"
  (assert-equal (* (/ 999999 1000000) 1000000) 999999))

(define-test "repeat-decimal"
  (begin
    (define thirds (/ 1 3))
    (assert-equal (* thirds 3) 1)))

(define-test "rational-comparison"
  (begin
    (assert-equal (= 1/2 0.5) #t)
    (assert-equal (< 1/3 1/2) #t)
    (assert-equal (> 3/4 2/3) #t)))