(define-test "string-length"
  (assert-equal (string-length "hello") 5))

(define-test "string-length-empty"
  (assert-equal (string-length "") 0))

(define-test "substring"
  (assert-equal (substring "hello world" 6 5) "world"))

(define-test "substring-beginning"
  (assert-equal (substring "hello world" 0 5) "hello"))

(define-test "substring-edge"
  (assert-equal (substring "hello" 4 1) "o"))

(define-test "string-join"
  (assert-equal (string-join (list "hello" "world") " ") "hello world"))

(define-test "string-join-empty"
  (assert-equal (string-join (list) ",") ""))

(define-test "string-join-single"
  (assert-equal (string-join (list "solo") "-") "solo"))

(define-test "string-split"
  (assert-equal (string-split "a,b,c" ",") (list "a" "b" "c")))

(define-test "string-split-empty"
  (assert-equal (string-split "" ",") (list "")))

(define-test "string-split-no-delimiter"
  (assert-equal (string-split "abc" ",") (list "abc")))

(define-test "string-to-number-integer"
  (assert-equal (string->number "42") 42))

(define-test "string-to-number-fraction"
  (assert-equal (string->number "3/4") 3/4))

(define-test "number-to-string-integer"
  (assert-equal (number->string 42) "42"))

(define-test "number-to-string-fraction"
  (assert-equal (number->string 3/4) "3/4"))