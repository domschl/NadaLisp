;; Tests for integer-sqrt
(define-test "integer-sqrt-zero"
  (assert-equal (integer-sqrt 0) 0))

(define-test "integer-sqrt-one"
  (assert-equal (integer-sqrt 1) 1))

(define-test "integer-sqrt-perfect-square"
  (assert-equal (integer-sqrt 16) 4))

(define-test "integer-sqrt-non-perfect-square"
  (assert-equal (integer-sqrt 15) 3))

(define-test "integer-sqrt-above-square"
  (assert-equal (integer-sqrt 17) 4))


;; Tests for largest-square-divisor
(define-test "largest-square-divisor-prime"
  (assert-equal (largest-square-divisor 7) 1))

(define-test "largest-square-divisor-square"
  (assert-equal (largest-square-divisor 16) 4))

(define-test "largest-square-divisor-composite-12"
  (assert-equal (largest-square-divisor 12) 2))

(define-test "largest-square-divisor-composite-18"
  (assert-equal (largest-square-divisor 18) 3))

;; Symbolic evaluation tests
(define-test "eval-symbolic-1"
  (assert-equal (eval-symbolic '(+ 1 (+ x 2) x 3 y)) '(+ 6 (* 2 x) y)))

;; Tests for sqrt-op
;(define-test "sqrt-op-perfect-square"
;  (assert-equal (sqrt-op 4) 2))

;(define-test "sqrt-op-non-perfect-square"
;  (assert-equal (sqrt-op 8) '(* 2 (sqrt 2))))

;(define-test "sqrt-op-negative-integer"
;  (assert-equal (sqrt-op -4) '(* i 2)))

;(define-test "sqrt-op-rational-extractable"
;  (assert-equal (sqrt-op 9/4) (/ 3 2)))

;(define-test "sqrt-op-rational-partial"
;  (assert-equal (sqrt-op 3/4) '(* 1/2 (sqrt 3))))

;(define-test "sqrt-op-symbolic-fallback"
;  (assert-equal (sqrt-op 'x) '(sqrt x)))