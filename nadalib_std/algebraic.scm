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

;; Evaluate an algebraic expression
(define eval-algebraic
  (lambda (expr)
    (eval (infix->prefix expr))))