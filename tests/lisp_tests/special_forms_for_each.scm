; Tests for for-each special form implementation

; ----- Basic For-each Tests -----

(define-test "for-each-basic-single-list"
  (begin
    (define sum 0)
    (for-each (lambda (x) (set! sum (+ sum x))) '(1 2 3 4 5))
    (assert-equal sum 15)))

(define-test "for-each-empty-list"
  (begin
    (define was-called 0)
    (for-each (lambda (x) (set! was-called 1)) '())
    (assert-equal was-called 0)))

(define-test "for-each-return-value"
  ; for-each should return an unspecified value (nil in NadaLisp)
  (assert-equal (null? (for-each (lambda (x) x) '(1 2 3))) #t))

; ----- Multiple List Tests -----

(define-test "for-each-multiple-lists-same-length"
  (begin
    (define result '())
    (for-each 
      (lambda (x y) 
        (set! result (cons (+ x y) result)))
      '(1 2 3) 
      '(4 5 6))
    ; Note: result will be in reverse order due to cons
    (assert-equal result '(9 7 5))))

(define-test "for-each-multiple-lists-different-length"
  (begin
    (define result '())
    (for-each 
      (lambda (x y) 
        (set! result (cons (+ x y) result)))
      '(1 2 3 4 5) 
      '(10 20 30))
    ; Should stop at the shortest list length
    (assert-equal result '(33 22 11))))

; ----- Complex Usage Tests -----

(define-test "for-each-with-defined-procedure"
  (begin
    (define elements '())
    (define (record-element x)
      (set! elements (cons x elements)))
    (for-each record-element '(a b c d))
    (assert-equal elements '(d c b a))))

(define-test "for-each-with-nested-lists"
  (begin
    (define total 0)
    (for-each 
      (lambda (sublist) 
        (for-each 
          (lambda (x) (set! total (+ total x))) 
          sublist))
      '((1 2) (3 4) (5 6)))
    (assert-equal total 21)))

(define-test "for-each-modifying-external-structure"
  (begin
    (define items '((name "apple" count 0)
                    (name "banana" count 0)
                    (name "cherry" count 0)))
    
    (define (increment-item name)
      (set! items
        (map 
          (lambda (item)
            (if (equal? (cadr item) name)
                (list 'name (cadr item) 'count (+ 1 (car (cdddr item))))
                item))
          items)))
    
    (increment-item "banana")
    (increment-item "cherry")
    (increment-item "cherry")
    
    (assert-equal 
      (map (lambda (item) (car (cdddr item))) items)
      '(0 1 2))))

(define-test "for-each-with-map-combination"
  (begin
    (define result '())
    (for-each
      (lambda (x) (set! result (cons x result)))
      (map (lambda (x) (* x x)) '(1 2 3 4)))
    (assert-equal result '(16 9 4 1))))