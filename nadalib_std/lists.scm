;; Basic list operations for NadaLisp

;; Append multiple lists together
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
    
(define append
  (lambda args
    (append-all args)))

;; Reverse a list
  (define rev-helper
    (lambda (remaining result)
      (if (null? remaining)
          result
          (rev-helper (cdr remaining)
                      (cons (car remaining) result)))))

(define reverse
  (lambda (lst)
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

;; member: Check if an item is a member of a list
;; Returns the sublist starting with the first occurrence of the item if found, #f otherwise
(define member
  (lambda (item lst)
    (cond
      ;; Empty list - item not found
      ((null? lst) #f)
      ;; First element matches - return the list
      ((equal? item (car lst)) lst)
      ;; Otherwise, recursively check the rest
      (else (member item (cdr lst))))))

;; member?: Check if an item is a member of a list (boolean version)
(define member?
  (lambda (item lst)
    (if (member item lst) #t #f)))
    
;; Access functions for nested lists
(define caar
  (lambda (lst)
    (car (car lst))))

(define cdar
  (lambda (lst)
    (cdr (car lst))))

(define cddr
  (lambda (lst)
    (cdr (cdr lst))))

(define cdddr
  (lambda (lst)
    (cdr (cdr (cdr lst)))))

;;(define cadr  (in core)
;;(define caddr (in core)

(define set-car!
  (lambda (lst new-car)
    (set! lst (cons new-car (cdr lst)))))

(define set-cdr!
  (lambda (lst new-cdr)
    (set! lst (cons (car lst) new-cdr))))

;; Move fold-left from algebraic.scm to here
(define fold-left
  (lambda (f init lst)
    (if (null? lst)
        init
        (fold-left f (f init (car lst)) (cdr lst)))))

;; Move assoc from algebraic.scm to here
(define assoc
  (lambda (key alist)
    (cond
      ((null? alist) #f)
      ((equal? key (caar alist)) (car alist))
      (else (assoc key (cdr alist))))))

;; Concatenate multiple strings together
(define string-append
  (lambda args
    (string-join args "")))

(define sort  ;; quicksort
  (lambda (lst)
    (if (null? lst)
        '()
        (let ((pivot (car lst))
              (rest (cdr lst)))
          (append 
            (sort (filter (lambda (x) (< x pivot)) rest))
            (list pivot)
            (sort (filter (lambda (x) (>= x pivot)) rest)))))))

(define symsort  ;; quicksort
  (lambda (lst)
    (if (null? lst)
        '()
        (let ((pivot (write-to-string (car lst)))
              (rest (cdr lst)))
          (append 
            (symsort (filter (lambda (x) (< (write-to-string x) pivot)) rest))
            (list pivot)
            (symsort (filter (lambda (x) (>= (write-to-string x) pivot)) rest)))))))
