(define-test "less-than-true" (assert-equal (< 1 2) #t))

(define-test "less-than-false" (assert-equal (< 2 1) #f))

(define-test "less-than-equal-true-1" (assert-equal (<= 1 2) #t))

(define-test "less-than-equal-true-2" (assert-equal (<= 2 2) #t))

(define-test "less-than-equal-false" (assert-equal (<= 3 2) #f))

(define-test "greater-than-true" (assert-equal (> 5 3) #t))

(define-test "greater-than-false" (assert-equal (> 3 5) #f))

(define-test "greater-than-equal-true-1" (assert-equal (>= 5 3) #t))

(define-test "greater-than-equal-true-2" (assert-equal (>= 5 5) #t))

(define-test "greater-than-equal-false" (assert-equal (>= 3 5) #f))

(define-test "equal-numbers-true" (assert-equal (= 42 42) #t))

(define-test "equal-numbers-false" (assert-equal (= 42 41) #f))

(define-test "equal-fractions" (assert-equal (= 1/2 0.5) #t))

(define-test "eq-symbols" (assert-equal (eq? (quote hello) (quote hello)) #t))

(define-test "eq-different-symbols" 
  (assert-equal (eq? (quote hello) (quote world)) #f))

(define-test "eq-numbers" (assert-equal (eq? 42 42) #t))

(define-test "eq-different-numbers" (assert-equal (eq? 42 43) #f))

(define-test "eq-empty-lists" 
  (assert-equal (eq? (quote ()) (quote ())) #t))

(define-test "equal-lists" 
  (assert-equal (equal? (quote (1 2 3)) (quote (1 2 3))) #t))

(define-test "equal-nested-lists"
  (assert-equal (equal? (quote (1 (2 3) 4)) (quote (1 (2 3) 4))) #t))

(define-test "equal-different-lists"
  (assert-equal (equal? (quote (1 2 3)) (quote (1 2 4))) #f))