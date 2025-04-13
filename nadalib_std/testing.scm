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
;; Simplified version that takes a single expression
(define define-test
  (lambda (name expr)
    ;; Increment the test counter
    (set! tests-run-count (+ tests-run-count 1))
    
    (display "Running test: ")
    (display name)
    (display "\n")
    
    ;; Evaluate the expression directly (no list processing)
    (let ((result expr))
      ;; Update global status if test failed
      (if result
          (set! tests-passed-count (+ tests-passed-count 1))
          (begin
            (set! tests-failed-count (+ tests-failed-count 1))
            (set! tests-all-passed #f)))
      
      (display "Test '")
      (display name)
      (display "' ")
      (if result
          (display "PASSED")
          (display "FAILED"))
      (display "\n")
      result)))
