(define-test "nested-list-creation"
  (assert-equal 
    (list 1 (list 2 (list 3 4) 5) (list 6 7))
    (quote (1 (2 (3 4) 5) (6 7)))))

(define-test "deep-list-access"
  (begin
    (define data (quote (1 (2 (3 4) 5) (6 7))))
    (assert-equal (car (car (cdr data))) 2)
    (assert-equal (car (car (cdr (car (cdr data))))) 3)))

(define-test "tree-manipulation"
  (begin
    (define tree (list (list 1 2) (list 3 4)))
    (define update-leaf 
      (lambda (t row col val)
        (if (= row 0)
            (cons (if (= col 0)
                      (cons val (cdr (car t)))
                      (cons (car (car t)) (list val)))
                  (cdr t))
            (cons (car t) 
                  (list (if (= col 0)
                            (cons val (cdr (car (cdr t))))
                            (cons (car (car (cdr t))) (list val))))))))
    
    (assert-equal (update-leaf tree 0 0 9) (list (list 9 2) (list 3 4)))
    (assert-equal (update-leaf tree 0 1 9) (list (list 1 9) (list 3 4)))
    (assert-equal (update-leaf tree 1 0 9) (list (list 1 2) (list 9 4)))
    (assert-equal (update-leaf tree 1 1 9) (list (list 1 2) (list 3 9)))))

(define-test "functional-map"
  (begin
    (define map
      (lambda (f lst)
        (if (null? lst)
            (quote ())
            (cons (f (car lst))
                  (map f (cdr lst))))))
    
    (assert-equal (map (lambda (x) (+ x 1)) (list 1 2 3))
                  (list 2 3 4))))

(define-test "functional-filter"
  (begin
    (define filter
      (lambda (pred lst)
        (if (null? lst)
            (quote ())
            (if (pred (car lst))
                (cons (car lst) (filter pred (cdr lst)))
                (filter pred (cdr lst))))))
    
    (assert-equal (filter (lambda (x) (> x 2)) (list 1 2 3 4 5))
                  (list 3 4 5))))

(define-test "functional-reduce"
  (begin
    (define reduce
      (lambda (f init lst)
        (if (null? lst)
            init
            (f (car lst) (reduce f init (cdr lst))))))
    
    (assert-equal (reduce + 0 (list 1 2 3 4 5)) 15)
    (assert-equal (reduce * 1 (list 1 2 3 4 5)) 120)))