;;; NadaLisp Testing Library
;;; Provides testing utilities for writing and running tests

;; Initialize global test counters
(define tests-all-passed #t)
(define tests-run-count 0)
(define tests-passed-count 0)
(define tests-failed-count 0)

;; assert-equal: Compare two values for equality
;; Returns the result directly, not wrapped in a function
(define assert-equal
  (lambda (actual expected)
    (let ((passed (equal? actual expected)))
      (if passed
          (begin
           ; (display "  ASSERTION PASSED\n")
           ; (display "  Expected: ")
           ; (display expected)
           ; (display "\n  Got:      ")
           ; (display actual)
           ; (display "\n")
            #t)
          (begin
            (display "  ASSERTION FAILED\n")
            (display "  Expected: ")
            (display expected)
            (display "\n  Got:      ")
            (display actual)
            (display "\n")
            #f)))))

;; define-test: Define and run a named test case
;; Now supports multiple assertion expressions
(define define-test
  (lambda args
    ;; Extract the name and expressions
    (if (< (length args) 2)
        (begin
          (display "ERROR: define-test requires at least a name and one assertion\n")
          #f)
        (let ((name (car args))
              (assertions (cdr args)))
          
          ;; Increment the test counter
          (set! tests-run-count (+ tests-run-count 1))
          
          (display "Running test: ")
          (display name)
          (display "\n")
          
          ;; Track if all assertions pass
          (define all-passed #t)
          
          ;; Process each assertion
          (define process-assertions
            (lambda (remaining)
              (if (null? remaining)
                  all-passed
                  (let ((result (car remaining)))
                    (if (not result)
                        (set! all-passed #f))
                    (process-assertions (cdr remaining))))))
          
          ;; Run all assertions
          (let ((test-passed (process-assertions assertions)))
            ;; Update global counters
            (if test-passed
                (set! tests-passed-count (+ tests-passed-count 1))
                (begin
                  (set! tests-failed-count (+ tests-failed-count 1))
                  (set! tests-all-passed #f)))
            
            (display "Test '")
            (display name)
            (display "' ")
            (if test-passed
                (display "PASSED")
                (display "FAILED"))
            (display "\n")
            test-passed)))))
