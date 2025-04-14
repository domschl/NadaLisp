;; Test 1: Basic apply functionality
(define-test "apply-basic" 
  (assert-equal (apply + '(10 5)) 15))

;; Test 2: Direct access to operators in op-list
(define-test "op-list-access"
  (begin
    (define op-list (list + - * /))
    (assert-equal (car op-list) +)))

;; Test 3: Apply with an element from op-list
(define-test "apply-with-op-list"
  (begin
    (define op-list (list + - * /))
    (assert-equal (apply (car op-list) '(10 5)) 15)))

;; Test 4: Using lambda directly with apply
(define-test "lambda-with-apply"
  (assert-equal ((lambda (op args) (apply op args)) + '(10 5)) 15))

;; Test 5: Map with a simple function
(define-test "map-simple"
  (assert-equal (map (lambda (x) (* x 2)) '(1 2 3)) '(2 4 6)))

;; Test 6: Map with apply on a single list
(define-test "map-with-apply-single"
  (begin
    (define op-list (list + - * /))
    (assert-equal (map (lambda (op) (apply op '(10 5))) op-list) 
                  '(15 5 50 2))))

;; Test 7: Simplified version with just two operators
(define-test "map-apply-subset"
  (begin
    (define ops (list + -))
    (define args '((10 5) (10 5)))
    (assert-equal (map (lambda (op args) (apply op args)) ops args)
                  '(15 5))))

;; Test 8: Print operator values for debugging
(define-test "debug-operators"
  (begin
    (define op-list (list + - * /))
    (define op (car op-list))
    (assert-equal (procedure? op) #t)  ; Check if op is a procedure 
    (assert-equal (apply + '(10 5)) 15)  ; Regular apply works
    (assert-equal (apply op '(10 5)) 15)))  ; Apply with op from list

;; Test 1: Check identity mapping of operators
(define-test "map-identity-operators"
  (begin
    (define op-list (list + - * /))
    (define result (map (lambda (x) x) op-list))
    (assert-equal (car result) +)))

;; Test 2: Print type of operators inside map
(define-test "map-print-operator-type"
  (begin
    (define op-list (list + - * /))
    (define result (map (lambda (x) (procedure? x)) op-list))
    (display result)
    (assert-equal result '(#t #t #t #t))))

;; Test 3: Use apply directly on captured operator
(define-test "map-direct-apply"
  (begin
    (define op-list (list + - * /))
    (define result (map (lambda (x) 
                          (if (procedure? x)  ; Check if it's a procedure/function
                              (apply x '(10 5)) ; If so, apply it to arguments
                              "not-function")) 
                        op-list))
    (display result)
    (assert-equal result '(15 5 50 2))))

;; Test 4: Extract and use operators one at a time
(define-test "step-by-step-apply"
  (begin
    (define op-list (list + - * /))
    (define op1 (car op-list))
    (define result1 (apply op1 '(10 5)))
    (assert-equal result1 15)))