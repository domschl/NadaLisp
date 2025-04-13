;; Basic list operations for NadaLisp

;; Append multiple lists together
(define append
  (lambda args
    (define append-two
      (lambda (lst1 lst2)
        (if (null? lst1)
            lst2
            (cons (car lst1) (append-two (cdr lst1) lst2)))))
    
    (define append-all
      (lambda (lists)
        (cond
          ;; No lists - return empty list
          ((null? lists) '())
          ;; One list left - return it
          ((null? (cdr lists)) (car lists))
          ;; Otherwise append first list with result of appending the rest
          (else (append-two (car lists) (append-all (cdr lists)))))))
    
    (append-all args)))

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

;; Filter a list using a predicate function
(define filter
  (lambda (pred lst)
    (cond
      ((null? lst) '())
      ((pred (car lst))
       (cons (car lst) (filter pred (cdr lst))))
      (else
       (filter pred (cdr lst))))))

;; Reduce a list using a binary function and initial value
(define reduce
  (lambda (func init lst)
    (if (null? lst)
        init
        (reduce func
                (func init (car lst))
                (cdr lst)))))
