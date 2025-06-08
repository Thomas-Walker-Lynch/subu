(defun rt-args-process (function args arg-specs)
  "Process ARGS according to ARG-SPECS.
Returns the processed ARGS list.

ARG-SPECS is a list where each element is an argument specification:
 - For required arguments: (TYPEP MESSAGE)
 - For optional arguments: (DEFAULT TYPEP MESSAGE)

The function validates the arguments, applies defaults for missing optional arguments,
and returns the list of arguments. If any validations fail, it signals an error
with all accumulated messages."
  (let ((error-messages '())
        (processed-args nil)
        (last-cons nil)  ;; Pointer to the last cons cell of processed-args
        (arg-specs-pt arg-specs)  ;; Pointer into arg-specs list
        (args-pt args))             ;; Pointer into args list
    ;; Validate that arg-specs and args are lists
    (unless (listp arg-specs)
      (error "%s:: arg-specs must be a list" (symbol-name function)))
    (unless (listp args)
      (error "%s:: args must be a list" (symbol-name function)))
    
    ;; Process each argument specification
    (while arg-specs-pt
      (let* ((spec (car arg-specs-pt))
             (required (eq (length spec) 2))  ;; If spec has 2 items, it's required
             (default (when (not required) (nth 0 spec)))
             (typep (if required (nth 0 spec) (nth 1 spec)))
             (message (if required (nth 1 spec) (nth 2 spec)))
             (arg (if args-pt (car args-pt) nil)))
        ;; Determine if we need to use the provided arg or apply default
        (if args-pt
            ;; Argument is provided
            (progn
              ;; Validate the provided argument
              (unless (funcall typep arg)
                (push message error-messages))
              ;; Add the argument to processed-args
              (let ((new-cons (cons arg nil)))
                (if (null processed-args)
                    (setq processed-args new-cons)
                  (setcdr last-cons new-cons))
                (setq last-cons new-cons))
              ;; Advance the args pointer
              (setq args-pt (cdr args-pt)))
          ;; Argument is not provided
          (if required
              ;; Missing required argument
              (push message error-messages)
            ;; Optional argument, apply default
            (let ((default-value (if (functionp default)
                                      (funcall default processed-args)
                                    default)))
              ;; Validate the default value
              (unless (funcall typep default-value)
                (push message error-messages))
              ;; Add the default value to processed-args
              (let ((new-cons (cons default-value nil)))
                (if (null processed-args)
                    (setq processed-args new-cons)
                  (setcdr last-cons new-cons))
                (setq last-cons new-cons)))))
        ;; Move to the next spec
        (setq arg-specs-pt (cdr arg-specs-pt))))
    
    ;; Check for extra arguments
    (when args-pt
      (push "Too many arguments provided." error-messages))
    
    ;; Report errors if any
    (when error-messages
      (error
       (concat
        "Errors in " (symbol-name function) ":\n"
        (mapconcat #'identity (reverse error-messages) "\n"))))
    
    ;; Return the processed arguments
    processed-args))
