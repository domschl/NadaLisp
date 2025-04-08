;; Basic algebraic notation utilities

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

;; Simple infix->prefix converter for basic expressions
(define infix->prefix
  (lambda (expr)
    (if (not (string? expr))
        (begin
          (display "Error: infix->prefix requires a string argument\n")
          '())
        (let ((tokens (tokenize-expr expr)))
          (cond
            ;; Empty expression
            ((null? tokens) '())
            
            ;; Single token (number or variable)
            ((= (length tokens) 1) 
             (let ((token (car tokens)))
               (if (string->number token)
                   (string->number token)
                   (string->symbol token))))
            
            ;; Simple binary operation: a op b
            ((= (length tokens) 3) 
             (list (string->symbol (cadr tokens)) 
                   (if (string->number (car tokens))
                       (string->number (car tokens))
                       (string->symbol (car tokens)))
                   (if (string->number (caddr tokens))
                       (string->number (caddr tokens))
                       (string->symbol (caddr tokens)))))
            
            ;; Expression with parentheses
            ((and (equal? (car tokens) "(")
                  (equal? (list-ref tokens (- (length tokens) 1)) ")"))
             (let ((inner-expr (string-join (sublist tokens 1 (- (length tokens) 1)) " ")))
               (infix->prefix inner-expr)))
            
            ;; Basic operator precedence handling
            (else
              (let ((op-pos (index-of tokens "*")))
                (if (>= op-pos 0)
                    ;; Handle multiplication first
                    (let ((left (sublist tokens 0 op-pos))
                          (right (sublist tokens (+ op-pos 1) (length tokens))))
                      (list '* 
                            (infix->prefix (string-join left " "))
                            (infix->prefix (string-join right " "))))
                    ;; Try addition
                    (let ((op-pos (index-of tokens "+")))
                      (if (>= op-pos 0)
                          (let ((left (sublist tokens 0 op-pos))
                                (right (sublist tokens (+ op-pos 1) (length tokens))))
                            (list '+ 
                                  (infix->prefix (string-join left " "))
                                  (infix->prefix (string-join right " "))))
                          tokens))))))))))

;; Helper: Find index of element in list
(define index-of
  (lambda (lst item)
    (define helper
      (lambda (l pos)
        (cond
          ((null? l) -1)
          ((equal? (car l) item) pos)
          (else (helper (cdr l) (+ pos 1))))))
    (helper lst 0)))

;; Evaluate an algebraic expression
(define eval-algebraic
  (lambda (expr)
    (eval (infix->prefix expr))))