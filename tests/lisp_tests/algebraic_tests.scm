(define-test "tokenize-simple-expression"
  (assert-equal (tokenize-expr "1+2*3")
                (list "1" "+" "2" "*" "3")))

(define-test "tokenize-with-parentheses"
  (assert-equal (tokenize-expr "(a+b)*(c-d)")
                (list "(" "a" "+" "b" ")" "*" "(" "c" "-" "d" ")")))

