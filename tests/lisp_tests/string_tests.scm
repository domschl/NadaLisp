(define-test "string-length"
  (assert-equal (string-length "hello") 5))

(define-test "string-length-empty"
  (assert-equal (string-length "") 0))

(define-test "substring"
  (assert-equal (substring "hello world" 6 11) "world"))

(define-test "substring-beginning"
  (assert-equal (substring "hello world" 0 5) "hello"))

(define-test "substring-edge"
  (assert-equal (substring "hello" 4 5) "o"))

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

(define-test "number-to-float-approx"
  (assert-equal (float 1/3 3) "0.333"))

;; String case conversion tests

(define-test "string-upcase-basic"
  (assert-equal (string-upcase "hello") "HELLO"))

(define-test "string-upcase-mixed"
  (assert-equal (string-upcase "Hello World") "HELLO WORLD"))

(define-test "string-upcase-already-upper"
  (assert-equal (string-upcase "CAPS") "CAPS"))

(define-test "string-upcase-with-numbers"
  (assert-equal (string-upcase "abc123") "ABC123"))

(define-test "string-upcase-with-symbols"
  (assert-equal (string-upcase "hello!@#") "HELLO!@#"))

(define-test "string-upcase-empty"
  (assert-equal (string-upcase "") ""))

(define-test "string-downcase-basic"
  (assert-equal (string-downcase "HELLO") "hello"))

(define-test "string-downcase-mixed"
  (assert-equal (string-downcase "Hello World") "hello world"))

(define-test "string-downcase-already-lower"
  (assert-equal (string-downcase "lower") "lower"))

(define-test "string-downcase-with-numbers"
  (assert-equal (string-downcase "ABC123") "abc123"))

(define-test "string-downcase-with-symbols"
  (assert-equal (string-downcase "HELLO!@#") "hello!@#"))

(define-test "string-downcase-empty"
  (assert-equal (string-downcase "") ""))

;; Unicode string case conversion tests

(define-test "string-upcase-unicode-basic"
  (assert-equal (string-upcase "café") "CAFÉ"))

(define-test "string-upcase-unicode-mixed"
  (assert-equal (string-upcase "Hållo Wörld") "HÅLLO WÖRLD"))

(define-test "string-upcase-unicode-extended"
  (assert-equal (string-upcase "řčšžýáíé") "ŘČŠŽÝÁÍÉ"))

(define-test "string-upcase-unicode-with-ascii"
  (assert-equal (string-upcase "abc-äöü-xyz") "ABC-ÄÖÜ-XYZ"))

(define-test "string-downcase-unicode-basic"
  (assert-equal (string-downcase "CAFÉ") "café"))

(define-test "string-downcase-unicode-mixed"
  (assert-equal (string-downcase "HÅLLO WÖRLD") "hållo wörld"))

(define-test "string-downcase-unicode-extended"
  (assert-equal (string-downcase "ŘČŠŽÝÁÍÉ") "řčšžýáíé"))

(define-test "string-downcase-unicode-with-ascii"
  (assert-equal (string-downcase "ABC-ÄÖÜ-XYZ") "abc-äöü-xyz"))

(define-test "string-case-conversion-preserves-non-mapped-chars-1"
  (assert-equal (string-upcase "こんにちは") "こんにちは"))

(define-test "string-case-conversion-preserves-non-mapped-chars-2"
  (assert-equal (string-downcase "こんにちは") "こんにちは"))

