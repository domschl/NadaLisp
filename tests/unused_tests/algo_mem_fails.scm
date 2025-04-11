(define-test "infix-to-prefix-simple"
  (assert-equal (infix->prefix "1+2*3")
                '(+ 1 (* 2 3))))

(define-test "infix-to-prefix-with-parentheses"
  (assert-equal (infix->prefix "(1+2)*3")
                '(* (+ 1 2) 3)))

(define-test "eval-algebraic-expression" 
  (assert-equal (eval-algebraic "2*(3+4)")
                14))