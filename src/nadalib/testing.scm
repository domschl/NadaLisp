;;; NadaLisp Testing Library
;;; Provides testing utilities for writing and running tests

;; assert-equal: Compare two values for equality
;; Returns #t on success, or a function that displays error details on failure
(define assert-equal
  (lambda (actual expected)
    (if (equal? actual expected)
        #t
        (lambda ()
          (display "  ASSERTION FAILED\n")
          (display "  Expected: ")
          (display expected)
          (display "\n  Got:      ")
          (display actual)
          (display "\n")
          #f))))

;; define-test: Define and run a named test case
;; Returns the test result (#t for pass, something else for fail)
(define define-test
  (lambda (name body)
    (display "Test: ")
    (display name)
    (let ((result body))
      (display "... ")
      (if (equal? result #t)
          (display "PASSED\n")
          (display "FAILED\n"))
      result)))

;; run-test-file: Run all tests in a file
;; This is a placeholder that could be expanded later
(define run-test-file
  (lambda (filename)
    (display "Running tests from ")
    (display filename)
    (display "\n")
    (load filename)))

;; Utility to run all test files in a directory
;; (This would require directory listing capabilities)
(define run-tests
  (lambda (dir-path)
    (display "Running tests from directory: ")
    (display dir-path)
    (display "\n")
    ;; This would need to be implemented with native code
    ;; or directory listing capabilities
    (display "Not yet implemented\n")))