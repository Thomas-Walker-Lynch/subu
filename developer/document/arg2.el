(defun rt-args-process (function args arg-specs)
  "Process ARGS according to ARG-SPECS.
Returns the processed ARGS list.

ARG-SPECS is a list where each element is an argument specification:
 - For required arguments: (TYPEP MESSAGE)
 - For optional arguments: (DEFAULT TYPEP MESSAGE)

The function validates the arguments, applies defaults for missing optional arguments,
and returns the list of arguments."
  (catch 'exit

    ;; Ensure args and arg-specs are provided
    (unless (listp args) (throw 'exit nil))
    (unless arg-specs (throw 'exit args))
    
    ;; Check for usage errors
    (let ((messages '()))
      (unless (listp arg-specs)
        (push "rt-args-process usage error: arg-specs must be a list" messages))
      (unless (listp args)
        (push "rt-args-process usage error: args must be a list" messages))
      (when messages
        (error "%s::\n  %s" (symbol-name function)
               (mapconcat #'identity messages "\n  "))))
    
    ;; Initialize variables
    (let ((processed-args nil)
          (last nil) ; Pointer to the last cons cell in processed-args
          (error-messages '())
          (args-pt args))

      ;; Process required arguments
      (let ((required-specs (car arg-specs)))
        (unless (listp required-specs)
          (error "%s:: arg-specs missing list of required argument specifications"
                 (symbol-name function)))
        (dolist (spec required-specs)
          (let ((typep (nth 0 spec))
                (message (nth 1 spec))
                (arg (if args-pt (car args-pt) nil)))
            (if arg
                (progn
                  ;; Type check the argument
                  (unless (funcall typep arg)
                    (push message error-messages))
                  ;; Add to processed-args
                  (let ((new-cons (cons arg nil)))
                    (if (null processed-args)
                        (setq processed-args new-cons)
                      (setcdr last new-cons))
                    (setq last new-cons))
                  ;; Move to the next argument
                  (setq args-pt (cdr args-pt)))
              ;; Missing required argument
              (push (concat "Missing required argument: " message) error-messages)))))

      ;; Process optional arguments
      (let ((optional-specs (cadr arg-specs)))
        (dolist (spec optional-specs)
          (let ((default (nth 0 spec))
                (typep (nth 1 spec))
                (message (nth 2 spec))
                (arg (if args-pt (car args-pt) nil)))
            (if arg
                (progn
                  ;; Type check the argument
                  (unless (funcall typep arg)
                    (push message error-messages))
                  ;; Add to processed-args
                  (let ((new-cons (cons arg nil)))
                    (if (null processed-args)
                        (setq processed-args new-cons)
                      (setcdr last new-cons))
                    (setq last new-cons))
                  ;; Move to the next argument
                  (setq args-pt (cdr args-pt)))
              ;; No argument provided, apply default
              (let ((default-value (if (functionp default)
                                       (funcall default processed-args)
                                     default)))
                ;; Type check the default value
                (unless (funcall typep default-value)
                  (push (concat "Default value error: " message) error-messages))
                ;; Add to processed-args
                (let ((new-cons (cons default-value nil)))
                  (if (null processed-args)
                      (setq processed-args new-cons)
                    (setcdr last new-cons))
                  (setq last new-cons)))))))

      ;; Check for extra arguments
      (when args-pt
        (push "Too many arguments provided." error-messages))

      ;; Print error messages if any
      (when error-messages
        (error "%s::\n  %s" (symbol-name function)
               (mapconcat #'identity (reverse error-messages) "\n  ")))

      ;; Return the processed arguments
      processed-args)))
