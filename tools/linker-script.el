;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                
;; Copyright (C) 2002,  Karlsruhe University
;;                
;; File path:     linker-script.el
;; Description:   Dummy major-mode for editing linker scripts
;;                
;; @LICENSE@
;;                
;; $Id:$
;;                
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar linker-script-mode-syntax-table nil
  "Syntax table in use in Linker-script--mode buffers.")

(if linker-script-mode-syntax-table
    ()
  (setq linker-script-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?/ ". 14"  linker-script-mode-syntax-table)  
  (modify-syntax-entry ?* ". 23b" linker-script-mode-syntax-table))

(defconst linker-script-font-lock-keywords
  (purecopy
   (list
    '("\\([a-zA-Z_]+\\)[ \t]*(.*)"
      1 font-lock-keyword-face)
    '("^[ \t]*\\(\\.[a-zA-Z_][a-zA-Z_0-9.]+\\)"
      1 font-lock-function-name-face)
    '("\\([a-zA-Z_][a-zA-Z_0-9]+\\)[ \t]*="
      1 font-lock-variable-name-face)
    '("^[ \t]*\\(\\.[^ \t]+\\)[ \t]+\\([a-zA-Z_][a-zA-Z_0-9]+\\)[ \t]*:"
      2 font-lock-variable-name-face)))
  "Additional expression to highlight in Linker-script mode.")
(put 'linker-script-mode 'font-lock-defaults
     '(linker-script-font-lock-keywords nil t))


(defun linker-script-mode ()
  "Major mode for editing linker scripts."
  (interactive)
  (setq major-mode 'linker-script-mode)
  (setq mode-name "Linker-Script")
  (set-syntax-table linker-script-mode-syntax-table)
  (make-local-variable 'comment-start)
  (setq comment-start "/*")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *")
  (make-local-variable 'comment-end)
  (setq comment-end "*/")
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(linker-script-font-lock-keywords nil t))
  (run-hooks 'linker-script-mode-hook))


(provide 'linker-script)
