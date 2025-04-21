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

;; String comparison tests
(define-test "string-less-than-true" 
  (assert-equal (< "abc" "def") #t))

(define-test "string-less-than-false" 
  (assert-equal (< "def" "abc") #f))

(define-test "string-less-than-prefix" 
  (assert-equal (< "abc" "abcd") #t))

(define-test "string-less-than-case" 
  (assert-equal (< "ABC" "abc") #t))

(define-test "string-less-equal-true-different" 
  (assert-equal (<= "abc" "def") #t))

(define-test "string-less-equal-true-same" 
  (assert-equal (<= "abc" "abc") #t))

(define-test "string-less-equal-false" 
  (assert-equal (<= "xyz" "abc") #f))

(define-test "string-greater-than-true" 
  (assert-equal (> "xyz" "abc") #t))

(define-test "string-greater-than-false" 
  (assert-equal (> "abc" "xyz") #f))

(define-test "string-greater-than-prefix" 
  (assert-equal (> "abcd" "abc") #t))

(define-test "string-greater-equal-true-different" 
  (assert-equal (>= "xyz" "abc") #t))

(define-test "string-greater-equal-true-same" 
  (assert-equal (>= "abc" "abc") #t))

(define-test "string-greater-equal-false" 
  (assert-equal (>= "abc" "xyz") #f))

(define-test "string-eq-true" 
  (assert-equal (eq? "hello" "hello") #t))

(define-test "string-eq-false" 
  (assert-equal (eq? "hello" "world") #f))

(define-test "string-equal-true" 
  (assert-equal (equal? "hello" "hello") #t))

(define-test "string-equal-false" 
  (assert-equal (equal? "hello" "world") #f))

;; Standard Scheme string comparison tests
(define-test "string<? true" 
  (assert-equal (string<? "abc" "def") #t))

(define-test "string<? false" 
  (assert-equal (string<? "def" "abc") #f))

(define-test "string<? prefix" 
  (assert-equal (string<? "abc" "abcd") #t))

(define-test "string<? case" 
  (assert-equal (string<? "ABC" "abc") #t))

(define-test "string<=? true-different" 
  (assert-equal (string<=? "abc" "def") #t))

(define-test "string<=? true-same" 
  (assert-equal (string<=? "abc" "abc") #t))

(define-test "string<=? false" 
  (assert-equal (string<=? "xyz" "abc") #f))

(define-test "string>? true" 
  (assert-equal (string>? "xyz" "abc") #t))

(define-test "string>? false" 
  (assert-equal (string>? "abc" "xyz") #f))

(define-test "string>? prefix" 
  (assert-equal (string>? "abcd" "abc") #t))

(define-test "string>=? true-different" 
  (assert-equal (string>=? "xyz" "abc") #t))

(define-test "string>=? true-same" 
  (assert-equal (string>=? "abc" "abc") #t))

(define-test "string>=? false" 
  (assert-equal (string>=? "abc" "xyz") #f))

(define-test "string=? true" 
  (assert-equal (string=? "hello" "hello") #t))

(define-test "string=? false" 
  (assert-equal (string=? "hello" "world") #f))

(define-test "string=? case-sensitive" 
  (assert-equal (string=? "Hello" "hello") #f))