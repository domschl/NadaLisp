(define-test "null-empty" (assert-equal (null? (quote ())) #t))

(define-test "null-list" (assert-equal (null? (quote (1 2 3))) #f))

(define-test "null-symbol" (assert-equal (null? (quote hello)) #f))

(define-test "integer-true" (assert-equal (integer? 42) #t))

(define-test "integer-false" (assert-equal (integer? 4.2) #f))

(define-test "integer-fraction" (assert-equal (integer? 1/2) #f))

(define-test "number-integer" (assert-equal (number? 42) #t))

(define-test "number-fraction" (assert-equal (number? 1/2) #t))

(define-test "number-string" (assert-equal (number? "42") #f))

(define-test "string-true" (assert-equal (string? "hello") #t))

(define-test "string-empty" (assert-equal (string? "") #t))

(define-test "string-symbol" (assert-equal (string? (quote hello)) #f))

(define-test "symbol-true" (assert-equal (symbol? (quote hello)) #t))

(define-test "symbol-string" (assert-equal (symbol? "hello") #f))

(define-test "boolean-true" (assert-equal (boolean? #t) #t))

(define-test "boolean-false" (assert-equal (boolean? #f) #t))

(define-test "boolean-symbol" (assert-equal (boolean? (quote true)) #f))

(define-test "pair-cons" 
  (assert-equal (pair? (cons 1 2)) #t))

(define-test "pair-list" 
  (assert-equal (pair? (quote (1 2 3))) #t))

(define-test "pair-empty-list" 
  (assert-equal (pair? (quote ())) #f))

(define-test "list-proper-list" 
  (assert-equal (list? (quote (1 2 3))) #t))

(define-test "list-empty" 
  (assert-equal (list? (quote ())) #t))

(define-test "list-dotted" 
  (assert-equal (list? (cons 1 2)) #f))

(define-test "function-lambda" 
  (assert-equal (function? (lambda (x) x)) #t))

(define-test "function-plus" 
  (assert-equal (function? +) #t))

(define-test "function-symbol" 
  (assert-equal (function? (quote plus)) #f))

(define-test "atom-number" 
  (assert-equal (atom? 42) #t))

(define-test "atom-symbol" 
  (assert-equal (atom? (quote hello)) #t))

(define-test "atom-string" 
  (assert-equal (atom? "hello") #t))

(define-test "atom-list" 
  (assert-equal (atom? (quote (1 2 3))) #f))

(define-test "atom-empty-list" 
  (assert-equal (atom? (quote ())) #f))

(define-test "builtin-plus" 
  (assert-equal (builtin? +) #t))

(define-test "builtin-lambda" 
  (assert-equal (builtin? (lambda (x) x)) #f))