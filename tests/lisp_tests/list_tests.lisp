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