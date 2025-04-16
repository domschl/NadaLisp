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

;; Modified expansion function that properly handles symbols
(define expand
  (lambda (expr)
    (let ((expanded-expr (expand-step expr)))
      ;; Keep expanding until no more changes
      (if (equal? expanded-expr expr)
          expr
          (expand expanded-expr)))))

;; Helper function to protect from evaluation
(define protect-expr
  (lambda (expr)
    (cond
      ((number? expr) expr)
      ((symbol? expr) expr) 
      ((not (list? expr)) expr)
      ((null? expr) expr)
      ;; For operators that cause evaluation problems
      ((and (list? expr) (not (null? expr)) 
            (or (eq? (car expr) '*) 
                (eq? (car expr) '+) 
                (eq? (car expr) '-) 
                (eq? (car expr) '/)
                (eq? (car expr) 'expt)))
       (list 'quote expr))
      (else expr))))

;; Modified expand-step function that prevents evaluation
(define expand-step
  (lambda (expr)
    (cond
      ;; Base cases
      ((atomic? expr) expr)
      
      ;; Distribution pattern: a * (b + c) -> (a * b) + (a * c)
      ((and (list? expr) 
            (= (length expr) 3)
            (eq? (car expr) '*)
            (list? (caddr expr))
            (eq? (car (caddr expr)) '+))
       (let ((factor (expand-step (cadr expr)))
             (sum-terms (cdr (caddr expr))))
         ;; Build a list structure directly without evaluation
         (cons '+ 
               (map (lambda (term)
                      ;; Create the multiplication structure
                      (list '* factor (expand-step term)))
                    sum-terms))))
      
      ;; Distribution pattern: (a + b) * c -> (a * c) + (b * c)
      ((and (list? expr) 
            (= (length expr) 3)
            (eq? (car expr) '*)
            (list? (cadr expr))
            (eq? (car (cadr expr)) '+))
       (let ((sum-terms (cdr (cadr expr)))
             (factor (expand-step (caddr expr))))
         ;; Build a list structure directly without evaluation
         (cons '+ 
               (map (lambda (term)
                      ;; Create the multiplication structure
                      (list '* (expand-step term) factor))
                    sum-terms))))
      
      ;; Recursive case for other expressions
      ((list? expr)
       (cons (car expr) (map expand-step (cdr expr))))
      
      ;; Default case
      (else expr))))

;; Collection of like terms

;; Extract the coefficient and variable parts of a term
(define extract-parts
  (lambda (term)
    (cond
      ;; Constant term
      ((number? term) (list term 1))
      
      ;; Plain variable term (implicit coefficient 1)
      ((variable? term) (list 1 term))
      
      ;; Coefficient * variable: (* 3 x)
      ((and (list? term) 
            (= (length term) 3)
            (eq? (car term) '*)
            (number? (cadr term))
            (variable? (caddr term)))
       (list (cadr term) (caddr term)))
      
      ;; Variable * coefficient: (* x 3)
      ((and (list? term) 
            (= (length term) 3)
            (eq? (car term) '*) 
            (variable? (cadr term))
            (number? (caddr term)))
       (list (caddr term) (cadr term)))
      
      ;; More complex terms - powers, etc.
      (else (list 1 term)))))

;; Combine like terms in an addition expression
(define collect-like-terms
  (lambda (expr)
    (cond
      ;; Base cases
      ((atomic? expr) expr)
      
      ;; Addition expression
      ((and (list? expr) (eq? (car expr) '+))
       ;; Process each term and collect by variable
       (let ((terms (map collect-like-terms (cdr expr))))
         ;; Group terms by their variable part
         (let ((term-groups (group-by-variable terms)))
           ;; Convert grouped terms back to an expression
           (groups->expr term-groups))))
      
      ;; Other expressions: preserve structure but process recursively
      ((list? expr)
       (cons (car expr) (map collect-like-terms (cdr expr))))
      
      ;; Default case
      (else expr))))

;; Group terms by their variable part
(define group-by-variable
  (lambda (terms)
    (letrec ((helper 
               (lambda (terms groups)
                 (if (null? terms)
                     groups
                     (let* ((term (car terms))
                            (parts (extract-parts term))
                            (coef (car parts))
                            (var (cadr parts))
                            (existing (assoc var groups)))
                       (if existing
                           ;; Add to existing group
                           (helper (cdr terms)
                                  (map (lambda (g)
                                         (if (equal? (car g) var)
                                             (list var (+ (cadr g) coef))
                                             g))
                                       groups))
                           ;; Create new group
                           (helper (cdr terms)
                                  (cons (list var coef) groups))))))))
      (helper terms '()))))

;; Simplified groups->expr function
(define groups->expr
  (lambda (groups)
    (let ((terms (map (lambda (group)
                        (let ((var (car group))
                              (coef (cadr group)))
                          (cond
                            ;; Zero coefficient - term vanishes
                            ((= coef 0) #f)
                            ;; Coefficient 1 with variable
                            ((and (= coef 1) (not (eq? var 1)))
                             var)
                            ;; Just a constant
                            ((eq? var 1) coef)
                            ;; Regular case: coefficient * variable
                            (else (list '* coef var)))))
                      groups)))
      ;; Remove false values (zero coefficients)
      (let ((non-zero-terms (filter identity terms)))
        (cond
          ((null? non-zero-terms) 0)
          ((= (length non-zero-terms) 1) (car non-zero-terms))
          (else (cons '+ non-zero-terms)))))))

;; Helper identity function for filter
(define identity
  (lambda (x) x))

;; Debug version of safe-simplify-arg to trace execution
(define safe-simplify-arg
  (lambda (expr)
    (cond
      ((atomic? expr) expr)
      ((multiply-by-zero? expr) 0)
      ((and (list? expr) (not (null? expr)))
       (let ((op (car expr)))
         (if (and (eq? op '*) 
                  (= (length expr) 3)
                  (or (and (number? (cadr expr)) (= (cadr expr) 0))
                      (and (number? (caddr expr)) (= (caddr expr) 0))))
             0
             (let ((protected-args (map safe-simplify-arg (cdr expr))))
               (cond
                 ((and (eq? op '*)
                       (or (and (not (null? protected-args))
                                (number? (car protected-args))
                                (= (car protected-args) 0))
                           (and (= (length protected-args) 2)
                                (number? (cadr protected-args))
                                (= (cadr protected-args) 0))))
                  0)
                 (else (simplify (cons op protected-args))))))))
      (else (simplify expr)))))

;; Debug version of multiply-by-zero?
(define multiply-by-zero?
  (lambda (expr)
    (and (list? expr)
         (= (length expr) 3)
         (eq? (car expr) '*)
         (or (and (number? (cadr expr)) (= (cadr expr) 0))
             (and (number? (caddr expr)) (= (caddr expr) 0))))))

;; Modular simplification system

;; Main simplify function - entry point
(define simplify
  (lambda (expr)
    ;; Special direct pattern matching for nested zero multiplication
    (define direct-zero-mult?
      (lambda (e)
        (and (list? e) 
             (= (length e) 3)
             (eq? (car e) '*)
             (or (and (number? (cadr e)) (= (cadr e) 0))
                 (and (number? (caddr e)) (= (caddr e) 0))))))
    
    (cond
      ;; Base cases
      ((atomic? expr) expr)
      
      ;; Direct pattern matching for zero multiplication
      ((direct-zero-mult? expr) 0)
      
      ;; Addition with zero multiplication inside
      ((and (list? expr) 
            (eq? (car expr) '+)
            (= (length expr) 3)
            (or (direct-zero-mult? (cadr expr))
                (direct-zero-mult? (caddr expr))))
       (let ((left (if (direct-zero-mult? (cadr expr)) 
                      0 
                      (simplify (cadr expr))))
             (right (if (direct-zero-mult? (caddr expr))
                       0
                       (simplify (caddr expr)))))
         (simplify (list '+ left right))))
      
      ;; Regular simplification for other expressions
      ((and (list? expr) (not (null? expr)))
       (let ((op (car expr))
             (simplified-args (map simplify (cdr expr))))
         (cond
           ((eq? op '+) (simplify-addition simplified-args))
           ((eq? op '*) (simplify-multiplication simplified-args))
           ((eq? op 'expt) (simplify-exponentiation simplified-args))
           ((eq? op '-) (simplify-subtraction simplified-args))
           ((eq? op '/) (simplify-division simplified-args))
           (else (cons op simplified-args)))))
      
      ;; Default case
      (else expr))))

;; Helper predicates
(define atomic?
  (lambda (expr)
    (or (constant? expr)
        (variable? expr)
        (symbol? expr)
        (not (list? expr))
        (null? expr))))

;; Simplification rule for addition
(define simplify-addition
  (lambda (args)
    (cond
      ((null? args) 0)
      ((null? (cdr args)) (car args))
      
      ;; Constant folding
      ((and (= (length args) 2)
            (number? (car args))
            (number? (cadr args)))
       (+ (car args) (cadr args)))
       
      ;; x + 0 = x
      ((and (= (length args) 2)
            (or (and (number? (car args)) (= (car args) 0))
                (and (number? (cadr args)) (= (cadr args) 0))))
       (if (and (number? (car args)) (= (car args) 0))
           (cadr args)
           (car args)))
           
      ;; Default
      (else (cons '+ args)))))

;; Simplification rule for multiplication
(define simplify-multiplication
  (lambda (args)
    (cond
      ((null? args) 1)
      ((null? (cdr args)) (car args))
      
      ;; 0 * x = 0
      ((or (and (number? (car args)) (= (car args) 0))
           (and (= (length args) 2) 
                (number? (cadr args)) 
                (= (cadr args) 0)))
       0)
       
      ;; Constant folding
      ((and (= (length args) 2)
            (number? (car args))
            (number? (cadr args)))
       (* (car args) (cadr args)))
       
      ;; x * 1 = x
      ((and (= (length args) 2)
            (or (and (number? (car args)) (= (car args) 1))
                (and (number? (cadr args)) (= (cadr args) 1))))
       (if (and (number? (car args)) (= (car args) 1))
           (cadr args)
           (car args)))
           
      ;; Default
      (else (cons '* args)))))

;; Simplification rule for exponentiation
(define simplify-exponentiation
  (lambda (args)
    (cond
      ;; Not enough arguments
      ((< (length args) 2) (cons 'expt args))
      
      ;; Too many arguments
      ((> (length args) 2) (cons 'expt args))
      
      ;; x^0 = 1
      ((and (number? (cadr args)) (= (cadr args) 0)) 1)
      
      ;; x^1 = x
      ((and (number? (cadr args)) (= (cadr args) 1)) (car args))
      
      ;; 0^x = 0 (x > 0)
      ((and (number? (car args)) 
            (= (car args) 0)
            (or (not (number? (cadr args)))
                (and (number? (cadr args)) (> (cadr args) 0))))
       0)
       
      ;; 1^x = 1
      ((and (number? (car args)) (= (car args) 1)) 1)
      
      ;; Constant folding
      ((and (number? (car args))
            (number? (cadr args))
            (integer? (cadr args))
            (>= (cadr args) 0))
       (expt (car args) (cadr args)))
       
      ;; Default: keep symbolic form
      (else (list 'expt (car args) (cadr args))))))

;; Additional operation simplifiers can be added here
;; ...

;; Enhanced simplify function that includes expansion and collection
(define full-simplify
  (lambda (expr)
    (let ((result (simplify expr)))
      (let ((expanded (expand result)))
        (collect-like-terms expanded)))))

;; Update eval-algebraic to use full-simplify
(define eval-algebraic
  (lambda (expr)
    (eval (full-simplify (infix->prefix expr)))))
