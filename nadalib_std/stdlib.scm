;; General purpose functions for the
;; NADA programming language.

(define newline (display "\n"))

(define let*
  (lambda (bindings body)
    (if (null? bindings)
        body ;; Base case: no bindings left, just the body
        (list 'let (list (car bindings)) ;; Create a let for the first binding
              (let* (cdr bindings) body))))) ;; Recurse

;; letrec: Allows defining local recursive or mutually recursive functions.
;; Transforms (letrec ((var1 expr1) ...) body) into
;; (let ((var1 '#undefined) ...) (set! var1 expr1) ... body)
;; Requires 'set!' to be implemented and modify lexical bindings.
;; Also requires '#undefined or similar to be a valid, distinct value.
(define letrec
  (lambda (bindings body)
    (if (null? bindings)
        body ;; No bindings, just the body
        (let ((vars (map car bindings))
              (exprs (map cadr bindings)))
          ;; Create initial bindings with a dummy value (e.g., '())
          ;; Using '() as the dummy value might be safer if '#undefined isn't special
          (let ((initial-bindings (map (lambda (v) (list v ''())) vars)))
            ;; Create the sequence of set! expressions
            (let ((set-exprs (map (lambda (v e) (list 'set! v e)) vars exprs)))
              ;; Combine into the final let expression:
              ;; (let ((v1 '()) (v2 '()) ...) (set! v1 e1) (set! v2 e2) ... body)
              (cons 'let (cons initial-bindings (append set-exprs (list body))))))))))
