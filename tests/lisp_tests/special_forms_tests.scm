; Tests focused on special forms and closures

; ----- Let Form Tests -----
(define-test "let-basic-form"
  (assert-equal (let ((a 1) (b 2)) 
                  (+ a b)) 
                3)
  (define a 5)
  (assert-equal (let ((a 1) 
                      (b (+ a 2)))  ; a refers to outer scope's a
                  (+ a b))
                8))

; ----- Lambda Tests -----
(define-test "lambda-basics"
  (assert-equal ((lambda (x) (* x x)) 4) 16)
  (assert-equal ((lambda (x y) (+ x y)) 2 3) 5)
  (assert-equal (((lambda (x) (lambda (y) (+ x y))) 5) 3) 8))


; ----- Function Definition Tests -----
(define-test "function-definition"
  (define (square x) (* x x))
  (define (sum-of-squares x y)
    (+ (square x) (square y)))
  (assert-equal (square 4) 16)
  (assert-equal (sum-of-squares 3 4) 25))

; ----- Recursion Tests -----
(define-test "recursive-functions"
  (define (factorial n)
    (if (= n 0)
        1
        (* n (factorial (- n 1)))))
  (assert-equal (factorial 5) 120)
  
  (define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))))
  (assert-equal (fib 7) 13))
