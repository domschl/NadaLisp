;; Tests for list operations and variadic arguments

;; Test basic list operations
(define-test "append-basic-1" (assert-equal (append '(1 2) '(3 4)) '(1 2 3 4)))
(define-test "append-basic-2" (assert-equal (append '(1 2) '(3 4) '(5 6)) '(1 2 3 4 5 6)))
(define-test "append-basic-3" (assert-equal (append '() '(1 2 3)) '(1 2 3)))
(define-test "append-basic-4" (assert-equal (append '(1 2 3) '()) '(1 2 3)))
(define-test "reverse-basic-1" (assert-equal (reverse '(1 2 3 4)) '(4 3 2 1)))
(define-test "reverse-basic-2" (assert-equal (reverse '()) '()))

;; Test filter function
(define-test "filter-1" (assert-equal (filter (lambda (x) (< x 5)) '(1 7 3 9 4 6)) '(1 3 4)))
(define-test "filter-2" (assert-equal (filter (lambda (x) (= (modulo x 2) 0)) '(1 2 3 4 5 6)) '(2 4 6)))
(define-test "filter-3" (assert-equal (filter (lambda (x) #t) '(1 2 3)) '(1 2 3)))
(define-test "filter-4" (assert-equal (filter (lambda (x) #f) '(1 2 3)) '()))

;; Test reduce function
(define-test "reduce-1" (assert-equal (reduce + 0 '(1 2 3 4)) 10))
(define-test "reduce-2" (assert-equal (reduce * 1 '(1 2 3 4)) 24))
(define-test "reduce-3" (assert-equal (reduce (lambda (x y) (+ x y)) 0 '(1 2 3 4)) 10))
(define-test "reduce-4" (assert-equal (reduce (lambda (x y) (* x y)) 1 '(1 2 3 4)) 24))
(define-test "reduce-5" 
  (assert-equal 
    (reduce (lambda (acc x) (cons x acc)) '() '(1 2 3)) 
    '(3 2 1)))

;; Define quicksort using filter and append with variadic arguments
(define quicksort
  (lambda (lst)
    (if (null? lst)
        '()
        (let ((pivot (car lst))
              (rest (cdr lst)))
          (append 
            (quicksort (filter (lambda (x) (< x pivot)) rest))
            (list pivot)
            (quicksort (filter (lambda (x) (>= x pivot)) rest)))))))

;; Test quicksort implementation
(define-test "quicksort-1" (assert-equal (quicksort '()) '()))
(define-test "quicksort-2" (assert-equal (quicksort '(1)) '(1)))
(define-test "quicksort-3" (assert-equal (quicksort '(3 1 4 1 5 9 2 6 5)) '(1 1 2 3 4 5 5 6 9)))
(define-test "quicksort-4" (assert-equal (quicksort '(9 8 7 6 5 4 3 2 1)) '(1 2 3 4 5 6 7 8 9)))
(define-test "quicksort-5" (assert-equal (quicksort '(1 2 3 4 5)) '(1 2 3 4 5)))

;; Test variadic arguments directly (without using apply)
(define variadic-test
  (lambda args
    args))  ;; Simply return the args list as-is

(define-test "variadic-args-1" (assert-equal (variadic-test) '()))
(define-test "variadic-args-2" (assert-equal (variadic-test 1) '(1)))
(define-test "variadic-args-3" (assert-equal (variadic-test 1 2 3) '(1 2 3)))

;; Test advanced append usage
(define-test "advanced-append-1" (assert-equal (append) '()))
(define-test "advanced-append-2" (assert-equal (append '(1 2)) '(1 2)))
(define-test "advanced-append-3" (assert-equal (append '(1) '(2) '(3) '(4) '(5)) '(1 2 3 4 5)))
(define-test "advanced-append-4" (assert-equal (append '(1 2) '() '(3 4)) '(1 2 3 4)))