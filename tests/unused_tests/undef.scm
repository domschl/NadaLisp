; ----- Environment Functions Tests -----
(define-test "environment-functions"
  (define test-var 42)
  (assert-equal (env-symbols) (lambda (syms) 
                               (member 'test-var syms)))
  ; Cannot easily test env-describe directly since it outputs to stdout
  (undef 'test-var)
; CMakes CTest bails out on this without disabling global symbol lookup errors!
  (assert-equal (eval 'test-var 
                     (lambda () 'undefined)
                     (lambda (val) val)) 'undefined))
