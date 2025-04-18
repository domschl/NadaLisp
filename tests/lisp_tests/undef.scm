; ----- Environment Functions Tests -----
(define test-var 42)
(define-test "environment-functions-1"
  (assert-equal (env-symbols) (lambda (syms) 
                               (member 'test-var syms))))
  ; Cannot easily test env-describe directly since it outputs to stdout
(undef 'test-var)
(define-test "environment-functions-1"
  (assert-equal (eval 'test-var 
                     (lambda () 'undefined)
                     (lambda (val) val)) 'undefined))

