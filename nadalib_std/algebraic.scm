;; Symbolic algebra library for NadaLisp
;; Provides infix notation and symbolic manipulation

(define symbolic-expt 'symbolic-expt)

;; Purely functional algebraic notation utilities

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

;; Directly handle common special cases in sqrt function
(define sqrt
  (lambda (x)
    (cond
      ;; Special cases we can compute exactly
      ((= x 0) 0)
      ((= x 1) 1)
      ((= x 4) 2)
      ((= x 9) 3)
      ((= x 16) 4)
      ((= x 25) 5)
      ((= x 36) 6)
      ((= x 49) 7)
      ((= x 64) 8)
      ((= x 81) 9)
      ((= x 100) 10)
      ;; For other cases, create a symbolic representation
      (else (list 'sqrt x)))))

;; Similarly for cube root
(define cbrt
  (lambda (x)
    (cond
      ((= x 0) 0)
      ((= x 1) 1)
      ((= x 8) 2)
      ((= x 27) 3)
      ((= x 64) 4)
      ((= x 125) 5)
      ;; For other cases, create a symbolic representation
      (else (list 'cbrt x)))))

;; Modified expt-op to handle more cases
(define expt-op
  (lambda (base exp)
    (cond
      ;; If base is a list (like result of an operation), evaluate it first
      ((and (list? base) (not (eq? (car base) 'symbolic-expt)))
       (expt-op (eval base) exp))
      
      ;; If exponent is a list, evaluate it first
      ((and (list? exp) (not (eq? (car exp) 'symbolic-expt)))
       (expt-op base (eval exp)))
      
      ;; Now we can assume base and exp are either numbers, symbols, or symbolic expressions
      ((not (number? base))
       (list 'symbolic-expt base exp))
      
      ((not (number? exp))
       (list 'symbolic-expt base exp))
      
      ;; Integer exponents can be computed exactly
      ((integer? exp) 
       (expt base exp))
      
      ;; Common fractional exponents
      ((= exp 1/2) (sqrt base))
      ((= exp 1/3) (cbrt base))
      
      ;; For other non-integer exponents, use symbolic form
      (else (list 'symbolic-expt base exp)))))

;; Update the operator list to handle ^
(define eval-op
  (lambda (op left right)
    (case op
      ((+) (+ left right))
      ((-) (- left right))
      ((*) (* left right))
      ((/) (/ left right))
      ((^) (expt-op left right))
      (else (display (string-append "Unknown operator: " (symbol->string op) "\n"))))))

;; Update process-tokens to properly handle nested expressions
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
       ;; Process what's inside the parentheses
       (process-tokens (sublist tokens 1 (- (length tokens) 1))))
      
      ;; Process by operator precedence
      (else
        (let ((op-pos (find-lowest-precedence-op tokens)))
          (if (>= op-pos 0)
              (let ((op (list-ref tokens op-pos))
                    (left (process-tokens (sublist tokens 0 op-pos)))
                    (right (process-tokens (sublist tokens (+ op-pos 1) (length tokens)))))
                ;; Create a proper list for all operators
                (list (string->symbol op) left right))
              tokens))))))

;; Evaluate an algebraic expression
(define eval-algebraic
  (lambda (expr)
    (let ((result (infix->prefix expr)))
      (define eval-expr
        (lambda (expr)
          (cond
            ;; Atomic values (numbers, symbols)
            ((or (number? expr) (symbol? expr)) expr)
            
            ;; Empty list
            ((null? expr) '())
            
            ;; Handle special case for exponentiation with non-integer exponent
            ((and (list? expr) (eq? (car expr) '^))
             (let ((base (eval-expr (cadr expr)))
                   (exp (eval-expr (caddr expr))))
               (expt-op base exp)))
            
            ;; Regular expression evaluation - recursively evaluate all parts
            ((list? expr)
             (let ((op (car expr))
                   (args (map eval-expr (cdr expr))))
               (cond
                 ;; Handle special operators directly
                 ((eq? op '+) (+ (car args) (cadr args)))
                 ((eq? op '-) (- (car args) (cadr args)))
                 ((eq? op '*) (* (car args) (cadr args)))
                 ((eq? op '/) (/ (car args) (cadr args)))
                 ((eq? op '^) (expt-op (car args) (cadr args)))
                 ;; For other operators, try to use eval-op
                 (else (eval-op op (car args) (cadr args))))))
            
            ;; Default case
            (else expr))))
      
      ;; Evaluate the expression
      (eval-expr result))))

;; Define numerator and denominator if they don't already exist
(define numerator
  (lambda (q)
    (if (integer? q)
        q
        (let ((str (number->string q)))
          (string->number (car (string-split str "/")))))))

(define denominator
  (lambda (q)
    (if (integer? q)
        1
        (let ((str (number->string q)))
          (let ((parts (string-split str "/")))
            (if (= (length parts) 2)
                (string->number (cadr parts))
                1))))))

(define display-algebraic
  (lambda (expr)
    (cond
      ((null? expr) "")
      ((number? expr) (number->string expr))
      ((symbol? expr) (symbol->string expr))
      ((and (list? expr) (= (length expr) 3) (eq? (car expr) 'symbolic-expt))
       (string-append 
         (display-algebraic (cadr expr)) 
         "^" 
         (display-algebraic (caddr expr))))
      (else (display expr)))))