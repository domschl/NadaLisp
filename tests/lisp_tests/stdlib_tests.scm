(define-test "let*-1"
    (assert-equal (let* '((x 1) (y (+ x 1))) '(+ x y)) '(let ((x 1)) (let ((y (+ x 1))) (+ x y))))
)

(define-test "let*-no-bindings"
    (assert-equal (let* '() '(+ 1 2)) '(+ 1 2))
)

(define-test "let*-single-binding"
    (assert-equal (let* '((x 10)) '(* x 2)) '(let ((x 10)) (* x 2)))
)

(define-test "let*-three-bindings"
    (assert-equal (let* '((a 1) (b (+ a 5)) (c (* b 2))) '(list a b c))
                  '(let ((a 1)) (let ((b (+ a 5))) (let ((c (* b 2))) (list a b c)))))
)

(define-test "let*-shadowing"
    ;; Tests if the transformation correctly nests, allowing inner bindings to shadow outer ones if evaluated.
    ;; The transformation itself shouldn't evaluate, just structure it correctly.
    (assert-equal (let* '((x 1) (x (+ x 1))) 'x)
                  '(let ((x 1)) (let ((x (+ x 1))) x)))
)

(define-test "let*-empty-body"
    (assert-equal (let* '((z 5)) '()) '(let ((z 5)) ()))
)

(define-test "let*-complex-expressions"
    (assert-equal (let* '((f (lambda (n) (* n n))) (val (f 5))) 'val)
                  '(let ((f (lambda (n) (* n n)))) (let ((val (f 5))) val)))
)
