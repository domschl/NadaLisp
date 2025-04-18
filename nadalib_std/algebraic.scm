;; Purely functional algebraic notation utilities

;; Define variables for testing
(define x 'x)
(define y 'y)
(define z 'z)
(define a 'a)
(define b 'b)
(define c 'c)
(define d 'd)
(define e 'e)
(define f 'f)
(define g 'g)


;; Fixed exponentiation operation - handles both numeric and symbolic cases
(define expt-op
  (lambda (base exp)
    (cond
      ;; Integer exponents can be computed exactly
      ((integer? exp) 
       (expt base exp))  ;; Simplified - both branches were identical
      
      ;; Special case: square root of perfect square
      ((and (= (denominator exp) 2) 
            (integer? base)
            (integer? (sqrt base)))
       (sqrt base))
      
      ;; Special case: cube root of perfect cube
      ((and (= (denominator exp) 3)
            (integer? base)
            (integer? (expt base (/ 1 3))))
       (expt base (/ 1 3)))
      
      ;; Keep symbolic for other cases
      (else (list 'expt base exp)))))


;; Update eval-algebraic to use full-simplify
(define eval-algebraic
  (lambda (expr)
    (eval (infix->prefix expr))))
