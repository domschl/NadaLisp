;; General purpose functions for the
;; NADA programming language.

(define newline (lambda () (display "\n")))
(define displayln
  (lambda (x)
    (display x)
    (newline)))
    
(define let*
  (lambda (bindings body)
    (if (null? bindings)
        body ;; Base case: no bindings left, just the body
        (list 'let (list (car bindings)) ;; Create a let for the first binding
              (let* (cdr bindings) body))))) ;; Recurse
