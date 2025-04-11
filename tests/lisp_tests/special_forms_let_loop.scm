; -------- special_forms_let_loop.scm --------
(define-test "let-named-form"
  (assert-equal (let loop ((i 0) (acc 0))
                  (if (= i 5)
                      acc
                      (loop (+ i 1) (+ acc i))))
                10))
