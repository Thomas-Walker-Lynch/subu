;; rt-format.el
(require 'cl-lib)

;;--------------------------------------------------------------------------------
;; basic types
;;

;; strn:: (string length)
;; interval:: (base length)
;; source-interval:: (source interval)
;; token:: (parser strn)
;; source-token:: (parser source-interval)

;; source being operated on is stored as an strn

  ;; rt-format.el

  ;; source being operated on is stored as an strn

  ;; strn:: (string length)
  ;; interval:: (base length)
  ;; source-interval:: (source interval)
  ;; token:: (parser strn)
  ;; source-token:: (parser source-interval)

  ;; rt-format.el

  ;; Types stored as lists instead of cons pairs
  ;; strn:: (string length)
  ;; interval:: (base length)
  ;; source-interval:: (source interval)
  ;; token:: (parser strn)
  ;; source-token:: (parser source-interval)

  (defun rt-unsigned-inntegerp (n)
    "Return t if N is a non-negative integer."
    (and
      (integerp n)
      (>= n 0)
      ))

(defun rt-args-process (function args arg-specs)
  "Process ARGS according to ARG-SPECS.
Returns the processed ARGS list.

ARG-SPECS is a list where each element is an argument specification:
 - For required arguments: (TYPEP MESSAGE)
 - For optional arguments: (DEFAULT TYPEP MESSAGE)

The function validates the arguments, applies defaults for missing optional arguments,
and returns the list of arguments."
  (catch 'exit

    (unless args (throw 'exit nil))
    (unless arg-specs (throw 'exit args))
    
    (let
      (
        (err-flag nil)
        (messages '())
        )
      (unless (listp arg-specs) (push messages "rt-args-process usage error, arg-specs must be a list"))
      (unless (listp arg) (push messages "rt-args-process usage error, passed in arg must be a list"))
      (when messages (error "%s::\n  %s" (symbol-name function) (mapconcat #'identity messages "\n  ")))
      )

    ;; process required args
    (let
      (
        (messages '(cons nil nil))
        (type-messages-last messages)
        (args-pt args)
        )

      (let
        (
          (spec-list-pt (car args-spec)) ; accessing the required args specs
          )
        (unless 
          (listp specs-list-pt) 
          (error "%s:: arg-specs missing list of required argument specifications" (symbol-name function))
          )

        (while specs-list-pt
          (if 
            (not args-pt)
            (progn
              (setq (cdr type-messages-last) (cons "missing required argument" nil))
              (setq type-messages-last (cdr type-messages-last))
              )
            (let
              (
                (spec (car specs-list-pt))
                )
              (if
                (not (and (listp spec) (= (length spec) 2)))
                (error "%s:: arg-spec malformeded %s" (symbol-name function) (prin1-to-string spec))
                (let
                  (
                    (typep (car spec))
                    (message (cadr spec))
                    )
                  (unless 
                    (funcall typep arg) 
                    (progn
                      (setq (cdr type-messages-last) (cons message nil))
                      (setq type-messages-last (cdr type-messages-last))
                      )
                    )))))
          (when args-pt (setq args-pt (cdr args-pt)))
          (setq specs-list-pt (cdr specs-list-pt))
          ))
      
      ;; process optional args
      (when
        (>= (length args-specs) 2)
        (let
          (
            (spec-list-pt (cadr args-spec)) ; accessing the optional args specs
            )
          (while
            spec-list-pt
            (when 
              args-pt
              (let
                (
                  (spec (car spec-list-pt))
                  )
                (if
                  (not (and (listp spec) (= (length spec) 3)))
                  (error "%s:: arg-spec malformeded %s" (symbol-name function) (prin1-to-string spec))
                  (let
                    (
                      (defult (car spec))
                      (typep (cadr spec))
                      (message (caddr spec))
                      )
                    (unless 
                      (funcall typep arg) 
                      (progn
                        (setq (cdr type-messages-last) (cons message nil))
                        (setq type-messages-last (cdr type-messages-last))
                        )
                      )))))
            (when args-pt (setq args-pt (cdr args-pt)))
            (setq specs-list-pt (cdr specs-list-pt))
            )))

      ;; print type error messages
      (when messages (error "%s::\n  %s" (symbol-name function) (mapconcat #'identity messages "\n  ")))
      ))


  (defun rt-pad-or-truncate (list target-length &optional padding-value)
    "Pad or truncate LIST to make it TARGET-LENGTH."
    (let
      (
        (padding-value (or padding-value nil))
        )
      (cond
        ((< (length list) target-length)
         (while (< (length list) target-length)
           (setq list (append list (list padding-value))))
         list)

        ((> (length list) target-length)
         (cl-subseq list 0 target-length))

        (t
         list
         )
        )))

  (when nil
    (let*
      (
        (list (list 1 2 3))
        (list-longer  (rt-pad-or-truncate list 5))
        (list-shorter (rt-pad-or-truncate list 1))
        )
      (message "%s" (prin1-to-string list-longer))
      (message "%s" (prin1-to-string list-shorter))
      ;;(1 2 3 nil nil)
      ;;(1)
      )
  )


  (defun rt-strn-make (&rest args)

    ;; argument guards
    (unless (>= (length args) 1) (error "rt-strn-make requires at least a string argument."))
    (let
      (
        (arg-types `(
                      (#'stringp "Argument 0 must be a string.")
                      (#'rt-unsigned-integerp "Argument 1 must be an unsigned integer.")
                      )))
      (rt-args-types #'rt-strn-make args arg-types)
      )

    (let
      (
        (arg-defaults `(
                         (length (car args))
                         )))
      (setq args (rt-defaults args 1 arg-defaults))
      )

    ;; processing
    (let
      (
        (string (nth 0 args))
        (length (nth 1 args))
        )
      (list string length)
      )
    )


  (when nil
    (progn
      (rt-strn-make 3 "abc")
      ;; if: Wrong type argument: rt-strn-make, ((stringp string) (rt-unsigned-inntegerp length))
      (rt-strn-make "abc" 4 5 6)
      (rt-strn-make "abc" 4)
    )
  )

  (defun rt-strnp (strn)
    (and
      (listp strn)
      (= (length strn 2))
      (stringp (car strn))
      (rt-unsigned-inntegerp (cadr strn))
      ))


  (defun rt-interval-make (base length)
    (unless
      (and
        (rt-unsigned-inntegerp base)
        (rt-unsigned-inntegerp length)
        )
      (signal 'wrong-type-argument '(rt-interval-make ((rt-unsigned-inntegerp base) (rt-unsigned-inntegerp length))))
      )
    (list base length))

  (defun rt-source-interval-make (source base length)
    (unless
      (and
        (stringp source)
        (rt-unsigned-inntegerp base)
        (rt-unsigned-inntegerp length)
        )
      (signal 'wrong-type-argument '(rt-source-interval-make ((stringp source) (rt-unsigned-inntegerp base) (rt-unsigned-inntegerp length))))
      )
    (list source (rt-interval-make base length)))

  (defun rt-token-make (parser string length)
    (unless
      (and
        (symbolp parser)
        (stringp string)
        (rt-unsigned-inntegerp length)
        )
      (signal 'wrong-type-argument '(rt-token-make ((symbolp parser) (stringp string) (rt-unsigned-inntegerp length))))
      )
    (list parser (rt-strn-make string length)))

  (defun rt-source-token-make (parser source base length)
    (unless
      (and
        (symbolp parser)
        (stringp source)
        (rt-unsigned-inntegerp base)
        (rt-unsigned-inntegerp length)
        )
      (signal 'wrong-type-argument '(rt-source-token-make ((symbolp parser) (stringp source) (rt-unsigned-inntegerp base) (rt-unsigned-inntegerp length))))
      )
    (list parser (rt-source-interval-make source base length)))

  (defmacro rt-strn-deconstruct (args &rest body)
    (unless
      (and 
        (listp args) 
        (= (length args) 3)
        )
      (signal 'wrong-number-of-arguments
              `(rt-strn-deconstruct
                "Expected 3 arguments (strn, string, length), but got: ,args")))
    (let
      (
        (strn (nth 0 args))
        (string (nth 1 args))
        (length (nth 2 args))
        )
      `(let
         (
           (,string (nth 0 ,strn))
           (,length (nth 1 ,strn))
           )
         ,@body
         )))

  (defmacro rt-interval-deconstruct (args &rest body)
    (unless
      (and (listp args) (= (length args) 3))
      (signal 'wrong-number-of-arguments
              `(rt-interval-deconstruct
                "Expected 3 arguments (interval, base, length), but got: ,args")))
    (let
      (
        (interval (nth 0 args))
        (base (nth 1 args))
        (length (nth 2 args))
        )
      `(let
         (
           (,base (nth 0 ,interval))
           (,length (nth 1 ,interval))
           )
         ,@body
         )))

  (defmacro rt-source-interval-deconstruct (args &rest body)
    (unless
      (and (listp args) (= (length args) 4))
      (signal 'wrong-number-of-arguments
              `(rt-source-interval-deconstruct
                "Expected 4 arguments (source-interval, source, base, length), but got: ,args")))
    (let
      (
        (source-interval (nth 0 args))
        (source (nth 1 args))
        (base (nth 2 args))
        (length (nth 3 args))
        )
      `(let
         (
           (,source (nth 0 ,source-interval))
           )
         (rt-interval-deconstruct ((nth 1 ,source-interval) ,base ,length)
           ,@body
           )
         )))

  (defmacro rt-token-deconstruct (args &rest body)
    (unless
      (and (listp args) (= (length args) 4))
      (signal 'wrong-number-of-arguments
              `(rt-token-deconstruct
                "Expected 4 arguments (token, parser, string, length), but got: ,args")))
    (let
      (
        (token (nth 0 args))
        (parser (nth 1 args))
        (string (nth 2 args))
        (length (nth 3 args))
        )
      `(let
         (
           (,parser (nth 0 ,token))
           )
         (rt-strn-deconstruct ((nth 1 ,token) ,string ,length)
           ,@body
           )
         )))

  (defmacro rt-source-token-deconstruct (args &rest body)
    (unless
      (and (listp args) (= (length args) 5))
      (signal 'wrong-number-of-arguments
              `(rt-source-token-deconstruct
                "Expected 5 arguments (source-token, parser, source, base, length), but got: ,args")))
    (let
      (
        (source-token (nth 0 args))
        (parser (nth 1 args))
        (source (nth 2 args))
        (base (nth 3 args))
        (length (nth 4 args))
        )
      `(let
         (
           (,parser (nth 0 ,source-token))
           )
         (rt-source-interval-deconstruct ((nth 1 ,source-token) ,source ,base ,length)
           ,@body
           )
         )))

  (defun rt-strn-to-string (strn)
    (rt-strn-deconstruct (strn string length)
      (format
        "(rt-strn-make \"%s\" %d)"
        string length
        )
      ))

  (defun rt-interval-to-string (interval)
    (rt-interval-deconstruct (interval base length)
      (format
        "(rt-interval-make %d %d)"
        base length
        )
      ))

  (defun rt-source-interval-to-string (source-interval)
    (rt-source-interval-deconstruct (source-interval source base length)
      (format
        "(rt-source-interval-make \"%s\" %d %d)"
        source base length
        )
      ))

  (defun rt-token-to-string (token)
    (rt-token-deconstruct (token parser string length)
      (format
        "(rt-token-make '%s \"%s\" %d)"
        (symbol-name parser) string length
        )
      ))

  (defun rt-source-token-to-string (source-token)
    (rt-source-token-deconstruct (source-token parser source base length)
      (format
        "(rt-source-token-make '%s \"%s\" %d %d)"
        (symbol-name parser) source base length
        )
      ))

  ;; examples
  (when nil

    (let 
      (
        (strn (rt-strn-make "1234" 4))
        )
      (message "strn:: %s" (rt-strn-to-string strn))
      )

    (let 
      (
        (interval (rt-interval-make 1 8))
        )
      (message "interval:: %s" (rt-interval-to-string interval))
      )

    (let 
      (
        (token (rt-token-make 'token "567" 3))
        )
      (message "token:: %s" (rt-token-to-string token))
      )

    (let* 
      (
        (source "now is the time for all good AI to come to the aid of their country")
        (source-interval (rt-source-interval-make source 11 15))
        )
      (message "source-interval:: %s" (rt-source-interval-to-string source-interval))
      )

    (let*
      (
        (source "now is the time for all good AI to come to the aid of their country")
        (source-token (rt-source-token-make 'source-token source 24 28))
        )
      (message "source-token:: %s" (rt-source-token-to-string source-token))
      )

    ;; strn:: (rt-strn-make "1234" 4)
    ;; interval:: (rt-interval-make 1 8)
    ;; token:: (rt-token-make 'token "567" 3)
    ;; source-interval:: (rt-source-interval-make "now is the time for all good AI to come to the aid of their country" 11 15)
    ;; source-token:: (rt-source-token-make 'source-token "now is the time for all good AI to come to the aid of their country" 24 28)

    )  

;;--------------------l------------------------------------------------------------
;; misc
;;

  (defmacro do-strn (args &rest body)
    "Iterate over the string in a strn structure.
  ARGS should be (strn char index). BODY will be executed for each character in the string.
  The index will start at 'start-index' if provided, otherwise from 0."
    (unless
      (and
        (listp args)
        (= (length args) 3)
        (rt-strnp (car args))
        (symbolp (cadr args))
        (symbolp (caddr args))
        )
      (signal 'wrong-number-of-arguments '(do-strn (rt-strnp symbolp symbolp)))
      )
    (let
      (
        (strn (car args))
        (char (cadr args))
        (index (caddr args))
        )
      `(rt-strn-deconstruct
        (,strn string length)
        (let
          (
            (,char nil)
            (,index (or ,index 0))
            )
          (while (< ,index length)
            (setq ,char (aref string ,index))
            ,@body
            (setq ,index (1+ ,index))
            )))))


  ;; for example
  (when nil

    (let
      (
        (s (rt-strn-make "Hello" 5))
        )
      (do-strn (s c i)
        (message "Character at index %d: %c" i c)
        ))

    )
  ;; Character at index 0: H
  ;; Character at index 1: e
  ;; Character at index 2: l
  ;; Character at index 3: l
  ;; Character at index 4: o



;;--------------------l------------------------------------------------------------
;; parsing helpers
;;

  (defun rt-sort-token-list (token-list)
    "Sort tokens by their length field, largest first."
    (sort token-list (lambda (a b) (> (cdr a) (cdr b))))
    )

  ;; for example
  (when nil
    (let
      (
        (example-token-list
          '(
            ((rt-token-make 'type "apple") . 5)
            ((rt-token-make 'type "banana") . 6)
            ((rt-token-make 'type "cherry") . 6)
            ((rt-token-make 'type "fig") . 3)
            )
          )
        (sorted-token-list (rt-sort-token-list example-token-list))
        )
      (message "%s" (prin1-to-string sorted-token-list))
      ;; (((rt-token-make 'type "banana") . 6) ((rt-token-make 'type "cherry") . 6) ((rt-token-make 'type "apple") . 5) ((rt-token-make 'type "fig") . 3))
      ))

  (defun rt-assoc-to-token-list (assoc-list access)
    "Given an (<X>.token) or (token.<X>) assoc-list, and an access function for the pair, return a sorted token list."
    (let
      (
        (result-list (cons nil nil))
        (last-cons nil)
        )
      (setq last-cons result-list)
      (while assoc-list
        (let
          (
            (token (funcall access (car assoc-list)))
            )
          (setcdr last-cons (cons (cons token (length (cdr token))) nil))
          (setq last-cons (cdr last-cons))
          (setq assoc-list (cdr assoc-list))
          ))
      (rt-sort-token-list (cdr result-list))
      ))

  (defun rt-key-list (assoc-list)
    "Given a (string.<X>) assoc-list, return a sorted strn list of the keys."
    (rt-assoc-to-token-list assoc-list #'car)
    )

  (defun rt-value-list (assoc-list)
    "Given a (<X>.string) assoc-list, return a sorted strn list of the values."
    (rt-assoc-to-token-list assoc-list #'cdr)
    )

  (defun rt-is-one-of (source base-index token-list)
    "Return source-token found at base-index, or nil if no match."
    (let
      (
        (string (car source))
        (length (cdr source))
        )
      (catch 'found
        (dolist
          (token token-list)
          (rt-token-deconstruct token parser strn-string strn-length
            (let
              (
                ;; each parser returns source-token of a specific type, otherwise nil
                (source-token (funcall parser source base-index))
                )
              (when source-token
                (throw 'found source-token))))))))

  (defun rt-search-forward (source start-index is-a-fn)
    "return match to #'is-a-fn, otherwise ni."
    (let 
      (
        (index start-index)
        (length (cdr source))
        (token-length nil)
        )
      (catch 'found
        (while 
          (< index length)
          (setq token-length (funcall is-x-fn source index))
          (when token-length (throw 'found (cons index token-length)))
          (setq index (1+ index))
          )
        nil
        )))



;;--------------------l------------------------------------------------------------
;; enclosure tokens
;;

  ;; enclosure delimiters are tokens possibly of multiple characters
  (defvar rt-default-enclosure-pairs
    '(
       ((rt-token-make #'rt-is-a-opening "{") . (rt-token-make #'rt-is-a-closing "}"))
       ((rt-token-make #'rt-is-a-opening "(") . (rt-token-make #'rt-is-a-closing ")"))
       ((rt-token-make #'rt-is-a-opening "[") . (rt-token-make #'rt-is-a-closing "]"))
       )
    "Association list of default enclosure pairs represented as tokens."
    )

  ;; for example
  (when nil
    (message "%s" (prin1-to-string (rt-key-list rt-default-enclosure-pairs)))
    (message "%s" (prin1-to-string (rt-value-list rt-default-enclosure-pairs)))
    )
    ;; (((rt-token-make 'open "{") . 2) ((rt-token-make 'open "(") . 2) ((rt-token-make 'open "[") . 2))
    ;; (((rt-token-make 'close "}") . 2) ((rt-token-make 'close ")") . 2) ((rt-token-make 'close "]") . 2))

  (defvar rt-opening-token-list
    (rt-key-list rt-default-enclosure-pairs)
    "Default list of opening tokens represented as token pairs."
    )

  (defvar rt-opening-token-list
    (rt-value-list rt-default-enclosure-pairs)
    "Default list of opening tokens represented as token pairs."
    )

  (defun rt-is-a-opening (source base-index)
    "Return interval for matching opening delimiter, otherwise nil."
    (rt-is-one-of source base-index rt-opening-list))

  (defun rt-is-a-closing (source base-index)
    "Return interval for matching closing delimiter, otherwise nil."
    (rt-is-one-of source base-index rt-closing-list))

  (defun rt-is-a-enclosure (source base-index)
    (or
      (rt-is-a-opening source base-index)
      (rt-is-a-closing source base-index)
      ))

;;--------------------l------------------------------------------------------------
;; other tokens
;;

  (defun rt-is-a-blank-char (char)
    (char-equal char ?\s)
    )


  (defun rt-is-a-blank (source start-index)
    "Returns source-token of blank characters, otherwise nil."
    (let
      (
        (string (car source))
        (length (cdr source))
        (start-index-var start-index)
        )
      (dostring (string char index)
        
        (when
          (and (>= index start-index-var) (rt-is-a-blank-char char))
          (setq start-index-var (1+ start-index-var))
          ))
      (if (> start-index-var start-index)
        (rt-token-make #'rt-is-a-blank string (- start-index-var start-index))
        nil
        )))


  (defun rt-is-a-blank (source start-index)
    "Returns source-token of blank characters, otherwise nil."
    (let
      (
        (index start-index)
        (string (car source))
        (length (cdr source))
        )
      (while
        (and
          (< index length)
          (rt-is-a-blank-char (aref string index))
          )
        (setq index (1+ index))
        )
      (if (> index start-index)
        (rt-token-make 'blank string (- index start-index))
        nil
        )))

  (defun rt-is-a-prefix (source)
    "Given a string, return the non-zero length up to the first opening enclosure, otherwise nil."
    (let
      (
        (interval (rt-search-forward source 0 #'rt-is-a-opening))
        )
      (when
        interval
        (let
          (
            (prefix-end (car interval))
            )
          (when
            (> prefix-end 0)
            end
            )))))

  (defun rt-is-a-stuff (string base-index)
    "Given a base index into a string, return nonzero length including non-blank and non-enclosure, otherwise nil."
    (let (
           (enclosure-interval (rt-search-forward source 0 #'rt-is-a-enclosure))
           (blank-itnerval (rt-search-forward source 0 #'rt-is-a-blank))
           (enclosure-base 0)
           (blank-base 0)
           (stuff-end 0)
           )
      (when enclosure-interval (setq enclosure-base (car enclosure-interval)))
      (when blank-interval (setq blank-base (car blank-interval)))
      
      (when 
        (and result (> (car result) 0))
        (cdr result))
      ))


  (defvar rt-token-types
    '(
      rt-is-a-prefix
      rt-is-a-blank
      rt-is-a-opening
      rt-is-a-closing
      rt-is-a-stuff
      )
    "List of functions to recognize different token types, excluding prefix."
    )

  (defun rt-parse-line (line token-types)
    "Given a LINE and a TOKEN-TYPES list, return a list of tokens."
    (let*
      (
        (tokens (cons nil nil))
        (last-cons tokens)
        (index 0)
        (token-length nil)
        (prefix-parser (car token-types))
        (other-parsers (cdr token-types))
        )
      ;; Match prefix once
      (setq token-length (funcall prefix-parser line index))
      (when token-length
        (setcdr last-cons (cons (list prefix-parser (cons index token-length)) nil))
        (setq last-cons (cdr last-cons))
        (setq index token-length)
        )
      ;; Match the rest of the tokens repeatedly
      (while (< index (length line))
        (let ((matched nil))
          (dolist (parse-fn other-parsers)
            (setq token-length (funcall parse-fn line index))
            (when token-length
              (setcdr last-cons (cons (list parse-fn (cons index (+ index token-length))) nil))
              (setq last-cons (cdr last-cons))
              (setq index (+ index token-length))
              (setq matched t)
              ))
          (unless matched
            (setq index (1+ index)))))
      (cdr tokens)
      ))

  (defun rt-token-list-to-string (line token-list)
    "Given a LINE and a TOKEN-LIST, assemble a new string based on the strings referenced in TOKEN-LIST and return it."
    (let ((result ""))
      (dolist (token token-list)
        (let
          (
            (start (car (cdr token)))
            (end (cdr (cdr token)))
            )
          (setq result (concat result (substring line start end)))))
      result
      ))

  (defun rt-format-lines-in-region (start end)
    "Format all lines within the selected region, defined by START and END, according to RT code style."
    (interactive "r")
    (save-excursion
      (goto-char start)
      (while 
        (< (point) end)
        (let*
          (
            (line-start (line-beginning-position))
            (line-end (line-end-position))
            (line (buffer-substring-no-properties line-start line-end))
            (token-list (rt-parse-line line rt-token-types))
            (formatted-line (rt-token-list-to-string line token-list))
            )
          ;;(message "Before formatting: %s" line) ; Message to show the line before formatting
          ;;(message "After formatting: %s" formatted-line) ; Message to show the line after formatting
          ;; Replace the line with the formatted version
          (delete-region line-start line-end)
          (insert formatted-line))
        ;; Move to the beginning of the next line
        (forward-line 1)
        )))

;;--------------------------------------------------------------------------------
;; format commas
;;

  (defun rt-format-comma (start end)
    "Format commas to follow RT code style in the selected region.
  Move spaces after a comma to before it, unless followed by a newline."
    (interactive "r") ; The "r" prefix allows it to get the start and end of the region
    (message "Starting comma formatting...")
    (save-excursion
      (goto-char start)
      ;; Step 1: Find commas with spaces before and after, and move spaces before the comma
      (while 
        (re-search-forward "\\( *\\),\\( +\\)\\([^\\n]\\)" end t)
        (let 
            (
              (leading-spaces (match-string 1))
              (trailing-spaces (match-string 2))
              (after (match-string 3))
              )
          (replace-match (concat trailing-spaces leading-spaces "," after))))
      )
    (message "Comma formatting completed.")
    )

;;--------------------------------------------------------------------------------
;; bind formatting to minor mode, trigger formatting on tab
;;

(defun rt-code-indent-and-format ()
  "Indent the current line or region and apply all RT code formatting rules."
  (interactive)
  ;; Step 1: Perform indentation first
  (indent-for-tab-command)

  ;; Step 2: Format commas in the selected region or line
  (if 
    (use-region-p) ; Check if a region is active
    (rt-format-comma (region-beginning) (region-end))
    (rt-format-comma (line-beginning-position) (line-end-position)))

  ;; Step 3: Parse and format enclosures in the selected region or line
  (if 
    (use-region-p)
    (rt-format-lines-in-region (region-beginning) (region-end))
    (rt-format-lines-in-region (line-beginning-position) (line-end-position))))

  (defvar rt-format-hook nil
    "Hook called by `rt-format-mode`."
    )

  (define-minor-mode rt-format-mode
    "Minor mode for RT code style formatting."
    :lighter " RTFmt"
    :keymap (let 
              (
                (map (make-sparse-keymap))
                )
              (define-key map (kbd "TAB") 'rt-code-indent-and-format) ; Bind TAB to run all formatting rules.
              map
              )
    (if 
      rt-format-mode
      (run-hooks 'rt-format-hook)
      ))

