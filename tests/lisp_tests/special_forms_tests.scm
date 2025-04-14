; Tests focused on special forms and closures

; ----- Let Form Tests -----
(define-test "let-basic-form-1" 
  (assert-equal (let ((a 1) (b 2)) 
                  (+ a b)) 
                3))

(define-test "let-outer-scope" 
  (begin
    (define a 5)
    (assert-equal (let ((a 1) 
                        (b (+ a 2)))  ; a refers to outer scope's a
                    (+ a b))
                  8)))

; ----- Lambda Tests -----
(define-test "lambda-basics-1"
  (assert-equal ((lambda (x) (* x x)) 4) 16))

(define-test "lambda-basics-2"
  (assert-equal ((lambda (x y) (+ x y)) 2 3) 5))

(define-test "lambda-basics-3"
  (assert-equal (((lambda (x) (lambda (y) (+ x y))) 5) 3) 8))

; ----- Function Definition Tests -----
(define-test "function-definition-1"
  (begin
    (define (square x) (* x x))
    (assert-equal (square 4) 16)))

(define-test "function-definition-2"
  (begin
    (define (square x) (* x x))
    (define (sum-of-squares x y)
      (+ (square x) (square y)))
    (assert-equal (sum-of-squares 3 4) 25)))

; ----- Recursion Tests -----
(define-test "recursive-functions-1"
  (begin
    (define (factorial n)
      (if (= n 0)
          1
          (* n (factorial (- n 1)))))
    (assert-equal (factorial 5) 120)))
  
(define-test "recursive-functions-2"
  (begin
    (define (fib n)
      (if (< n 2)
          n
          (+ (fib (- n 1)) (fib (- n 2)))))
    (assert-equal (fib 7) 13)))

; ----- Apply Tests -----
(define-test "apply-basic-addition"
  (assert-equal (apply + '(1 2 3 4 5)) 15))

(define-test "apply-basic-multiplication"
  (assert-equal (apply * '(2 3 4)) 24))

(define-test "apply-with-empty-list"
  (assert-equal (apply + '()) 0))

(define-test "apply-with-user-defined-function"
  (begin
    (define (sum-squares lst)
      (apply + (map (lambda (x) (* x x)) lst)))
    (assert-equal (sum-squares '(1 2 3 4)) 30)))

(define-test "apply-with-lambda"
  (assert-equal (apply (lambda (x y) (+ x (* y 2))) '(5 7)) 19))

;; currently not working
;;(define-test "apply-nested"
;;  (begin
;;    (define op-list (list + - * /))
;;    (define arg-lists '((10 5) (10 5) (10 5) (10 5)))
;;    (assert-equal 
;;      (map (lambda (op args) (apply op args)) op-list arg-lists)
;;      '(15 5 50 2))))

(define-test "apply-with-varargs-function"
  (begin
    (define (sum-and-multiply first . rest)
      (* first (apply + rest)))
    (assert-equal (apply sum-and-multiply '(2 3 4 5)) 24)))

(define-test "apply-higher-order"
  (begin
    (define (compose f g)
      (lambda args 
        (f (apply g args))))
    (define add-then-square (compose (lambda (x) (* x x)) +))
    (assert-equal (apply add-then-square '(1 2 3 4)) 100)))
