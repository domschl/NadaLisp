; Comprehensive tests for built-in functions

; ----- List Functions Tests -----
(define-test "list-creation" 
  (assert-equal (list 1 2 3) '(1 2 3))
  (assert-equal (list) '()))

(define-test "cons-functions" 
  (assert-equal (cons 1 '(2 3)) '(1 2 3))
  (assert-equal (cons 'a '()) '(a))
  (assert-equal (car (cons 'x 'y)) 'x)
  (assert-equal (cdr (cons 'x 'y)) 'y))

(define-test "list-accessors" 
  (assert-equal (car '(1 2 3)) 1)
  (assert-equal (cdr '(1 2 3)) '(2 3))
  (assert-equal (cadr '(1 2 3)) 2)
  (assert-equal (caddr '(1 2 3)) 3))

(define-test "list-operations" 
  (assert-equal (length '(1 2 3 4)) 4)
  (assert-equal (length '()) 0)
  (assert-equal (list-ref '(a b c d) 2) 'c)
  (assert-equal (sublist '(a b c d e) 1 3) '(b c)))

; ----- Higher-order Function Tests -----
(define-test "map-function" 
  (define (square x) (* x x))
  (assert-equal (map square '(1 2 3)) '(1 4 9))
  (assert-equal (map (lambda (x) (+ x 1)) '(0 1 2)) '(1 2 3))
  (assert-equal (map car '((a b) (c d) (e f))) '(a c e)))

; ----- Predicate Tests -----
(define-test "type-predicates" 
  (assert-equal (number? 42) #t)
  (assert-equal (number? 'symbol) #f)
  (assert-equal (integer? 5) #t)
  (assert-equal (integer? 1/2) #f)
  (assert-equal (string? "hello") #t)
  (assert-equal (string? 42) #f)
  (assert-equal (symbol? 'mysym) #t)
  (assert-equal (symbol? "string") #f)
  (assert-equal (boolean? #t) #t)
  (assert-equal (boolean? 0) #f)
  (assert-equal (pair? '(1 . 2)) #t)
  (assert-equal (pair? '(1 2)) #t)
  (assert-equal (pair? '()) #f)
  (assert-equal (function? car) #t)
  (assert-equal (function? 'car) #f)
  (assert-equal (function? (lambda (x) x)) #t))

(define-test "list-predicates"
  (assert-equal (list? '(1 2 3)) #t)
  (assert-equal (list? '()) #t)
  (assert-equal (list? '(1 . 2)) #f)
  (assert-equal (null? '()) #t)
  (assert-equal (null? '(1)) #f)
  (assert-equal (atom? 5) #t)
  (assert-equal (atom? 'symbol) #t)
  (assert-equal (atom? '(1 2)) #f))

(define-test "equality-predicates"
  (assert-equal (eq? 'a 'a) #t)
  (assert-equal (eq? 'a 'b) #f)
  (assert-equal (eq? '() '()) #t)
  (assert-equal (equal? '(1 2 3) '(1 2 3)) #t)
  (assert-equal (equal? '(1 (2 3)) '(1 (2 3))) #t)
  (assert-equal (equal? '(1 2) '(1 3)) #f))

(define-test "logical-operations"
  (assert-equal (not #t) #f)
  (assert-equal (not #f) #t)
  (assert-equal (not '()) #f)  ; Empty list is not falsy in Scheme
  (assert-equal (and #t #t) #t)
  (assert-equal (and #t #f) #f)
  (assert-equal (and #f (/ 1 0)) #f)  ; Short-circuit evaluation
  (assert-equal (or #f #t) #t)
  (assert-equal (or #f #f) #f)
  (assert-equal (or #t (/ 1 0)) #t))  ; Short-circuit evaluation

; ----- Control Flow Tests -----
(define-test "if-special-form"
  (assert-equal (if #t 'yes 'no) 'yes)
  (assert-equal (if #f 'yes 'no) 'no)
  (assert-equal (if '() 'yes 'no) 'yes)  ; Non-empty list is truthy
  (assert-equal (if 0 'yes 'no) 'yes))   ; 0 is truthy in Scheme

(define-test "cond-special-form"
  (assert-equal (cond (#f 1) (#t 2) (else 3)) 2)
  (assert-equal (cond (#f 1) (#f 2) (else 3)) 3)
  (assert-equal (cond (#t 'first) (#t 'second)) 'first))

(define-test "begin-special-form"
  (define x 1)
  (assert-equal (begin (set! x 2) (set! x (+ x 1)) x) 3))

; ----- Environment Functions Tests -----
(define-test "environment-functions"
  (define test-var 42)
  (assert-equal (env-symbols) (lambda (syms) 
                               (member 'test-var syms)))
  ; Cannot easily test env-describe directly since it outputs to stdout
  (undef 'test-var)

; ----- String and I/O Function Tests -----
(define-test "string-functions"
  (assert-equal (string-length "hello") 5)
  (assert-equal (substring "hello" 1 3) "el")
  (assert-equal (string-split "a,b,c" ",") '("a" "b" "c"))
  (assert-equal (string-join '("a" "b" "c") ",") "a,b,c")
  (assert-equal (string->number "42") 42)
  (assert-equal (number->string 42) "42")
  (assert-equal (string->symbol "xyz") 'xyz)
  ; For read/write-to-string, test with a simple expression
  (assert-equal (read-from-string "(+ 1 2)") '(+ 1 2))
  (assert-equal (write-to-string '(1 2 3)) "(1 2 3)"))

; ----- Eval Function Test -----
(define-test "eval-function"
  (assert-equal (eval '(+ 1 2)) 3)
  (assert-equal (eval (list '+ 2 3)) 5))