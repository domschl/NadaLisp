; Tests for numeric operations and comparisons

; ----- Basic Arithmetic Tests -----
(define-test "addition"
  (assert-equal (+ 1 2) 3)
  (assert-equal (+ 1 2 3 4 5) 15)
  (assert-equal (+ 1/2 1/2) 1)
  (assert-equal (+ -5 5) 0))

(define-test "subtraction"
  (assert-equal (- 5 3) 2)
  (assert-equal (- 10 2 3) 5)
  (assert-equal (- 5) -5)  ; Unary minus
  (assert-equal (- 1/2 1/4) 1/4))

(define-test "multiplication"
  (assert-equal (* 2 3) 6)
  (assert-equal (* 2 3 4) 24)
  (assert-equal (* 1/2 4) 2)
  (assert-equal (* -2 -3) 6))

(define-test "division"
  (assert-equal (/ 6 3) 2)
  (assert-equal (/ 12 2 3) 2)
  (assert-equal (/ 2) 0.5)  ; Unary division (1/x)
  (assert-equal (/ 1 2) 0.5))

(define-test "modulo"
  (assert-equal (modulo 7 3) 1)
  (assert-equal (% 7 3) 1)  ; % alias
  (assert-equal (modulo -7 3) 2)  ; Check behavior with negative numbers
  (assert-equal (modulo 7 -3) -2))

; ----- Numeric Comparisons Tests -----
(define-test "less-than"
  (assert-equal (< 1 2) #t)
  (assert-equal (< 2 1) #f)
  (assert-equal (< 2 2) #f))

(define-test "less-than-equal"
  (assert-equal (<= 1 2) #t)
  (assert-equal (<= 2 2) #t)
  (assert-equal (<= 3 2) #f))

(define-test "greater-than"
  (assert-equal (> 5 3) #t)
  (assert-equal (> 3 5) #f)
  (assert-equal (> 3 3) #f))

(define-test "greater-than-equal"
  (assert-equal (>= 5 3) #t)
  (assert-equal (>= 5 5) #t)
  (assert-equal (>= 3 5) #f))

(define-test "equal-numbers"
  (assert-equal (= 42 42) #t)
  (assert-equal (= 42 41) #f)
  (assert-equal (= 1/2 0.5) #t))

; ----- Complex Numeric Tests -----
(define-test "numeric-expressions"
  (assert-equal (+ (* 2 3) (/ 6 2)) 9)
  (assert-equal (- (* 3 (+ 2 1)) 4) 5)
  (assert-equal (/ (* 8 2) (- 5 1)) 4))