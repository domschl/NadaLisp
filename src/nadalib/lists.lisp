;; Basic list operations for NadaLisp

;; Reverse a list
(define reverse
  (lambda (lst)
    (define rev-helper
      (lambda (remaining result)
        (if (null? remaining)
            result
            (rev-helper (cdr remaining)
                       (cons (car remaining) result)))))
    (rev-helper lst '())))