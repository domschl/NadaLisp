(define-test "if-true-branch" 
  (assert-equal (if #t 1 2) 1))

(define-test "if-false-branch" 
  (assert-equal (if #f 1 2) 2))

(define-test "if-expr-condition" 
  (assert-equal (if (< 2 3) "yes" "no") "yes"))

(define-test "if-no-else" 
  (assert-equal (if #f 1) ()))

(define-test "cond-first" 
  (assert-equal 
    (cond (#t 1) (#t 2) (#t 3)) 
    1))

(define-test "cond-second" 
  (assert-equal 
    (cond (#f 1) (#t 2) (#t 3)) 
    2))

(define-test "cond-else" 
  (assert-equal 
    (cond (#f 1) (#f 2) (else 3)) 
    3))

(define-test "cond-expr" 
  (assert-equal 
    (cond 
      ((< 5 3) "wrong") 
      ((= 4 4) "right") 
      (else "also wrong")) 
    "right"))

(define-test "cond-no-match" 
  (assert-equal 
    (cond (#f 1) (#f 2)) 
    ()))

(define-test "let-simple" 
  (assert-equal 
    (let ((x 10)) x) 
    10))

(define-test "let-multiple" 
  (assert-equal 
    (let ((x 10) (y 20)) (+ x y)) 
    30))

(define-test "let-nested" 
  (assert-equal 
    (let ((x 10)) 
      (let ((y (+ x 5))) 
        (+ x y))) 
    25))

(define-test "let-shadowing" 
  (assert-equal 
    (let ((x 10)) 
      (let ((x 20)) 
        x)) 
    20))

(define-test "define-simple" 
  (begin
    (define x 42)
    (assert-equal x 42)))

(define-test "define-function" 
  (begin
    (define add-five (lambda (x) (+ x 5)))
    (assert-equal (add-five 10) 15)))

(define-test "define-sugar" 
  (begin
    (define (mul-ten x) (* x 10))
    (assert-equal (mul-ten 5) 50)))