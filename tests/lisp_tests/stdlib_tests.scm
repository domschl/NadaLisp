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

;; --- letrec tests ---
;; Note: These tests assume 'letrec' transforms the code as intended,
;; and that the underlying interpreter can evaluate the resulting
;; 'let' and 'set!' structure correctly, including handling recursion.
;; The assert-equal checks the *transformed* structure.
;; Assumes the dummy value used in letrec is ''() (quoted empty list).

(define-test "letrec-no-bindings"
    (assert-equal (letrec '() '(+ 1 2))
                  '(+ 1 2))
)

(define-test "letrec-single-recursive-func"
    (assert-equal (letrec '((fact (lambda (n) (if (= n 0) 1 (* n (fact (- n 1))))))) '(fact 5))
                  '(let ((fact '()))
                     (set! fact (lambda (n) (if (= n 0) 1 (* n (fact (- n 1))))))
                     (fact 5)))
)

(define-test "letrec-mutual-recursion"
    (assert-equal (letrec '((is-even? (lambda (n) (if (= n 0) #t (is-odd? (- n 1)))))
                           (is-odd? (lambda (n) (if (= n 0) #f (is-even? (- n 1))))))
                          '(is-odd? 3))
                  '(let ((is-even? '()) (is-odd? '()))
                     (set! is-even? (lambda (n) (if (= n 0) #t (is-odd? (- n 1)))))
                     (set! is-odd? (lambda (n) (if (= n 0) #f (is-even? (- n 1)))))
                     (is-odd? 3)))
)

(define-test "letrec-non-function-binding"
    ;; While primarily for functions, letrec should technically handle other bindings.
    ;; The order of set! matters here if bindings depend on each other sequentially.
    (assert-equal (letrec '((x 10) (y (+ x 5))) '(list x y))
                  '(let ((x '()) (y '()))
                     (set! x 10)
                     (set! y (+ x 5)) ;; Relies on x being set in the previous step
                     (list x y)))
)

(define-test "letrec-empty-body"
    (assert-equal (letrec '((foo (lambda () 1))) '())
                  '(let ((foo '()))
                     (set! foo (lambda () 1))
                     ()))
)

(define-test "letrec-binding-referencing-later-binding"
    ;; Tests if the structure allows expr1 to reference var2 (classic letrec need)
    (assert-equal (letrec '((ping (lambda () (pong))) (pong (lambda () 'ponged))) '(ping))
                  '(let ((ping '()) (pong '()))
                     (set! ping (lambda () (pong)))
                     (set! pong (lambda () 'ponged))
                     (ping)))
)
