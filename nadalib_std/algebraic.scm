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

(define integer-sqrt-leak
  (lambda (n)
    (if (< n 0)
        (error "Cannot compute square root of negative number")
        (let loop ((low 0) (high (+ n 1)))
          (let ((mid (quotient (+ low high) 2)))
            (if (= mid low)
                low
                (if (> (* mid mid) n)
                    (loop low mid)
                    (loop mid high))))))))

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


;; Helper function to identify symbolic expressions
(define symbolic?
  (lambda (expr)
    (or (variable? expr)                  ; Single variable
        (and (list? expr)                 ; Function application
             (not (null? expr))
             (not (number? expr))))))

;; Addition operation
(define add-op
  (lambda args
    (let ((numeric-sum 0)
          (symbolic-terms '()))
      ;; First pass: separate numeric and symbolic terms
      (for-each
        (lambda (term)
          (if (number? term)
              (set! numeric-sum (+ numeric-sum term))
              (set! symbolic-terms (cons term symbolic-terms))))
        args)
      ;; Second pass: build result
      (let ((result
              (if (null? symbolic-terms)
                  numeric-sum                ; Just a number
                  (if (= numeric-sum 0)
                      (if (null? (cdr symbolic-terms))
                          (car symbolic-terms)  ; Just one symbolic term
                          (cons '+ (reverse symbolic-terms))) ; Multiple symbolic terms
                      (cons '+ (cons numeric-sum (reverse symbolic-terms)))))))
        result))))

;; Subtraction operation
(define sub-op
  (lambda args
    (if (null? args)
        0  ; No arguments means 0
        (if (null? (cdr args))
            (if (number? (car args))
                (- (car args))  ; Negate a single numeric argument
                (list '- (car args)))  ; Negate a single symbolic argument
            ;; Multiple arguments: first - rest
            (let ((first (car args))
                  (rest (cdr args)))
              (add-op first (mul-op -1 (apply add-op rest))))))))

;; Multiplication operation
(define mul-op
  (lambda args
    (let ((numeric-product 1)
          (symbolic-factors '()))
      ;; First pass: separate numeric and symbolic factors
      (for-each
        (lambda (factor)
          (if (number? factor)
              (set! numeric-product (* numeric-product factor))
              (set! symbolic-factors (cons factor symbolic-factors))))
        args)
      ;; Second pass: build result
      (let ((result
              (cond
                ((= numeric-product 0) 0)  ; Anything * 0 = 0
                ((null? symbolic-factors) numeric-product)  ; Just a number
                ((= numeric-product 1)
                 (if (null? (cdr symbolic-factors))
                     (car symbolic-factors)  ; Just one symbolic factor
                     (cons '* (reverse symbolic-factors)))) ; Multiple symbolic factors
                (else (cons '* (cons numeric-product (reverse symbolic-factors)))))))
        result))))

;; Division operation
(define div-op
  (lambda args
    (if (null? args)
        1  ; No arguments means 1
        (if (null? (cdr args))
            (if (number? (car args))
                (/ 1 (car args))  ; Reciprocal of a single numeric argument
                (list '/ 1 (car args)))  ; Reciprocal of a single symbolic argument
            ;; Multiple arguments: first / rest
            (let ((first (car args))
                  (rest (cdr args)))
              (mul-op first (expt-op (apply mul-op rest) -1)))))))

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
       (sqrt-op (expt base (numerator exp))))
      
      ;; Keep symbolic for other cases
      (else (list 'expt base exp)))))

;; Evaluates symbolic expressions, simplifying where possible
(define eval-symbolic
  (lambda (expr)
    (cond
      ;; Constants evaluate to themselves
      ((constant? expr) expr)
      
      ;; Variables evaluate to themselves
      ((variable? expr) expr)
      
      ;; Lists are function applications
      ((list? expr)
       (if (null? expr)
           expr  ; Empty list evaluates to itself
           (let ((op (car expr))
                 (args (map eval-symbolic (cdr expr))))
             (cond
               ((eq? op '+) (apply add-op args))
               ((eq? op '-) (apply sub-op args))
               ((eq? op '*) (apply mul-op args))
               ((eq? op '/) (apply div-op args))
               ((eq? op 'expt) (apply expt-op args))
               ((eq? op 'sqrt) (sqrt-op (car args)))
               (else (cons op args))))))  ; Unknown operation
      
      ;; Default: return as-is
      (else expr))))