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

(define-test "eval-algebraic-expression" 
  (assert-equal (eval-algebraic "2*(3+4)")
                14))

;; Tests for simplification rules

(define-test "simplify-add-identity-left"
  (assert-equal (simplify '(+ 0 x)) x))

(define-test "simplify-add-identity-right"
  (assert-equal (simplify '(+ y 0)) y))

(define-test "simplify-multiply-identity-left"
  (assert-equal (simplify '(* 1 z)) z))

(define-test "simplify-multiply-identity-right"
  (assert-equal (simplify '(* a 1)) a))

(define-test "simplify-multiply-zero-left"
  (assert-equal (simplify '(* 0 b)) 0))

(define-test "simplify-multiply-zero-right"
  (assert-equal (simplify '(* c 0)) 0))

(define-test "simplify-expt-zero-exponent"
  (assert-equal (simplify '(expt a 0)) 1))

(define-test "simplify-expt-one-exponent"
  (assert-equal (simplify '(expt e 1)) 'e))

(define-test "simplify-expt-base-zero"
  (assert-equal (simplify '(expt 0 f)) 0))

(define-test "simplify-expt-base-one"
  (assert-equal (simplify '(expt 1 c)) 1))

(define-test "simplify-constant-folding-add"
  (assert-equal (simplify '(+ 3 4)) 7))

(define-test "simplify-constant-folding-multiply"
  (assert-equal (simplify '(* 3 4)) 12))

(define-test "simplify-constant-folding-expt"
  (assert-equal (simplify '(expt 2 3)) 8))

(define-test "simplify-nested-expression"
  (assert-equal (simplify '(+ (* 2 3) (* x 0))) 6))

;; Test expansion functionality

(define-test "expand-distribution-right"
  (assert-equal (expand '(* 2 (+ x y)))
                '(+ (* 2 x) (* 2 y))))

(define-test "expand-distribution-left"
  (assert-equal (expand '(* (+ x y) 2))
                '(+ (* x 2) (* y 2))))

(define-test "expand-nested-expression"
  (assert-equal (expand '(* 2 (+ x (+ y z))))
                '(+ (* 2 x) (* 2 y) (* 2 z))))

;; Test collection of like terms

(define-test "collect-like-terms-simple"
  (assert-equal (collect-like-terms '(+ (* 2 x) (* 3 x)))
                '(* 5 x)))

(define-test "collect-like-terms-mixed"
  (assert-equal (collect-like-terms '(+ (* 2 x) (* 3 y) (* 4 x)))
                '(+ (* 6 x) (* 3 y))))

(define-test "collect-like-terms-with-constants"
  (assert-equal (collect-like-terms '(+ (* 2 x) 3 (* 4 x) 5))
                '(+ (* 6 x) 8)))

;; Test integration of expansion and collection

(define-test "full-simplify-distribution-and-collection"
  (assert-equal (full-simplify '(* 2 (+ x (* 3 x))))
                '(* 8 x)))