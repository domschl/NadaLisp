;; Purely functional algebraic notation utilities

;; Example implementation for special constants
(define pi-value 'pi)  ;; Symbolic representation
(define e-value 'e)
(define i-value 'i)

;; All numbers in the core are rational, so this should just use number?
(define rational? 
  (lambda (expr)
    (number? expr)))

;; A fraction is a rational that's not an integer
(define fraction? 
  (lambda (expr)
    (and (rational? expr) 
         (not (integer? expr)))))

;; Irrational numbers can only be represented symbolically
(define irrational? 
  (lambda (expr)
    (or (eq? expr 'pi) 
        (eq? expr 'e))))

;; Complex numbers are represented by the symbol i or compound expressions
(define complex? 
  (lambda (expr)
    (eq? expr 'i)))

;; Updated constant? predicate to include all special numeric constants
(define constant?
  (lambda (expr)
    (or (number? expr)       ;; Rational numbers
        (irrational? expr)   ;; Symbolic irrational constants
        (complex? expr))))   ;; Symbolic complex constants

;; Add a variable? predicate to check if something is a variable
(define variable?
  (lambda (expr)
    (and (symbol? expr) 
         (not (irrational? expr))
         (not (complex? expr)))))

;; Update eval-algebraic to use full-simplify
(define eval-algebraic
  (lambda (expr)
    (eval (infix->prefix expr))))

;; Integer division truncated toward zero, using `remainder`
(define quotient
  (lambda (n d)
    (if (= d 0)
        (error "Division by zero in quotient")
        (let ((r (remainder n d)))
          ;; n = d*q + r  ⇒  q = (n – r)/d
          (/ (- n r) d)))))

;; Integer square root - returns largest integer not exceeding sqrt(n)
(define integer-sqrt-loop (lambda (low high n)
  (let ((mid (quotient (+ low high) 2)))
    (if (= mid low)
        low
        (if (> (* mid mid) n)
            (integer-sqrt-loop low mid n)
            (integer-sqrt-loop mid high n))))))

(define integer-sqrt
  (lambda (n)
    (if (< n 0)
        (error "Cannot compute square root of negative number")
        (integer-sqrt-loop 0 (+ n 1) n))))

;(define integer-sqrt
;  (lambda (n)
;    (if (< n 0)
;        (error "Cannot compute square root of negative number")
;        (let loop ((low 0) (high (+ n 1)))
 ;         (let ((mid (quotient (+ low high) 2)))
;            (if (= mid low)
;                low
;                (if (> (* mid mid) n)
;                    (loop low mid)
;                    (loop mid high))))))))

(define zero?
  (lambda (x)
    (cond ((number? x) (= x 0))
          (else #f))))

;; Extract the largest square divisor of n by walking down from integer‑sqrt(n)
;; Extract the largest square divisor of n using mutation (`set!`)
(define largest-square-divisor
  (lambda (n)
    (let ((i (integer-sqrt n)))
      (let loop ()
        (cond
          ((<= i 1) 1)
          ((zero? (remainder n (* i i))) i)
          (else
            (begin
              (set! i (- i 1))   ; decrement i by 1
              (loop))))))))

;; √‑operation that pulls perfect squares out of a rational
(define sqrt-op
  (lambda (x)
    (cond
      ;; negative case: factor out i
      ((and (number? x) (< x 0))
       (list '* i-value (sqrt-op (- x))))
      ;; integer case
      ((integer? x)
       (let ((f (largest-square-divisor x)))
         (let ((r (/ x (* f f))))
           (cond
             ((= f 1)      (list 'sqrt x))         ; no square factor
             ((= r 1)      f)                      ; perfect square
             (else         (list '* f (list 'sqrt r)))))))
      ;; rational case
      ((rational? x)
       (let ((n (numerator x)))
         (let ((d (denominator x)))
           (let ((fn (largest-square-divisor n)))
             (let ((fd (largest-square-divisor d)))
               (let ((coef (/ fn fd)))
                 (let ((rn (/ n (* fn fn))))
                   (let ((rd (/ d (* fd fd))))
                     (let ((rem (if (= rd 1) rn (/ rn rd))))
                       (let ((root (list 'sqrt rem)))
                         (cond
                           ((and (= rn 1) (= rd 1)) coef)   ; everything extracted
                           ((= coef 1)           root)       ; only root remains
                           (else                  (list '* coef root)))))))))))))
      ;; fallback: symbolic
      (else (list 'sqrt x)))))

;; Fixed exponentiation operation - handles both numeric and symbolic cases
(define expt-op
  (lambda (base exp)
    (cond
      ;; Integer exponents can be computed exactly
      ((integer? exp) 
       (expt base exp))  ;; Simplified - both branches were identical
      
      ;; Special case: square root of perfect square
      (if (= (denominator exp) 2) 
       (sqrt-op (expt base (numerator exp)))
      
      ;; Keep symbolic for other cases
      (else (list 'expt base exp)))))

