; Tests for numeric operations and comparisons

; ----- Basic Arithmetic Tests -----
(define-test "addition-1" (assert-equal (+ 1 2) 3))
(define-test "addition-2" (assert-equal (+ 1 2 3 4 5) 15))
(define-test "addition-3" (assert-equal (+ 1/2 1/2) 1))
(define-test "addition-4" (assert-equal (+ -5 5) 0))

(define-test "subtraction-1" (assert-equal (- 5 3) 2))
(define-test "subtraction-2" (assert-equal (- 10 2 3) 5))
(define-test "subtraction-3" (assert-equal (- 5) -5))  ; Unary minus
(define-test "subtraction-4" (assert-equal (- 1/2 1/4) 1/4))

(define-test "multiplication-1" (assert-equal (* 2 3) 6))
(define-test "multiplication-2" (assert-equal (* 2 3 4) 24))
(define-test "multiplication-3" (assert-equal (* 1/2 4) 2))
(define-test "multiplication-4" (assert-equal (* -2 -3) 6))

(define-test "division-1" (assert-equal (/ 6 3) 2))
(define-test "division-2" (assert-equal (/ 12 2 3) 2))
(define-test "division-3" (assert-equal (/ 2) 0.5))  ; Unary division (1/x)
(define-test "division-4" (assert-equal (/ 1 2) 0.5))

(define-test "modulo-1" (assert-equal (modulo 7 3) 1))
(define-test "modulo-2" (assert-equal (% 7 3) 1))  ; % alias
(define-test "modulo-3" (assert-equal (modulo -7 3) 2))  ; Check behavior with negative numbers
(define-test "modulo-4" (assert-equal (modulo 7 -3) -2))
(define-test "remainder-1" (assert-equal (remainder -7 3) -1))  ; Check behavior with negative numbers

; ----- Numeric Comparisons Tests -----
(define-test "less-than-1" (assert-equal (< 1 2) #t))
(define-test "less-than-2" (assert-equal (< 2 1) #f))
(define-test "less-than-3" (assert-equal (< 2 2) #f))

(define-test "less-than-equal-1" (assert-equal (<= 1 2) #t))
(define-test "less-than-equal-2" (assert-equal (<= 2 2) #t))
(define-test "less-than-equal-3" (assert-equal (<= 3 2) #f))

(define-test "greater-than-1" (assert-equal (> 5 3) #t))
(define-test "greater-than-2" (assert-equal (> 3 5) #f))
(define-test "greater-than-3" (assert-equal (> 3 3) #f))

(define-test "greater-than-equal-1" (assert-equal (>= 5 3) #t))
(define-test "greater-than-equal-2" (assert-equal (>= 5 5) #t))
(define-test "greater-than-equal-3" (assert-equal (>= 3 5) #f))

(define-test "equal-numbers-1" (assert-equal (= 42 42) #t))
(define-test "equal-numbers-2" (assert-equal (= 42 41) #f))
(define-test "equal-numbers-3" (assert-equal (= 1/2 0.5) #t))

; ----- Complex Numeric Tests -----
(define-test "numeric-expressions-1" (assert-equal (+ (* 2 3) (/ 6 2)) 9))
(define-test "numeric-expressions-2" (assert-equal (- (* 3 (+ 2 1)) 4) 5))
(define-test "numeric-expressions-3" (assert-equal (/ (* 8 2) (- 5 1)) 4))