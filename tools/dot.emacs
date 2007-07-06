;;;;;;;;;;;;;;;;;;;;;;;-*- mode: emacs-lisp -*-;;;;;;;;;;;;;;;;;;;;;;;
;;                
;; Copyright (C) ,  Karlsruhe University
;;                
;; File path:     dot.emacs
;; Description:   Various emacs initialization stuff
;;                
;; @LICENSE@
;;                
;; $Id: dot.emacs,v 1.3 2003/09/24 19:21:51 skoglund Exp $
;;                
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;; Add private elisp dirctory to the Emacs load path
(setq load-path (append (list (expand-file-name "~/.elisp"))
			load-path))

;;; C mode with Stroustrup indentation
(setq c-default-style "stroustrup")


;;; Auto header
(require 'auto-header)

(header-set-entry "license" nil "@LICENSE@")
(setcdr (assoc 'c++-mode header-comment-strings)
	(list "/*" "*/"  " *"  "*"))
(defconst header-function-hdrstyle-doc
  '(funcname ":" 12 mark "\n" ("@" param ":" 12" \n") blank)
  "Function-header style for auto documentation.")

(setq header-copyright-notice "Copyright (C) ,  Karlsruhe University"
      header-field-list '(blank copyright blank filepath description
				blank bsd blank cvsid blank)
      header-pathname-sep-reg (concat "\\(src/\\)\\|"
				      "\\(include/\\)\\|"
				      "\\(kernel/\\)\\|"
				      "\\(apps/\\)")
      header-function-hdrstyle header-function-hdrstyle-doc)



;;; Auto format of ifdef wrappers
(setq ifdef-pathname-match "\\(SRC/\\)\\|\\(INCLUDE/\\)\\|\\(KERNEL/\\)")

(defun insert-ifdef-stuff ()
  (interactive)
  (save-excursion
    (let ((fname (upcase buffer-file-name))
	  (parse-sexp-ignore-comments t))

      ;; Format file name
      (setq fname (cond ((not (null (string-match ifdef-pathname-match fname)))
			 (while (string-match ifdef-pathname-match fname)
			   (setq fname (substring fname (match-end 0))))
			 fname)
			((not (null (string-match "/" fname)))
			 (substring fname (match-end 0)))
			(t fname)))
      (while (string-match "/" fname)
	(setq fname (concat (substring fname 0 (match-beginning 0)) "__"
			    (substring fname (match-end 0)))))
      (while (string-match "[-+.]" fname)
	(setq fname (concat (substring fname 0 (match-beginning 0)) "_"
			    (substring fname (match-end 0)))))

      (goto-char (point-min))

      ;; Skip past comment (i.e., file header)
      (if (save-excursion
	    (or (eobp) (forward-char 2))
	    (nth 4 (parse-partial-sexp (point-min) (point))))
	  (progn
	    (forward-sexp 1)
	    (or (eobp) (backward-sexp 1))
	    (skip-chars-backward "\n\r\t ")
	    (or (eobp) (bobp) (insert "\n"))))

      ;; Insert stuff
      (insert "#ifndef __" fname "__\n#define __" fname "__\n")
      (goto-char (point-max))
      (insert "\n#endif /* !__" fname "__ */\n")
      )))

(define-key global-map [(control x) (control h) (d)] 'insert-ifdef-stuff)


;;; Linker script
(require 'linker-script)
(setq auto-mode-alist (cons '("\\.lds$" . linker-script-mode)
			    auto-mode-alist))
(setq header-comment-strings
      (append header-comment-strings
	      '((linker-script-mode . ("/*"   "*/"  " *"   "*")))))

