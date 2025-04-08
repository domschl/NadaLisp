(define-test "string-length"
  (assert-equal (string-length "hello") 5))

(define-test "substring"
  (assert-equal (substring "hello world" 6 5) "world"))

(define-test "string-join"
  (assert-equal (string-join '("hello" "world") " ") "hello world"))

(define-test "string-split"
  (assert-equal (string-split "a,b,c" ",") '("a" "b" "c")))