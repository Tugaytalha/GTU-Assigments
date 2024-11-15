(setq KeyWords (list "and" "or" "not" "equal" "less" "nil" "list" "append" "concat" "set" "def" "for" "if" "exit" "load" "display" "true" "false"))
(setq KWs (list "KW_AND" "KW_OR" "KW_NOT" "KW_EQUAL" "KW_LESS" "KW_NIL" "KW_LIST" "KW_APPEND" "KW_CONCAT" "KW_SET" "KW_DEF" "KW_FOR" "KW_IF" "KW_EXIT" "KW_LOAD" "KW_DISPLAY" "KW_TRUE" "KW_FALSE"))
(setq Operators (list "+" "-" "/" "*" "(" ")" ","))
(setq OPs (list "OP_PLUS" "OP_MINUS" "OP_DIV" "OP_MULT" "OP_OP" "OP_CP" "OP_COMMA"))
(setq Whites (list " " "\n" "\t" "\r"))

(defun fileReader (filename)
    (with-open-file (stream filename)
        (setf contents (make-string (file-length stream)))
        (read-sequence contents stream)
        contents
    )
)

(defun isInList (word listCheck &optional (i 0))
	(if (null listCheck)
		nil
		(if (string= word (car listCheck))
			i
			(isInList word (cdr listCheck) (+ i 1))
		)
	)
)

(defun lexer (strToAnalyze &optional prev state)
  (let ((ch nil)   ; Define CH variable
            (len 0)    ; Define LEN variable
            (restStr "") ; Define RESTSTR variable
            )
          (when (null prev) (setf prev ""))
          (when (null state) (setf state "start"))

          (setf len (length strToAnalyze))
          (when (= 0 len) (return-from lexer NIL))

          (setf ch (char strToAnalyze 0))
          (setf restStr (subseq strToAnalyze 1 len))
        (cond
          ((string= state "start")
              (cond
                  ((char= ch #\;) lexer(restStr (string ch) "cmmt1"))
                  ((not (equal (isInList (string ch) Whites) nil)) (lexer restStr))
                  ((not (equal (isInList (string ch) Operators) nil)) (progn (format t  "~a ~%" (nth (isInList (string ch) Operators) OPs)) (lexer restStr (string ch) "start")))
                  ((digit-char-p ch) (lexer restStr (string ch) "sdigit"))
                  ((alpha-char-p ch) (lexer restStr (string ch) "letter"))
                  (t (lexer restStr (string ch) "error")) ; Error
              )
          ((string= state "cmmt1")
              (cond
                  ((char= ch #\;) (lexer restStr (string ch) "inComment")) ; Inside comment
                  (t (lexer restStr ";" "error")) ; Error
              )
          )
          ((string= state "inComment")
              (cond
                  ((char= ch #\newline) (lexer restStr)) ; End of comment
                  (t (lexer restStr (string ch) "inComment")) ; Inside comment
              )
          )
          ((string= state "sdigit")
              (cond
                  ((digit-char-p ch) (lexer restStr (concatenate 'string prev (string ch)) "sdigit"))
                  ((char= ch #\b) (lexer restStr (concatenate 'string prev (string ch)) "bdigit"))
                  (t (lexer restStr (concatenate 'string prev (string ch)) "error")) ; Error
              )
          )
          ((string= state "letter")
              (cond
                  ((or (alpha-char-p ch) (digit-char-p ch)) (lexer restStr (concatenate 'string prev (string ch)) "letter")) 
                  ((not (equal (isInList (string ch) Whites) nil)) (if (equal (isInList prev KeyWords) nil) 
                    (progn (format t  "IDENTIFIER~%") (lexer restStr (string ch) "start")) 
                    (progn (format t  "~a ~%" (nth (isInList prev KeyWords) KWs)) (lexer restStr (string ch) "start")))
                  )
              )
          )
          ((string= state "bdigit")
              (cond
                  ((digit-char-p ch) (lexer restStr (concatenate 'string prev (string ch)) "valuef"))
                  (t (lexer restStr (concatenate 'string prev (string ch)) "error")) ; Error
              )
          )
          ((string= state "valuef")
              (cond
                  ((digit-char-p ch) (lexer restStr (concatenate 'string prev (string ch)) "valuef"))
                  ((not (equal (isInList (string ch) Whites) nil)) (progn (format t  "VALUEF~%") (lexer restStr (concatenate 'string prev (string ch)) "start")))
                  (t (lexer restStr (concatenate 'string prev (string ch)) "error")) ; Error
              )
          )
          ((string= state "error")
              (format t "SYNTAX_ERROR, ~a  cannot be tokenized.~%" prev)
              (return-from lexer NIL)
          )
      )
    )
  )
)

(defun gpplexer (&optional filename)
  (if filename
      (progn
        (setf content (fileReader filename))
        (lexer content))
      (let ((line (read-line)))
        (if line
            (progn
              (lexer line)
              (gpplexer))
            (format t "End of input.~%"))))
)


(defun main ()
    (gpplexer)
)

(main)