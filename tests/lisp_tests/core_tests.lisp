(define-test "quote-basic" (assert-equal (quote hello) hello))

(define-test "quote-list" (assert-equal (quote (1 2 3)) (quote (1 2 3))))

(define-test "car-basic" (assert-equal (car (quote (1 2 3))) 1))

(define-test "car-nested" (assert-equal (car (quote ((a b) c d))) (quote (a b))))

(define-test "cdr-basic" (assert-equal (cdr (quote (1 2 3))) (quote (2 3))))

(define-test "cdr-single" (assert-equal (cdr (quote (1))) (quote ())))

(define-test "cons-basic" (assert-equal (cons 1 (quote (2 3))) (quote (1 2 3))))

(define-test "cons-empty" (assert-equal (cons 1 (quote ())) (quote (1))))

(define-test "cons-pair" (assert-equal (cons 1 2) (quote (1 . 2))))

(define-test "list-basic" (assert-equal (list 1 2 3) (quote (1 2 3))))

(define-test "list-empty" (assert-equal (list) (quote ())))

(define-test "list-nested" 
  (assert-equal (list 1 (list 2 3) 4) (quote (1 (2 3) 4))))

(define-test "length-empty" (assert-equal (length (quote ())) 0))

(define-test "length-list" (assert-equal (length (quote (1 2 3 4))) 4))

(define-test "length-nested" 
  (assert-equal (length (quote ((1 2) 3 (4 5)))) 3))