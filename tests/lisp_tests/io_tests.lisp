(define-test "display-basic" 
  (begin
    (display "Hello Test\n")
    (assert-equal #t #t)))

(define-test "display-value" 
  (begin
    (display 42)
    (assert-equal #t #t)))

(define-test "write-read-file"
  (begin
    (define test-content "Test file content")
    (write-file "test_output.txt" test-content)
    (assert-equal (read-file "test_output.txt") test-content)))

(define-test "read-write-to-string"
  (begin
    (define expr (list 1 (list 2 3) 4))
    (define str (write-to-string expr))
    (assert-equal (read-from-string str) expr)))

(define-test "eval-simple"
  (assert-equal (eval (read-from-string "(+ 1 2)")) 3))

(define-test "eval-complex"
  (assert-equal 
    (eval (read-from-string "(+ (* 2 3) (- 10 5))")) 
    11))