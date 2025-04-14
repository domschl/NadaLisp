;;; NadaLisp Simple Testing Library
;;; Provides basic testing utilities with one assertion per test

;; Initialize global test counters
(define tests-all-passed #t)
(define tests-run-count 0)
(define tests-passed-count 0)
(define tests-failed-count 0)

;; assert-equal: Compare two values for equality
(define assert-equal
  (lambda (actual expected)
    (let ((passed (equal? actual expected)))
      (if passed
          #t
          (begin
            (display "  ASSERTION FAILED\n")
            (display "  Expected: ")
            (display expected)
            (display "\n  Got:      ")
            (display actual)
            (display "\n")
            #f)))))

;; define-test: Define and run a named test case with ONE assertion
(define define-test
  (lambda (name assertion)
    ;; Increment the test counter
    (set! tests-run-count (+ tests-run-count 1))
    
    (display "Running test: ")
    (display name)
    (display "\n")
    
    ;; Update global counters based on result
    (if assertion
        (set! tests-passed-count (+ tests-passed-count 1))
        (begin
          (set! tests-failed-count (+ tests-failed-count 1))
          (set! tests-all-passed #f)))
    
    (display "Test '")
    (display name)
    (display "' ")
    (if assertion
        (display "PASSED")
        (display "FAILED"))
    (display "\n")
    
    ;; Return the test result
    assertion))
