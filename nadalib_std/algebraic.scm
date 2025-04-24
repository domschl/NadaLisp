;; Purely functional algebraic notation utilities

;; Example implementation for special constants
(define pi-value 'pi)  ;; Symbolic representation
(define e-value 'e)
(define i-value 'i)

(define x 'x)
(define y 'y)
(define z 'z)

;; All numbers in the core are rational, so this should just use number?
(define rational? 
  (lambda (expr)
    (number? expr)))

;; A fraction is a rational that's not an integer
(define fraction? 
  (lambda (expr)
    (and (rational? expr) 
         (not (integer? expr)))))

;; Irrational numbers can only be represented symbolically
(define irrational? 
  (lambda (expr)
    (or (eq? expr 'pi) 
        (eq? expr 'e))))

;; Complex numbers are represented by the symbol i or compound expressions
(define complex? 
  (lambda (expr)
    (eq? expr 'i)))

;; Updated constant? predicate to include all special numeric constants
(define constant?
  (lambda (expr)
    (or (number? expr)       ;; Rational numbers
        (irrational? expr)   ;; Symbolic irrational constants
        (complex? expr))))   ;; Symbolic complex constants

;; Add a variable? predicate to check if something is a variable
(define variable?
  (lambda (expr)
    (and (symbol? expr) 
         (not (irrational? expr))
         (not (complex? expr)))))

;; Helper function to identify symbolic expressions
(define symbolic?
  (lambda (expr)
    (or (variable? expr)                  ; Single variable
        (and (list? expr)                 ; Function application
             (not (null? expr))
             (not (number? expr))))))

;; Integer division truncated toward zero, using `remainder`
(define quotient
  (lambda (n d)
    (if (= d 0)
        (error "Division by zero in quotient")
        (let ((r (remainder n d)))
          ;; n = d*q + r  ⇒  q = (n – r)/d
          (/ (- n r) d)))))

(define integer-sqrt-loop (lambda (low high n)
  (let ((mid (quotient (+ low high) 2)))
    (if (= mid low)
        low
        (if (> (* mid mid) n)
            (integer-sqrt-loop low mid n)
            (integer-sqrt-loop mid high n))))))

;; Integer square root - returns largest integer not exceeding sqrt(n)
(define integer-sqrt
  (lambda (n)
    (if (< n 0)
        (error "Cannot compute square root of negative number")
        (integer-sqrt-loop 0 (+ n 1) n))))

(define zero?
  (lambda (x)
    (cond ((number? x) (= x 0))
          (else #f))))

;; Extract the largest square divisor of n by walking down from integer‑sqrt(n)
;; Extract the largest square divisor of n using mutation (`set!`)
(define largest-square-divisor
  (lambda (n)
    (let ((i (integer-sqrt n)))
      (let loop ()
        (cond
          ((<= i 1) 1)
          ((zero? (remainder n (* i i))) i)
          (else
            (begin
              (set! i (- i 1))   ; decrement i by 1
              (loop))))))))

(define notnumber?
  (lambda (x)
    (not (number? x))))

;; In a sorted argument list 'args' like (8 a b x x) transform it to (8 a b (* 2 x))
(define factor-equal-symbols
  (lambda (args)
    (if (null? args)
        '()
        (factor-equal-symbols-helper (cdr args) (car args) 1 '()))))

(define factor-equal-symbols-helper
  (lambda (rest current count result)
    (if (null? rest)
        ;; Handle the last element and return the final result
        (reverse
          (if (= count 1)
              (cons current result)
              (cons (list '* count current) result)))
        ;; Process the list
        (if (equal? (car rest) current)
            ;; Same element, increment count
            (factor-equal-symbols-helper (cdr rest) current (+ count 1) result)
            ;; Different element, add the current element to result
            (if (= count 1)
                (factor-equal-symbols-helper (cdr rest) (car rest) 1 (cons current result))
                (factor-equal-symbols-helper (cdr rest) (car rest) 1 (cons (list '* count current) result)))))))

(define factor-exp-equal-symbols
  (lambda (args)
    (if (null? args)
        '()
        (factor-exp-equal-symbols-helper (cdr args) (car args) 1 '()))))

(define non-zero-number?
  (lambda (x)
    (and (number? x) (not (= x 0)))))

(define factor-exp-equal-symbols-helper
  (lambda (rest current count result)
    (if (null? rest)
        ;; Handle the last element and return the final result
        (reverse
          (if (= count 1)
              (cons current result)
              (cons (list '^ current count) result)))
        ;; Process the list
        (if (equal? (car rest) current)
            ;; Same element, increment count
            (factor-exp-equal-symbols-helper (cdr rest) current (+ count 1) result)
            ;; Different element, add the current element to result
            (if (= count 1)
                (factor-exp-equal-symbols-helper (cdr rest) (car rest) 1 (cons current result))
                (factor-exp-equal-symbols-helper (cdr rest) (car rest) 1 (cons (list '^ current count) result)))))))

(define associative-expand (lambda (args op)
  (define exp-args '())
  (for-each (lambda (arg)
    (if (list? arg)
       (if (eq? op (car arg))
          (set! exp-args (append exp-args (cdr arg)))
          (set! exp-args (append exp-args (list arg))))
       (set! exp-args (append exp-args (list arg)))))
    args)
    exp-args))

(define add-op (lambda (args)
  (define exp-args (associative-expand args '+))
  (display "Exp-args: ") (display exp-args) (newline)
  (define num-sum-list (filter non-zero-number? exp-args))
  (define num-sum '())
  (if (not (null? num-sum-list))
      (set! num-sum (apply + num-sum-list)))
  ;; (define sym-sum (filter (lambda (x) (not (number? x))) exp-args))  ;; LEAKs
  (define sym-sum (factor-equal-symbols (filter notnumber? exp-args)))
  (display "Add-op: ") (display num-sum) (display " ")
  (display sym-sum) (newline)
  (if (null? num-sum)
      (if (null? sym-sum)
          0
          (if (= (length sym-sum) 1)
              (car sym-sum)
              (list '+ sym-sum)))
      (if (null? sym-sum)
          num-sum
          (cons '+ (cons num-sum sym-sum))))
  ))



(define sub-op (lambda (args)
  (cond 
    ((null? args) 0)  ; No arguments case
    ((= (length args) 1)  ; Unary negation case
     (if (number? (car args))
         (- (car args))
         (list '* -1 (car args))))
    (else  ; Subtraction case: a - b - c becomes a + (-b) + (-c)
     (let ((transformed-args 
             (cons (car args)  ; Keep the first argument as is
                   (map (lambda (x)  ; Negate all the rest
                          (if (number? x)
                              (- x)  ; Negate numbers directly
                              (list '* -1 x)))  ; Symbolic negation for expressions
                        (cdr args)))))
       (add-op transformed-args))))))


(define mul-number-filter?
  (lambda (x)
    (and (number? x) (not (= x 1)))))

(define mul-op (lambda (args)
  (define exp-args (associative-expand args '*))
  (display "Exp-args: ") (display exp-args) (newline)
  (define num-mul (apply * (filter mul-number-filter? exp-args)))
  (define sym-mul (factor-exp-equal-symbols (filter notnumber? exp-args)))
  (display "Mul-op: ") (display num-mul) (display "| ")
  (display sym-mul) (newline)
  (if (null? num-mul)
      (if (null? sym-mul)
          1
          (if (= (length sym-mul) 1)
              (car sym-mul)
              (list '* sym-mul)))
      (if (null? sym-mul)
          num-mul
          (if (= num-mul 0)
              0
              (if (= num-mul 1)
                (if (= (length sym-mul) 1)
                  (car sym-mul)
                  (cons '* sym-mul))
                (if (= (length sym-mul) 1)
                    (cons '* (cons num-mul sym-mul))
                    (cons '* (cons num-mul sym-mul)))))))
  ))

(define div-op (lambda (args)
  (define exp-args (associative-expand args '/))
  (display "Exp-args: ") (display exp-args) (newline)
  (define num-div '())
  (define num-div-list (filter number? exp-args))
  (cond 
    ((null? num-div-list) (set! num-div '()))
    ((= (length num-div-list) 1) (set! num-div (car num-div-list)))
    (else (set! num-div (apply / num-div-list))))
  (define sym-div (factor-exp-equal-symbols (filter notnumber? exp-args)))
  (display "Div-op: ") (display num-div) (display "| ")
  (display sym-div) (newline)
  (if (null? num-div)
      (if (null? sym-div)
          1
          (if (= (length sym-div) 1)
              (car sym-div)
              (list '/ sym-div)))
      (if (= num-div 0)
          0
          (if (= num-div 1)
            (if (= (length sym-div) 1)
              (car sym-div)
              (cons '/ sym-div))
            (cond 
              ((null? sym-div) num-div)
              ((= (length sym-div) 1)
                (if (= num-div 1)
                    (car sym-div)
                    (cons '/ (cons num-div sym-div))))
              (else 
                (cons '/ (cons num-div sym-div)))))))))

(define expt-op (lambda (args) (apply expt args)))

(define eval-symbolic (lambda (expr)
  (cond ((number? expr) expr)
        ((irrational? expr) expr)
        ((complex? expr) expr)
        ((variable? expr) expr)
        ((list? expr)
        (begin
          (define op (car expr))
          (define args (map
                  (lambda (arg)
                    (if (list? arg)
                        (eval-symbolic arg)
                        arg)) (cdr expr)))
          (display "Op: ") (display op) (newline) (display "Args: ") (display args) (newline)
                (cond 
                  ((eq? op '+) (add-op args))
                  ((eq? op '-) (sub-op args))
                  ((eq? op '*) (mul-op args))
                  ((eq? op '/) (div-op args))
                  ((eq? op '^) (expt-op args))
                  (else expr)))))))

(define calc (lambda (expr) (eval-symbolic (infix->prefix expr))))
