(define-test "car-operation"
  (assert-equal (car '(1 2 3)) 1))

(define-test "cdr-operation"
  (assert-equal (cdr '(1 2 3)) '(2 3)))

(define-test "cons-operation"
  (assert-equal (cons 1 '(2 3)) '(1 2 3)))

(define-test "list-operation"
  (assert-equal (list 1 2 3) '(1 2 3)))

(define-test "dotted-pair"
  (assert-equal (cons 1 2) '(1 . 2)))

;; Tests for append-two
(define-test "append-two-empty-lists"
  (assert-equal (append-two '() '()) '()))

(define-test "append-two-first-empty"
  (assert-equal (append-two '() '(3 4)) '(3 4)))

(define-test "append-two-second-empty"
  (assert-equal (append-two '(1 2) '()) '(1 2)))

(define-test "append-two-normal-lists"
  (assert-equal (append-two '(1 2) '(3 4)) '(1 2 3 4)))

;; Tests for append-all
(define-test "append-all-empty-list"
  (assert-equal (append-all '()) '()))

(define-test "append-all-single-list"
  (assert-equal (append-all '((1 2 3))) '(1 2 3)))

(define-test "append-all-multiple-lists"
  (assert-equal (append-all '((1) (2 3) (4 5 6))) '(1 2 3 4 5 6)))

;; Tests for append
(define-test "append-no-args"
  (assert-equal (append) '()))

(define-test "append-one-arg"
  (assert-equal (append '(1 2 3)) '(1 2 3)))

(define-test "append-multiple-args"
  (assert-equal (append '(1 2) '(3) '(4 5 6)) '(1 2 3 4 5 6)))

(define-test "append-with-empty-lists"
  (assert-equal (append '() '(1 2) '() '(3 4)) '(1 2 3 4)))

;; Tests for reverse
(define-test "reverse-empty-list"
  (assert-equal (reverse '()) '()))

(define-test "reverse-single-element"
  (assert-equal (reverse '(1)) '(1)))

(define-test "reverse-multiple-elements"
  (assert-equal (reverse '(1 2 3 4 5)) '(5 4 3 2 1)))

;; Tests for filter
(define-test "filter-empty-list"
  (assert-equal (filter (lambda (x) (> x 3)) '()) '()))

(define-test "filter-no-matches"
  (assert-equal (filter (lambda (x) (> x 10)) '(1 2 3 4 5)) '()))

(define-test "filter-all-match"
  (assert-equal (filter (lambda (x) (> x 0)) '(1 2 3 4 5)) '(1 2 3 4 5)))

(define-test "filter-some-match"
  (assert-equal (filter (lambda (x) (> x 3)) '(1 2 3 4 5)) '(4 5)))

;; Tests for reduce
(define-test "reduce-empty-list"
  (assert-equal (reduce + 0 '()) 0))

(define-test "reduce-sum"
  (assert-equal (reduce + 0 '(1 2 3 4 5)) 15))

(define-test "reduce-product"
  (assert-equal (reduce * 1 '(2 3 4)) 24))

(define-test "reduce-string-concat"
  (assert-equal (reduce string-append "" '("a" "b" "c")) "abc"))

;; Tests for member and member?
(define-test "member-empty-list"
  (assert-equal (member 1 '()) #f))

(define-test "member-element-present"
  (assert-equal (member 2 '(1 2 3 4)) '(2 3 4)))

(define-test "member-element-absent"
  (assert-equal (member 5 '(1 2 3 4)) #f))

(define-test "member?-element-present"
  (assert-equal (member? 3 '(1 2 3 4)) #t))

(define-test "member?-element-absent"
  (assert-equal (member? 5 '(1 2 3 4)) #f))

;; Tests for nested list accessors
(define-test "caar-operation"
  (assert-equal (caar '((1 2) (3 4))) 1))

(define-test "cdar-operation"
  (assert-equal (cdar '((1 2) (3 4))) '(2)))

(define-test "cddr-operation"
  (assert-equal (cddr '(1 2 3 4)) '(3 4)))

(define-test "cdddr-operation"
  (assert-equal (cdddr '(1 2 3 4)) '(4)))

(define-test "cadr-operation"
  (assert-equal (cadr '(1 2 3 4)) 2))

(define-test "caddr-operation"
  (assert-equal (caddr '(1 2 3 4)) 3))

;; Tests for fold-left
(define-test "fold-left-empty-list"
  (assert-equal (fold-left + 0 '()) 0))

(define-test "fold-left-sum"
  (assert-equal (fold-left + 0 '(1 2 3 4 5)) 15))

(define-test "fold-left-string-concat"
  (assert-equal (fold-left string-append "" '("a" "b" "c")) "abc"))

(define-test "fold-left-custom-function"
  (assert-equal (fold-left (lambda (acc x) (+ (* 2 acc) x)) 1 '(2 3 4)) 26))

;; Tests for assoc
(define-test "assoc-empty-list"
  (assert-equal (assoc 'a '()) #f))

(define-test "assoc-key-present"
  (assert-equal (assoc 'b '((a . 1) (b . 2) (c . 3))) '(b . 2)))

(define-test "assoc-key-absent"
  (assert-equal (assoc 'd '((a . 1) (b . 2) (c . 3))) #f))

(define-test "assoc-first-match"
  (assert-equal (assoc 'a '((a . 1) (b . 2) (a . 3))) '(a . 1)))