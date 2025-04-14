; Comprehensive tests for built-in functions

; ----- List Functions Tests -----
(define-test "list-creation-1" (assert-equal (list 1 2 3) '(1 2 3)))
(define-test "list-creation-2" (assert-equal (list) '()))

(define-test "cons-functions-1" (assert-equal (cons 1 '(2 3)) '(1 2 3)))
(define-test "cons-functions-2" (assert-equal (cons 'a '()) '(a)))
(define-test "cons-functions-3" (assert-equal (car (cons 'x 'y)) 'x))
(define-test "cons-functions-4" (assert-equal (cdr (cons 'x 'y)) 'y))

(define-test "list-accessors-1" (assert-equal (car '(1 2 3)) 1))
(define-test "list-accessors-2" (assert-equal (cdr '(1 2 3)) '(2 3)))
(define-test "list-accessors-3" (assert-equal (cadr '(1 2 3)) 2))
(define-test "list-accessors-4" (assert-equal (caddr '(1 2 3)) 3))

(define-test "list-operations-1" (assert-equal (length '(1 2 3 4)) 4))
(define-test "list-operations-2" (assert-equal (length '()) 0))
(define-test "list-operations-3" (assert-equal (list-ref '(a b c d) 2) 'c))
(define-test "list-operations-4" (assert-equal (sublist '(a b c d e) 1 3) '(b c)))

; ----- Higher-order Function Tests -----
(define (square x) (* x x))
(define-test "map-function-1" (assert-equal (map square '(1 2 3)) '(1 4 9)))
(define-test "map-function-2" (assert-equal (map (lambda (x) (+ x 1)) '(0 1 2)) '(1 2 3)))
(define-test "map-function-3" (assert-equal (map car '((a b) (c d) (e f))) '(a c e)))

; ----- Predicate Tests -----
(define-test "type-predicates-1" (assert-equal (number? 42) #t))
(define-test "type-predicates-2" (assert-equal (number? 'symbol) #f))
(define-test "type-predicates-3" (assert-equal (integer? 5) #t))
(define-test "type-predicates-4" (assert-equal (integer? 1/2) #f))
(define-test "type-predicates-5" (assert-equal (string? "hello") #t))
(define-test "type-predicates-6" (assert-equal (string? 42) #f))
(define-test "type-predicates-7" (assert-equal (symbol? 'mysym) #t))
(define-test "type-predicates-8" (assert-equal (symbol? "string") #f))
(define-test "type-predicates-9" (assert-equal (boolean? #t) #t))
(define-test "type-predicates-10" (assert-equal (boolean? 0) #f))
(define-test "type-predicates-11" (assert-equal (pair? '(1 . 2)) #t))
(define-test "type-predicates-12" (assert-equal (pair? '(1 2)) #t))
(define-test "type-predicates-13" (assert-equal (pair? '()) #f))
(define-test "type-predicates-14" (assert-equal (function? car) #t))
(define-test "type-predicates-15" (assert-equal (function? 'car) #f))
(define-test "type-predicates-16" (assert-equal (function? (lambda (x) x)) #t))

(define-test "list-predicates-1" (assert-equal (list? '(1 2 3)) #t))
(define-test "list-predicates-2" (assert-equal (list? '()) #t))
(define-test "list-predicates-3" (assert-equal (list? '(1 . 2)) #f))
(define-test "list-predicates-4" (assert-equal (null? '()) #t))
(define-test "list-predicates-5" (assert-equal (null? '(1)) #f))
(define-test "list-predicates-6" (assert-equal (atom? 5) #t))
(define-test "list-predicates-7" (assert-equal (atom? 'symbol) #t))
(define-test "list-predicates-8" (assert-equal (atom? '(1 2)) #f))

(define-test "equality-predicates-1" (assert-equal (eq? 'a 'a) #t))
(define-test "equality-predicates-2" (assert-equal (eq? 'a 'b) #f))
(define-test "equality-predicates-3" (assert-equal (eq? '() '()) #t))
(define-test "equality-predicates-4" (assert-equal (equal? '(1 2 3) '(1 2 3)) #t))
(define-test "equality-predicates-5" (assert-equal (equal? '(1 (2 3)) '(1 (2 3))) #t))
(define-test "equality-predicates-6" (assert-equal (equal? '(1 2) '(1 3)) #f))

(define-test "logical-operations-1" (assert-equal (not #t) #f))
(define-test "logical-operations-2" (assert-equal (not #f) #t))
(define-test "logical-operations-3" (assert-equal (not '()) #f))  ; Empty list is not falsy in Scheme
(define-test "logical-operations-4" (assert-equal (and #t #t) #t))
(define-test "logical-operations-5" (assert-equal (and #t #f) #f))
(define-test "logical-operations-6" (assert-equal (and #f (/ 1 0)) #f))  ; Short-circuit evaluation
(define-test "logical-operations-7" (assert-equal (or #f #t) #t))
(define-test "logical-operations-8" (assert-equal (or #f #f) #f))
(define-test "logical-operations-9" (assert-equal (or #t (/ 1 0)) #t))  ; Short-circuit evaluation

; ----- Control Flow Tests -----
(define-test "if-special-form-1" (assert-equal (if #t 'yes 'no) 'yes))
(define-test "if-special-form-2" (assert-equal (if #f 'yes 'no) 'no))
(define-test "if-special-form-3" (assert-equal (if '() 'yes 'no) 'yes))  ; Non-empty list is truthy
(define-test "if-special-form-4" (assert-equal (if 0 'yes 'no) 'yes))   ; 0 is truthy in Scheme

(define-test "cond-special-form-1" (assert-equal (cond (#f 1) (#t 2) (else 3)) 2))
(define-test "cond-special-form-2" (assert-equal (cond (#f 1) (#f 2) (else 3)) 3))
(define-test "cond-special-form-3" (assert-equal (cond (#t 'first) (#t 'second)) 'first))

; Special case with side effects
(define-test "begin-special-form" 
  (begin
    (define x 1)
    (assert-equal (begin (set! x 2) (set! x (+ x 1)) x) 3)))

; ----- Environment Functions Tests -----
(define-test "environment-functions-1"
  (begin
    (define test-var 42)
    (assert-equal (member? 'test-var (env-symbols)) #t)))

(define-test "environment-functions-2"
  (begin
    (define test-var-2 43)
    (undef 'test-var-2)
    (assert-equal (member? 'test-var-2 (env-symbols)) #f)))

; ----- String and I/O Function Tests -----
(define-test "string-functions-1" (assert-equal (string-length "hello") 5))
(define-test "string-functions-3" (assert-equal (string-split "a,b,c" ",") '("a" "b" "c")))
(define-test "string-functions-4" (assert-equal (string-join '("a" "b" "c") ",") "a,b,c"))
(define-test "string-functions-5" (assert-equal (string->number "42") 42))
(define-test "string-functions-6" (assert-equal (number->string 42) "42"))
(define-test "string-functions-7" (assert-equal (string->symbol "xyz") 'xyz))
(define-test "string-functions-8" (assert-equal (read-from-string "(+ 1 2)") '(+ 1 2)))
(define-test "string-functions-9" (assert-equal (write-to-string '(1 2 3)) "(1 2 3)"))

; ----- Eval Function Test -----
(define-test "eval-function-1" (assert-equal (eval '(+ 1 2)) 3))
(define-test "eval-function-2" (assert-equal (eval (list '+ 2 3)) 5))