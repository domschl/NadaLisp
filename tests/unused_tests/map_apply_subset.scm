(define-test "map-apply-subset"
  (begin
    (define ops (list + -))
    (define args '((10 5) (10 5)))
    (assert-equal (map (lambda (op arg) (apply op arg)) ops args)
                  '(15 5))))

(define-test "apply-nested"
  (begin
    (define op-list (list + - * /))
    (define arg-lists '((10 5) (10 5) (10 5) (10 5)))
    (assert-equal 
      (map (lambda (op args) (apply op args)) op-list arg-lists)
      '(15 5 50 2))))
