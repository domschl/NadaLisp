;; Tests for the type predicates - rational numbers

(define-test "rational-predicate-integer"
  (assert-equal (rational? 42) #t))

(define-test "rational-predicate-negative"
  (assert-equal (rational? -7) #t))

(define-test "rational-predicate-zero"
  (assert-equal (rational? 0) #t))

(define-test "rational-predicate-fraction"
  (assert-equal (rational? 3/4) #t))

(define-test "rational-predicate-pi"
  (assert-equal (rational? 'pi) #f))

(define-test "rational-predicate-e"
  (assert-equal (rational? 'e) #f))

(define-test "rational-predicate-i"
  (assert-equal (rational? 'i) #f))

(define-test "rational-predicate-symbol"
  (assert-equal (rational? 'x) #f))

;; Tests for fraction predicate

(define-test "fraction-predicate-true"
  (assert-equal (fraction? 3/4) #t))

(define-test "fraction-predicate-negative"
  (assert-equal (fraction? -1/2) #t))

(define-test "fraction-predicate-integer"
  (assert-equal (fraction? 5) #f))

(define-test "fraction-predicate-pi"
  (assert-equal (fraction? 'pi) #f))

;; Tests for irrational predicate

(define-test "irrational-predicate-pi"
  (assert-equal (irrational? 'pi) #t))

(define-test "irrational-predicate-e"
  (assert-equal (irrational? 'e) #t))

(define-test "irrational-predicate-i"
  (assert-equal (irrational? 'i) #f))

(define-test "irrational-predicate-fraction"
  (assert-equal (irrational? 3/4) #f))

;; Tests for complex predicate

(define-test "complex-predicate-i"
  (assert-equal (complex? 'i) #t))

(define-test "complex-predicate-pi"
  (assert-equal (complex? 'pi) #f))

(define-test "complex-predicate-rational"
  (assert-equal (complex? 3/4) #f))

;; Tests for constant predicate

(define-test "constant-predicate-integer"
  (assert-equal (constant? 5) #t))

(define-test "constant-predicate-fraction"
  (assert-equal (constant? 3/4) #t))

(define-test "constant-predicate-pi"
  (assert-equal (constant? 'pi) #t))

(define-test "constant-predicate-e"
  (assert-equal (constant? 'e) #t))

(define-test "constant-predicate-i"
  (assert-equal (constant? 'i) #t))

(define-test "constant-predicate-variable"
  (assert-equal (constant? 'x) #f))

;; Tests for operator precedence and tokenization

(define-test "tokenize-simple-expression"
  (assert-equal (tokenize-expr "1+2*3")
                (list "1" "+" "2" "*" "3")))

(define-test "tokenize-with-parentheses"
  (assert-equal (tokenize-expr "(a+b)*(c-d)")
                (list "(" "a" "+" "b" ")" "*" "(" "c" "-" "d" ")")))

(define-test "infix-to-prefix-simple"
  (assert-equal (infix->prefix "1+2*3")
                '(+ 1 (* 2 3))))

(define-test "infix-to-prefix-with-parentheses"
  (assert-equal (infix->prefix "(1+2)*3")
                '(* (+ 1 2) 3)))

(define-test "eval-algebraic-simple" 
  (assert-equal (eval-algebraic "2*(3+4)")
                14))

;; Tests for function call conversions

(define-test "infix-to-prefix-simple-function-call"
  (assert-equal (infix->prefix "f(x)")
                '(f x)))

(define-test "infix-to-prefix-function-with-constant"
  (assert-equal (infix->prefix "f(5)")
                '(f 5)))

(define-test "infix-to-prefix-function-with-expression"
  (assert-equal (infix->prefix "f(x+y)")
                '(f (+ x y))))

(define-test "infix-to-prefix-nested-function-calls"
  (assert-equal (infix->prefix "f(g(x))")
                '(f (g x))))

(define-test "infix-to-prefix-function-with-complex-expression"
  (assert-equal (infix->prefix "f(x*y+z)")
                '(f (+ (* x y) z))))

(define-test "infix-to-prefix-function-in-expression"
  (assert-equal (infix->prefix "a+f(x)")
                '(+ a (f x))))

(define-test "infix-to-prefix-complex-with-function"
  (assert-equal (infix->prefix "3+4*f(x)")
                '(+ 3 (* 4 (f x)))))

(define-test "infix-to-prefix-function-with-parenthesized-expr"
  (assert-equal (infix->prefix "f((x+y)*z)")
                '(f (* (+ x y) z))))

(define-test "infix-to-prefix-functions-with-operators"
  (assert-equal (infix->prefix "f(x)+g(y)")
                '(+ (f x) (g y))))