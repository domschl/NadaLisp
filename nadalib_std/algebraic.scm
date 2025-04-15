;; Purely functional algebraic notation utilities

;; Define variables for testing
(define x 'x)
(define y 'y)
(define z 'z)
(define a 'a)
(define b 'b)
(define c 'c)

;; Example implementation for special constants
(define pi-value 'pi)  ;; Symbolic representation
(define e-value 'e)
(define i-value 'i)

;; Updated type predicates

;; Already defined in core, returns true for any numeric value
;; (number? x)

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

;; Operator precedence table
(define op-precedence 
  (lambda (op)
    (cond
      ((equal? op "+") 1)
      ((equal? op "-") 1)
      ((equal? op "*") 2)
      ((equal? op "/") 2)
      ((equal? op "^") 3)
      (else 0))))

;; Check if a token is an operator
(define operator? 
  (lambda (token)
    (or (equal? token "+")
        (equal? token "-")
        (equal? token "*")
        (equal? token "/")
        (equal? token "^"))))

;; Find matching closing parenthesis - recursive version
(define find-matching-paren
  (lambda (tokens start)
    (define find-match
      (lambda (pos depth)
        (cond
          ((>= pos (length tokens)) -1)
          ((equal? (list-ref tokens pos) "(") 
           (find-match (+ pos 1) (+ depth 1)))
          ((equal? (list-ref tokens pos) ")")
           (if (= depth 1) 
               pos
               (find-match (+ pos 1) (- depth 1))))
          (else (find-match (+ pos 1) depth)))))
    (find-match (+ start 1) 1)))

;; Find the operator with lowest precedence - recursive version
(define find-lowest-precedence-op
  (lambda (tokens)
    (define find-op
      (lambda (pos lowest-idx lowest-prec paren-level)
        (cond
          ((>= pos (length tokens)) lowest-idx)
          
          ((equal? (list-ref tokens pos) "(")
           (let ((match (find-matching-paren tokens pos)))
             (if (> match pos)
                 (find-op (+ match 1) lowest-idx lowest-prec paren-level)
                 (find-op (+ pos 1) lowest-idx lowest-prec (+ paren-level 1)))))
          
          ((equal? (list-ref tokens pos) ")")
           (find-op (+ pos 1) lowest-idx lowest-prec (- paren-level 1)))
          
          ((and (= paren-level 0)
                (operator? (list-ref tokens pos))
                (or (= lowest-idx -1)
                    (<= (op-precedence (list-ref tokens pos)) lowest-prec)))
           (find-op (+ pos 1) pos (op-precedence (list-ref tokens pos)) paren-level))
          
          (else (find-op (+ pos 1) lowest-idx lowest-prec paren-level)))))
    (find-op 0 -1 999 0)))

;; Update process-tokens to use eval-op
(define process-tokens
  (lambda (tokens)
    (cond
      ;; Empty expression
      ((null? tokens) '())
      
      ;; Single token
      ((= (length tokens) 1) 
       (process-token (car tokens)))
      
      ;; Parenthesized expression
      ((and (equal? (car tokens) "(")
            (let ((closing (find-matching-paren tokens 0)))
              (and (> closing 0) 
                   (= closing (- (length tokens) 1)))))
       (process-tokens (sublist tokens 1 (- (length tokens) 1))))
      
      ;; Process by operator precedence
      (else
        (let ((op-pos (find-lowest-precedence-op tokens)))
          (if (>= op-pos 0)
              (let ((op (list-ref tokens op-pos))
                    (left (process-tokens (sublist tokens 0 op-pos)))
                    (right (process-tokens (sublist tokens (+ op-pos 1) (length tokens)))))
                (if (equal? op "^")
                    (expt-op left right)  ; Special handling for exponentiation
                    (list (string->symbol op) left right)))
              tokens))))))

;; Process a token into a value or symbol
(define process-token
  (lambda (token)
    (if (string->number token)
        (string->number token)
        (string->symbol token))))

;; Simple infix to prefix converter
(define infix->prefix
  (lambda (expr)
    (if (not (string? expr))
        (begin
          (display "Error: infix->prefix requires a string argument\n")
          '())
        (process-tokens (tokenize-expr expr)))))

;; Exponentiation operation - handles both numeric and symbolic cases
(define expt-op
  (lambda (base exp)
    (cond
      ;; Integer exponents can be computed exactly
      ((integer? exp) 
       (if (and (integer? base) (>= exp 0))
           (expt base exp)  ; Use built-in expt function for all cases
           (expt base exp)))
      
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

;; Add a variable? predicate to check if something is a variable
(define variable?
  (lambda (expr)
    (and (symbol? expr) 
         (not (irrational? expr))
         (not (complex? expr)))))

;; Helper function to check for multiplication by zero pattern
(define multiply-by-zero?
  (lambda (expr)
    (and (list? expr)
         (= (length expr) 3)
         (eq? (car expr) '*)
         (or (and (number? (cadr expr)) (= (cadr expr) 0))
             (and (number? (caddr expr)) (= (caddr expr) 0))))))

;; Improved simplification function
(define simplify
  (lambda (expr)
    (cond
      ;; Base cases
      ((constant? expr) expr)
      ((variable? expr) expr)
      ((symbol? expr) expr)
      ((not (list? expr)) expr)
      ((null? expr) expr)
      
      ;; Early pattern matching for special cases
      ((multiply-by-zero? expr) 0)
      
      ;; Recursively process lists
      ((list? expr)
       (let ((op (car expr))
             (simplified-args (map simplify (cdr expr))))
         (cond
           ;; Addition
           ((eq? op '+)
            (cond
              ((= (length simplified-args) 0) 0)
              ((= (length simplified-args) 1) (car simplified-args))
              ((and (= (length simplified-args) 2)
                    (number? (car simplified-args))
                    (number? (cadr simplified-args)))
               (+ (car simplified-args) (cadr simplified-args)))
              ((and (= (length simplified-args) 2)
                    (or (and (number? (car simplified-args)) (= (car simplified-args) 0))
                        (and (number? (cadr simplified-args)) (= (cadr simplified-args) 0))))
               (if (and (number? (car simplified-args)) (= (car simplified-args) 0))
                   (cadr simplified-args)
                   (car simplified-args)))
              (else (cons '+ simplified-args))))
           
           ;; Multiplication
           ((eq? op '*)
            (cond
              ((= (length simplified-args) 0) 1)
              ((= (length simplified-args) 1) (car simplified-args))
              ;; Check again for zero pattern after simplifying arguments
              ((or (and (number? (car simplified-args)) (= (car simplified-args) 0))
                   (and (= (length simplified-args) 2) 
                        (number? (cadr simplified-args)) 
                        (= (cadr simplified-args) 0)))
               0)
              ((and (= (length simplified-args) 2)
                    (number? (car simplified-args))
                    (number? (cadr simplified-args)))
               (* (car simplified-args) (cadr simplified-args)))
              ((and (= (length simplified-args) 2)
                    (or (and (number? (car simplified-args)) (= (car simplified-args) 1))
                        (and (number? (cadr simplified-args)) (= (cadr simplified-args) 1))))
               (if (and (number? (car simplified-args)) (= (car simplified-args) 1))
                   (cadr simplified-args)
                   (car simplified-args)))
              (else (cons '* simplified-args))))
           
           ;; Handle exponentiation expressions
           ((eq? op 'expt)
            (cond
              ;; x^0 = 1 for any x
              ((and (= (length simplified-args) 2)
                    (number? (cadr simplified-args))
                    (= (cadr simplified-args) 0))
               1)
              
              ;; x^1 = x
              ((and (= (length simplified-args) 2)
                    (number? (cadr simplified-args))
                    (= (cadr simplified-args) 1))
               (car simplified-args))
              
              ;; 0^x = 0 (where x is positive)
              ((and (= (length simplified-args) 2)
                    (number? (car simplified-args))
                    (= (car simplified-args) 0)
                    (or (not (number? (cadr simplified-args)))  ;; symbolic exponent
                        (and (number? (cadr simplified-args))   ;; numeric positive exponent
                             (> (cadr simplified-args) 0))))
               0)
              
              ;; 1^x = 1 for any x
              ((and (= (length simplified-args) 2)
                    (number? (car simplified-args))
                    (= (car simplified-args) 1))
               1)
              
              ;; Numeric evaluation for constant expressions
              ((and (= (length simplified-args) 2)
                    (number? (car simplified-args))
                    (number? (cadr simplified-args))
                    (integer? (cadr simplified-args))
                    (>= (cadr simplified-args) 0))
               (expt (car simplified-args) (cadr simplified-args)))
              
              ;; Default: rebuilt expt expression
              (else (cons 'expt simplified-args))))
           
           ;; Other operators (keep the same structure)
           (else (cons op simplified-args)))))
      
      ;; Default case
      (else expr))))

;; Update eval-algebraic to include simplification
(define eval-algebraic
  (lambda (expr)
    (eval (simplify (infix->prefix expr)))))