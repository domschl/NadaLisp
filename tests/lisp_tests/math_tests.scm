(define-test "simple-addition" (assert-equal (+ 1 2) 3))

(define-test "multi-add" (assert-equal (+ 1 2 3 4 5) 15))

(define-test "subtraction" (assert-equal (- 10 4) 6))

(define-test "multiplication" (assert-equal (* 3 4) 12))

(define-test "division" (assert-equal (/ 10 2) 5))

(define-test "nested-arithmetic" (assert-equal (+ (* 2 3) (- 10 5)) 11))

(define-test "modulo" (assert-equal (modulo 10 3) 1))

;; Factorial function - recursive implementation
(define factorial 
  (lambda (n)
    (if (<= n 1)
        1
        (* n (factorial (- n 1))))))

;; Factorial tests - one test per assertion
(define-test "factorial-0" (assert-equal (factorial 0) 1))
(define-test "factorial-1" (assert-equal (factorial 1) 1))
(define-test "factorial-2" (assert-equal (factorial 2) 2))
(define-test "factorial-3" (assert-equal (factorial 3) 6))
(define-test "factorial-5" (assert-equal (factorial 5) 120))
(define-test "factorial-6" (assert-equal (factorial 6) 720))
(define-test "factorial-8" (assert-equal (factorial 8) 40320))
(define-test "factorial-10" (assert-equal (factorial 10) 3628800))
(define-test "factorial-12" (assert-equal (factorial 12) 479001600))
(define-test "factorial-15" (assert-equal (factorial 15) 1307674368000))

;; Prime number utilities
(define divides
  (lambda (a b)
    (= (modulo b a) 0)))

;; Helper function for prime?
(define prime-iter
  (lambda (n i)
    (cond 
      ((> (* i i) n) #t)
      ((divides i n) #f)
      (else (prime-iter n (+ i 1))))))

(define prime
  (lambda (n)
    (if (< n 2) 
        #f
        (prime-iter n 2))))

;; Helper function for primes-up-to
(define collect-primes
  (lambda (n i result)
    (cond
      ((> i n) (reverse result))
      ((prime i) (collect-primes n (+ i 1) (cons i result)))
      (else (collect-primes n (+ i 1) result)))))

(define primes-up-to
  (lambda (n)
    (collect-primes n 2 '())))

;; Helper function for reverse
(define reverse-iter
  (lambda (lst result)
    (if (null? lst)
        result
        (reverse-iter (cdr lst) (cons (car lst) result)))))

(define reverse
  (lambda (lst)
    (reverse-iter lst '())))

;; Prime number tests - one test per assertion
(define-test "prime-1" (assert-equal (prime 1) #f))
(define-test "prime-2" (assert-equal (prime 2) #t))
(define-test "prime-3" (assert-equal (prime 3) #t))
(define-test "prime-4" (assert-equal (prime 4) #f))
(define-test "prime-5" (assert-equal (prime 5) #t))
(define-test "prime-7" (assert-equal (prime 7) #t))
(define-test "prime-11" (assert-equal (prime 11) #t))
(define-test "prime-97" (assert-equal (prime 97) #t))
(define-test "prime-100" (assert-equal (prime 100) #f))
(define-test "prime-101" (assert-equal (prime 101) #t))

;; Length function tests
(define-test "length-empty" (assert-equal (length '()) 0))
(define-test "length-list" (assert-equal (length '(1 2 3)) 3))

;; Exponentiation tests - integer exponents only
(define-test "expt-basic" (assert-equal (expt 2 3) 8))
(define-test "expt-basic-2" (assert-equal (expt 3 2) 9))
(define-test "expt-zero-exp" (assert-equal (expt 5 0) 1))
(define-test "expt-zero-base" (assert-equal (expt 0 5) 0))
(define-test "expt-identity-1" (assert-equal (expt 7 1) 7))
(define-test "expt-identity-2" (assert-equal (expt 1 100) 1))
(define-test "expt-neg-base-even" (assert-equal (expt -2 2) 4))
(define-test "expt-neg-base-odd" (assert-equal (expt -2 3) -8))
(define-test "expt-neg-exp" (assert-equal (expt 2 -3) 1/8))
(define-test "expt-neg-both" (assert-equal (expt -3 -2) 1/9))
(define-test "expt-large" (assert-equal (expt 2 10) 1024))
(define-test "expt-fraction-base" (assert-equal (expt 1/2 3) 1/8))

