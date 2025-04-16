;; Tests for square bracket parsing support in NadaLisp

;; Basic square bracket usage
(define-test "bracket-basic" (assert-equal [+ 1 2] 3))

;; Nested square brackets
(define-test "bracket-nested" (assert-equal [+ 1 [+ 2 3]] 6))

;; Mixed bracket types
(define-test "bracket-mixed-list" (assert-equal (list [+ 0 1] [+ 1 1] [+ 1 2]) '(1 2 3)))

;; Function definitions with square brackets
(define-test "bracket-function-def"
  (begin
    [define square [lambda (x) [* x x]]]
    (assert-equal (square 4) 16)))

;; List literals with square brackets
(define-test "bracket-list-literal" (assert-equal [list 1 2 3] '(1 2 3)))

;; Let expressions with square brackets
(define-test "bracket-let" (assert-equal [let [[x 3] [y 4]] [+ x y]] 7))

;; Conditionals with square brackets
(define-test "bracket-conditional" (assert-equal [if [> 3 2] "yes" "no"] "yes"))

;; Mixing brackets in the same expression
(define-test "bracket-mixing" (assert-equal [+ 1 (+ 2 3) [+ 4 5]] 15))

;; Square brackets with quote
(define-test "bracket-quote" (assert-equal '[a b c] '[a b c]))

;; Complex nesting with multiple bracket types
(define-test "bracket-complex-nesting" 
  (assert-equal [let [(a 1) [b 2]] 
                  (let [(c [+ a b]) 
                        (d [* a b])]
                    [+ c d])]
                5))

;; Multi-expression in square brackets
(define-test "bracket-begin" (assert-equal [begin (define x 5) [+ x 3]] 8))

;; Map with square brackets
(define-test "bracket-map" (assert-equal (map [lambda (x) [* x x]] '(1 2 3)) '(1 4 9)))

;; Multi-line test
(define ml-1 1) (define ml-2 2) (define ml-3 3)
(define-test "multi-line-define"
  (assert-equal (+ ml-1 ml-2 ml-3) 6))
  