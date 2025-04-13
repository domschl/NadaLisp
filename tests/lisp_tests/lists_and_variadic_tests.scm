;; Tests for list operations and variadic arguments

;; Test basic list operations
(define-test "Basic list operations"
  (equal? (append '(1 2) '(3 4)) '(1 2 3 4))
  (equal? (append '(1 2) '(3 4) '(5 6)) '(1 2 3 4 5 6))
  (equal? (append '() '(1 2 3)) '(1 2 3))
  (equal? (append '(1 2 3) '()) '(1 2 3))
  (equal? (reverse '(1 2 3 4)) '(4 3 2 1))
  (equal? (reverse '()) '()))

;; Test filter function
(define-test "Filter function"
  (equal? (filter (lambda (x) (< x 5)) '(1 7 3 9 4 6)) '(1 3 4))
  (equal? (filter (lambda (x) (= (modulo x 2) 0)) '(1 2 3 4 5 6)) '(2 4 6))
  (equal? (filter (lambda (x) #t) '(1 2 3)) '(1 2 3))
  (equal? (filter (lambda (x) #f) '(1 2 3)) '()))

;; Test reduce function
(define-test "Reduce function"
  (equal? (reduce + 0 '(1 2 3 4)) 10)
  (equal? (reduce * 1 '(1 2 3 4)) 24  
  (equal? (reduce (lambda (x y) (+ x y)) 0 '(1 2 3 4)) 10)
  (equal? (reduce (lambda (x y) (* x y)) 1 '(1 2 3 4)) 24)
  ; (equal? (reduce (lambda (acc x) (cons x acc)) '() '(1 2 3)) '(3 2 1))
  (equal? (reduce (lambda (acc x) (cons x acc)) '() '(3 2 1)) '(3 2 1))
  (equal? (reduce (lambda (acc x) (cons x acc)) '() '(1 2 3)) '(3 2 1))
  )

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
(define-test "Quicksort with variadic append"
  (equal? (quicksort '()) '())
  (equal? (quicksort '(1)) '(1))
  (equal? (quicksort '(3 1 4 1 5 9 2 6 5)) '(1 1 2 3 4 5 5 6 9))
  (equal? (quicksort '(9 8 7 6 5 4 3 2 1)) '(1 2 3 4 5 6 7 8 9))
  (equal? (quicksort '(1 2 3 4 5)) '(1 2 3 4 5)))

;; Test variadic arguments directly (without using apply)
(define variadic-test
  (lambda args
    args))  ;; Simply return the args list as-is

(define-test "Variadic arguments"
  (equal? (variadic-test) '())
  (equal? (variadic-test 1) '(1))
  (equal? (variadic-test 1 2 3) '(1 2 3)))

;; Test advanced append usage
(define-test "Advanced append usage"
  (equal? (append) '())
  (equal? (append '(1 2)) '(1 2))
  (equal? (append '(1) '(2) '(3) '(4) '(5)) '(1 2 3 4 5))
  (equal? (append '(1 2) '() '(3 4)) '(1 2 3 4)))