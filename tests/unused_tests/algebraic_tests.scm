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