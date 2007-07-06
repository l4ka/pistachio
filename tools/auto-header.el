;;; auto-header.el --- Support for automatically updated file headers.

;; Copyright (C) 1996, 1998-2002, 2004-2005, Espen Skoglund.

;; Author: Espen Skoglund <esk@ira.uka.de>
;; Keywords: file headers

;; This file is NOT part of GNU Emacs.

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

;;; Commentary:

;; To use auto-header, you should put auto-header.el somewhere in the
;; emacs loadpath and optionally byte compile it.  Then you should
;; put the line:
;;
;;    (require 'auto-header)
;;
;; somewhere in your .emacs.

;; Auto-header defines the following global keys:
;;    C-x C-h m    -- Create header.
;;    C-x C-h c    -- Increase the update counter.
;;    C-x C-h i    -- Insert header field (prompted for in minibuffer).
;;    C-x C-h r    -- Make revision entry.  Turns on auto-fill.
;;    C-x C-h g    -- Goto field within header.  Turns on auto-fill.
;;    C-x C-h f    -- Insert function header.
;;    C-x C-h a    -- Toggle auto-fill-mode for headers.

;; The auto-header might be configured by the following variables:
;;
;;    header-full-name          Full name of the user.
;;    header-email-address      Email address of the user.
;;    header-field-list         List of fields to insert when creating
;;                              a new header.
;;    header-update-on-save     List of field entries to update when
;;                              saving files.  Possible elements are
;;                              `modified', `filename' and `counter'.
;;    header-copyright-notice   Text to be inserted when a copyright is
;;                              inserted into the header.
;;    header-function-hdrstyle  Style sheet for function headers.
;;
;; See also the header-set-entry function for instructions on how to
;; further customize your headers.  The header-list-fields function
;; displays a list of the current valid header field-types.

;; Acknowledgements:
;;   - Michael Hinz <michael@farmasi.uit.no> for customization
;;     improvements (and being a faithfull user).
;;   - Dag Brattli <dagb@cs.uit.no> for header-comment-strings additions.
;;   - Martin Trautmann <martintrautmann@gmx.de> for some C++ regexp fixes.



;;; History:
;; 

;;; Code:

(provide 'auto-header)

(defconst header-version "1.0.2"
  "Version of `auto-header.el'.")

;;;;
;;;; Keymap definitions
;;;;
(define-key global-map "\C-x\C-hm" 'header-make)
(define-key global-map "\C-x\C-hc" 'header-update-count)
(define-key global-map "\C-x\C-hi" 'header-insert-field)
(define-key global-map "\C-x\C-hr" 'header-make-revision)
(define-key global-map "\C-x\C-hg" 'header-goto-field)
(define-key global-map "\C-x\C-hf" 'header-make-fnheader)
(define-key global-map "\C-x\C-ha" 'header-toggle-autofill)

;;;;
;;;; User customizable variables
;;;;
(defgroup auto-header nil
  "Support for automatically updated file headers."
  :group 'tools)

(defcustom header-full-name nil
  "*Full name of user.
Defaults to the value returned by the function `user-full-name'."
  :type 'string
  :group 'auto-header)
;;; Default value
(or header-full-name
    (setq header-full-name (user-full-name)))

(defcustom header-email-address nil
  "*Email address of user.
Defaults to the address part of the variable `user-mail-address'."
  :type 'string
  :group 'auto-header)
;;; Default value
(or header-email-address
    (setq header-email-address (concat (user-login-name) "@" (system-name))))

(defcustom header-copyright-notice nil
  "*Copyright notice to be inserted in top of headers."
  :type 'string
  :group 'auto-header)
;;; Default value
(or header-copyright-notice
    (setq header-copyright-notice (concat "Copyright (C) ,  "
					  header-full-name)))

(defcustom header-field-list
  '(blank copyright blank filename author description blank cvsid blank)
  "*List of default fields to include in headers.
The fields will be created in the order they are listed.
The `header-set-entry' function may be used to add new valid field types or
change existing types.  \\[header-list-fields] will display a list of
valid types."
  :type '(repeat (choice (const :tag "Copyright notice" copyright)
			 (const :tag "Filename" filename)
			 (const :tag "File path" filepath)
			 (const :tag "Version number" version)
			 (const :tag "Description" description)
			 (const :tag "Author" author)
			 (const :tag "Created at" created)
			 (const :tag "Modified at" modified)
			 (const :tag "Modified by" modified_by)
			 (const :tag "Status" status)
			 (const :tag "Update counter" update)
			 (const :tag "CVS identifier" cvsid)
			 (const :tag "CVS log" cvslog)
			 (const :tag "GNU General Public License " gpl)
			 (const :tag "GNU Lesser General Public License " lgpl)
			 (const :tag "Two clause BSD License " bsd)
			 (const :tag "Blank line" blank)))
  :group 'auto-header)

(defcustom header-update-on-save
  '(filename modified copyright)
  "*List of fields that should be updated automatically upon saving.
Possible elements are: `filename', `modified', `counter', and `copyright'."
  :type '(set :extra-offset 8
	      (const :tag "Filename" filename)
	      (const :tag "Modified At/By" modified)
	      (const :tag "Update counter" counter)
	      (const :tag "Copyright notice" copyright))
  :group 'auto-header)

(defcustom header-copyright-spans t
  "*Should generated copyright notices contain year spans.
If t, a year span will be created if possible (e.g., `1998-1999').
If nil, the current year will be inserted after a comma (e.g., `1998, 1999')"
  :type 'boolean
  :group 'auto-header)

(defconst header-function-hdrstyle-long
  '("Function " funcname " (" (param ", " c) ")" blank
    " Returns\n" 3 mark blank
    " Parameters\n" ("   " param ":" 16 "\n") blank
    " Description\n" 3 blank)
  "Example of a long function-header style.")

(defconst header-function-hdrstyle-short
  '("Function " funcname " (" (param ", " c) ")"
    blank 3 mark blank)
  "Example of a short function-header style.")

(defconst header-function-hdrstyle-infunc
  '("Method " funcname " (" (param ", " c) ")"
    infunc "    \"\"\"\n" "    " funcname ": " mark "\n" "    \"\"\"\n")
  "Example of a header style using the `infunc' keyword.")

(defvar header-function-hdrstyle header-function-hdrstyle-short
  "*Description of what function-headers should look like.
The description is a list of strings, symbols, numbers and lists.

  Strings   Strings are just inserted into the header.

  Symbols   The following symbols may be used:
               funcname  --  The name of the function is inserted.
               blank     --  An empty line is inserted.
               mark      --  When the header is printed, the pointer is
                             moved to this point.  If severeal marks are set,
                             only the last one will take effect.
               infunc    --  Move control point into function body.  This is
                             useful, e.g., when inserting doc strings in Python
                             methods.  When `infunc' is used, lines will no
                             longer be prefixed with comment starters.  See
                             `header-function-hdrstyle-infunc' for example
                             usage.

  Numbers   Indent to column.

  Lists     Decalaration of parameterlist.  A list of strings, integers,
            symbols and numbers (and even more lists), that is parsed
            once for each parameter.  In addition to the ordinary symbols,
            the parameterlist may contain the symbols:
               param     --  The name of the current parameter.
               c         --  When this symbol is encountered, the element
                             that preceded this one is not evaluted if
                             the current parameter is the last one.

             E.g., if we declare the elements '(\"(\" (param \", \" c) \")\"),
             it will print out the following:

               (parameter1, some_parameter, another_parameter)

Two examples of function header styles are provided;
`header-function-hdrstyle-short' and `header-function-hdrstyle-long'.
The short version of the header style is the default.")

;;;;
;;;; Internal variables/constants
;;;;
(defvar header-update-func-alist
  '((filename  . header-update-filename)
    (modified  . header-update-modified)
    (counter   . header-update-count)
    (copyright . header-update-copyright))
  "Alist of functions to call when updating header fields.")

(defvar header-max-search 1500
  "Search limit for locating file header in beginning of buffer.")

(defvar header-fields
  '(("copyright" .   (nil header-copyright-notice))
    ("filename" .    ("Filename:" (file-name-nondirectory (buffer-file-name))))
    ("filepath" .    ("File path:" (header-get-filepath)))
    ("version" .     ("Version:" ""))
    ("description" . ("Description:" ""))
    ("author" .      ("Author:" (header-get-author)))
    ("created" .     ("Created at:" (current-time-string
				     (nth 6 (file-attributes
					     (buffer-file-name))))))
    ("modified" .    ("Modified at:" ""))
    ("modified_by" . ("Modified by:" ""))
    ("status" .      ("Status:" "Experimental, do not distribute."))
    ("update" .      ("Update count:" "0"))
    ("cvsid" .       (nil "$Id:$"))
    ("cvslog" .      (nil "$Log:$"))
    ("gpl" .         (nil "\
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA."))
    ("lgpl" .        (nil "\
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307  USA"))
    ("bsd" .         (nil "\
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE."))
    ("blank" .       ("" "")))
  "List of header fields -- their text and their default value.")


(defvar header-comment-strings
  ;;(major-mode      . (cstart cstop lipre fill))
  '((c-mode          . ("/*"   "*/"  " *"   "*"))
    (c++-mode        . ("//"   ""    "//"   "="))
    (asm-mode        . ("/*"   "*/"  " *"   "*"))
    (eiffel-mode     . ("--"   ""    "--"   "-"))
    (emacs-lisp-mode . (";"    ""    ";;"   ";"))
    (html-mode       . ("<!--" "-->" " ---" "-"))
    (idl-mode        . ("//"   ""    "//"   "="))
    (java-mode       . ("/*"   "*/"  " *"   "*"))
    (jde-mode        . ("/*"   "*/"  " *"   "*"))
    (ksh-mode        . ("#"    ""    "##"   "#"))
    (latex-mode      . ("%"    ""    "%%"   "%"))
    (LaTeX-mode      . ("%"    ""    "%%"   "%"))
    (lisp-mode       . (";"    ""    ";;"   ";"))
    (metafont-mode   . ("%"    ""    "%%"   "%"))
    (metapost-mode   . ("%"    ""    "%%"   "%"))
    (pascal-mode     . ("(*"   "*)"  " *"   "*"))
    (perl-mode       . ("#"    ""    "##"   "#"))
    (postscript-mode . ("%"    ""    "%%"   "%"))
    (prolog-mode     . ("/*"   "*/"  " *"   "*"))
    (python-mode     . ("#"    ""    "##"   "#"))
    (scheme-mode     . (";"    ""    ";;"   ";"))
    (sgml-mode       . ("<!--" "-->" " ---" "-"))
    (tcl-mode        . ("#"    ""    "##"   "#"))
    (tex-mode        . ("%"    ""    "%%"   "%"))
    (TeX-mode        . ("%"    ""    "%%"   "%"))
    (texinfo-mode    . ("@c "  ""    "@c "  "="))
    (text-mode       . ("#"    ""    "##"   "#"))
    (default         . ("#"    ""    "##"   "#")))
  "Description of which comments to use in the different modes.
The list contains entries of the form:

  (mode-name . (comment-start comment-end line-prefix fill-character))")

(defvar header-pathname-sep-reg "\\(src/\\)\\|\\(include/\\)")
(defvar header-line-width 70)
(defvar header-fieldtext-width 15)
(defvar header-old-fill-function nil)

;;;;
;;;; Variables for storing buffer specific information
;;;;
(defvar header-line-prefix nil)
(defvar header-comment-begin nil)
(defvar header-comment-end nil)
(defvar header-comment-char nil)
(defvar header-initialized-mode nil)
(make-variable-buffer-local 'header-line-prefix)
(make-variable-buffer-local 'header-comment-begin)
(make-variable-buffer-local 'header-comment-end)
(make-variable-buffer-local 'header-comment-char)
(make-variable-buffer-local 'header-initialized-mode)

(defvar header-line-prefix-re nil)
(defvar header-comment-begin-re nil)
(defvar header-comment-end-re nil)
(defvar header-comment-char-re nil)
(make-variable-buffer-local 'header-line-prefix-re)
(make-variable-buffer-local 'header-comment-begin-re)
(make-variable-buffer-local 'header-comment-end-re)
(make-variable-buffer-local 'header-comment-char-re)


;;;;
;;;; Functions for creating headers
;;;;

(defun header-split-string (str pat)
  "Split the string STR into substrings separated by PAT.
Return the list of substrings."
  (let* ((bg 0)
	 (tp (cons nil nil))
	 (rt tp))
    (while (string-match pat str bg)
      (setq tp (setcdr tp (cons (substring str bg (match-beginning 0)) nil))
	    bg (match-end 0)))
    (setcdr tp (cons (substring str bg) nil))
    (cdr rt)))

(defun header-regexpify (str)
  "Escape the characters *+-\\^$ in STR.  Return the new string."
  (let ((start 0)
	(ret "")
	match prev)
    (while (setq match (string-match "[*+\\-\\\\^$]" str start))
      (setq prev start
	    start (match-end 0)
	    ret (concat ret (substring str prev match) "\\"
			(substring str (match-beginning 0) start))))
    (concat ret (substring str start))))

(defun header-init (&optional quiet)
  "Set comment style according to current `major-mode'.
Optional argument QUIET, forces function to be silent."
  (let* ((mm major-mode) cs)
    (setq cs (if (assoc mm header-comment-strings)
		 (cdr (assoc mm header-comment-strings))
	       (if (not quiet)
		   (message (concat "No header comment style spciefied for "
				    "current mode.  Using default.")))
	       (cdr (assoc 'default header-comment-strings))))
    (setq header-comment-begin (nth 0 cs)
	  header-comment-end   (nth 1 cs)
	  header-line-prefix   (nth 2 cs)
	  header-comment-char  (nth 3 cs)
	  header-initialized-mode major-mode)
    (setq header-comment-begin-re (header-regexpify header-comment-begin)
	  header-comment-end-re   (header-regexpify header-comment-end)
	  header-line-prefix-re   (header-regexpify header-line-prefix)
	  header-comment-char-re  (header-regexpify header-comment-char))))

(defun header-exists-p ()
  "Return t if there exist a header in the current buffer, nil otherwise."
  (save-excursion
    (beginning-of-buffer)
    (if (looking-at "^#!")
	(forward-line 1))
    (if (not (equal header-initialized-mode major-mode))
	(header-init t))
;;;    (momentary-string-display  (concat "<^" header-comment-begin-re
;;;				       header-comment-char-re "+" "$>")
;;;			       (point))
    (and (search-forward-regexp (concat "^" header-comment-begin-re
					header-comment-char-re "+" "$")
				header-max-search t)
;;;	 (or (momentary-string-display "^" (point)) t)
	 (or (forward-line 1) t)
	 (looking-at (concat header-line-prefix-re " ")))))

;;;###autoload
(defun header-set-entry (name string contents)
  "Create a new header-entry type called NAME.
Second argument STRING is used as a constant string describing the
header type (e.g. \"Version:\").  Third argument CONTENTS is evalueted
at header creation time to generate the default header contents."
  (let ((cdr-ent (assoc name header-fields)))
    (if cdr-ent
	(setcdr cdr-ent (list string contents))
      (setq header-fields
	    (cons (list name string contents) header-fields)))))

(defun header-list-fields ()
  "Display a list of all valid header types."
  (interactive)
  (let ((buf (current-buffer)))
    ;; Create buffer to show list
    (set-buffer (setq tbuf (get-buffer-create "*Valid header types*")))
    (delete-region (point-min) (point-max))
    (insert "Valid header types:\n\n"
	    (format "%-14s %-16s %s\n"
		    "Field type" "Field string" "Default value")
	    (make-string 14 ?-) " " (make-string 16 ?-) " "
	    (make-string 16 ?-) "\n")
    (display-buffer tbuf t)
    (mapcar '(lambda (hf)
	       (let ((a (car hf))
		     (b (cadr hf))
		     (c (caddr hf)))
		 (insert (format "%-14s %-16s %s\n" a
				 (and b (concat "\"" b "\""))
				 (if (stringp c)
				     (concat "\"" c "\"")
				   c)))))
	    header-fields)
    (set-buffer buf)))

;;;###autoload
(defun header-make (force)
  "Insert a header at the top of the current buffer.
A prefix argument, FORCE, forces the header to be inserted even if there
already exists a header."
  (interactive "P")
  (if (not (equal header-initialized-mode major-mode))
      (header-init))
  (if (and (not force) (header-exists-p))
      (message "Header already exists.")
    (beginning-of-buffer)
    (if (looking-at "^#!")
	(forward-line 1))
    (insert header-comment-begin
	    (make-string (- header-line-width (length header-comment-begin))
			 (string-to-char header-comment-char))
	    "\n")
    (mapcar 'header-insert-field header-field-list)
    (insert header-line-prefix
	    (make-string (- header-line-width (length header-comment-end)
			    (length header-line-prefix))
			 (string-to-char header-comment-char))
	    header-comment-end "\n")))

;;;###autoload
(defun header-insert-field (field)
  "Insert a header FIELD into to current buffer."
  (interactive (list (completing-read "Field: " header-fields)))

  (and (symbolp field) (setq field (symbol-name field)))
  (if (not (equal header-initialized-mode major-mode))
      (header-init t))

  (let* ((fieldent (assoc field header-fields))
	 (pretext (car (cdr fieldent)))
	 (content (eval (car (cdr (cdr fieldent)))))
	 (spaces (- header-fieldtext-width (length pretext))))

    ;; Insert the new line
    (beginning-of-line)
    (cond ((null pretext)
	   ;; Insert ordinary (possibly multiline) text
	   (mapcar '(lambda (line)
		      (insert header-line-prefix " " line "\n"))
		   (header-split-string content "\n")))
	  (t
	   ;; Insert header line
	   (insert header-line-prefix " " pretext
		   (make-string (if (< spaces 0) 0 spaces) ? )
		   content "\n"))))
  (if (interactive-p)
      (progn
	(backward-char 1)
	(or (eq auto-fill-function 'header-fill-function)
	    (setq header-old-fill-function auto-fill-function))
	(setq auto-fill-function 'header-fill-function))))

(defun header-get-author ()
  "Make a string containing ``Full name <Email address>''."
  (concat header-full-name " <" header-email-address ">"))

(defun header-get-filepath ()
  "Make s string of the current filename with some of its path added.
The part added to the filename is the path which comes after any ``src'' or
``include'' directory in the full pathname.  For example, a file
``/project/include/foo/bar.h'' will give ``foo/bar.h''."
  (let ((fname (buffer-file-name))
	(case-fold-search t))
    (while (string-match header-pathname-sep-reg fname)
      (setq fname (substring fname (match-end 0))))
    (if (string= fname (buffer-file-name))
	(file-name-nondirectory fname)
      fname)))


;;;;
;;;; Update functions
;;;;

;;;###autoload
(defun header-goto-field (field)
  "Go to header FIELD prompted for in minibuffer."
  (interactive (list (completing-read "Field: " header-fields)))
  (and (symbolp field) (setq field (symbol-name field)))
  (let (pos)
    (save-excursion
      (beginning-of-buffer)
      (if (search-forward (concat header-line-prefix " "
				  (car (cdr (assoc field header-fields))))
			  header-max-search t)
	  (setq pos (point))))
    (if (not pos)
	(and (interactive-p)
	     (message "Unable to find `%s' field." field) nil)
      (goto-char pos)
      (forward-char 1)
      (skip-chars-forward " \t")
      (if (interactive-p)
	  (progn
	    (or (eq auto-fill-function 'header-fill-function)
		(setq header-old-fill-function auto-fill-function))
	    (setq auto-fill-function 'header-fill-function)))
      t)))

(defun header-update-modified ()
  "Update the `modified' and `modified_by' part of the header."
  (save-excursion
    (if (header-goto-field 'modified)
	(progn
	  (delete-region (point) (progn (end-of-line) (point)))
	  (insert (current-time-string))))
    (if (header-goto-field 'modified_by)
	(progn
	  (delete-region (point) (progn (end-of-line) (point)))
	  (insert (header-get-author))))))

;;;###autoload
(defun header-update-count ()
  "Update the `Update count' field in the header."
  (interactive)
  (save-excursion
    (if (header-goto-field 'update)
	(let* ((beg (point))
	       (end (progn (end-of-line) (point)))
	       (cnt (buffer-substring beg end)))
	  (delete-region beg end)
	  (insert (int-to-string (1+ (string-to-int cnt)))))
      (message "No update counter found."))))

(defun header-update-filename ()
  "Update filename in the header."
  (interactive)
  (save-excursion
    (if (header-goto-field 'filename)
	(progn
	  (delete-region (point) (progn (end-of-line) (point)))
	  (insert (file-name-nondirectory (buffer-file-name)))))
    (if (header-goto-field 'filepath)
	(progn
	  (delete-region (point) (progn (end-of-line) (point)))
	  (insert (header-get-filepath))))))


(defun header-update-copyright ()
  "Update copyright field in the header."
  (save-excursion
    (beginning-of-buffer)
    (let ((case-fold-search t)
	  (year-re (format-time-string
		    (concat "\\(%Y\\)\\|"
			    "\\([0-9][0-9]\\([0-9][0-9]\\)-%y\\)\\|"
			    "\\([0-9][0-9][0-9][0-9]-%Y\\)")
;			    "\\([0-9]\\{2,4\\}-%y\\)\\|"
;			    "\\([0-9]\\{4,4\\}-%Y\\)")
		    (current-time)))
	  (cur-year (format-time-string "%Y" (current-time)))
	  prev-match prev-year bpos epos)
      (if (re-search-forward (concat "^[ \t]*" header-line-prefix-re
					  ".*copyright (c) *")
			     header-max-search t)
	  (catch 'done
	    (while t
	      (if (looking-at year-re)
		  ;; Current year exists in copyright notice
		  (throw 'done t))
;	      (if (looking-at "[0-9]\\{2,4\\}\\(-[0-9]\\{2,4\\}\\)?")
	      (if (looking-at (concat "[0-9][0-9]\\([0-9][0-9]\\)?"
				      "\\([0-9][0-9]\\([0-9][0-9]\\)?\\)?"))
		  (progn
		    (setq prev-match t)
		    (goto-char (match-end 0))
		    (skip-chars-forward "-, "))
		;; It's time to insert current year
		(skip-chars-backward ", ")
		(if (and header-copyright-spans prev-match
			 (save-excursion
			   (setq epos (point)
				 bpos (progn (skip-chars-backward "0-9")
					     (point))
				 prev-year (string-to-number
					    (buffer-substring bpos epos)))
			   (and (< prev-year 100)
				(setq prev-year (+ 1900 prev-year)))
			   (= (1+ prev-year) (string-to-number cur-year))))
		    ;; We should do a year span
		    (if (/= (char-before bpos) ?-)
			(progn
			  (goto-char epos)
			  (insert "-" cur-year)
			  (throw 'done nil))
		      (delete-region bpos epos)
		      (insert cur-year)
		      (throw 'done nil)))
		;; No year span.  Just insert current year
		(if prev-match
		    (insert ", ")
		  (skip-chars-forward " "))
		(insert (format-time-string "%Y" (current-time)))
		(throw 'done nil))))
	))))


(defun header-automatic-update ()
  "Set variable `auto-fill-function' to its original value."
  (and (eq auto-fill-function 'header-fill-function)
       (setq auto-fill-function header-old-fill-function))

  ;; Calls the update function for each element in the
  ;; `header-update-on-save' list.
  (if (header-exists-p)
      (mapcar '(lambda (f)
		 (funcall (cdr (assoc f header-update-func-alist))))
	      header-update-on-save))
  nil)

;;; Call header-automatic-update upon writing files
(add-hook 'write-file-hooks 'header-automatic-update)


;;;;
;;;; Revision history
;;;;

;;;###autoload
(defun header-make-revision ()
  "Add a revision entry."
  (interactive)
  (if (not (equal header-initialized-mode major-mode))
      (header-init))
  (let* ((cc header-comment-char-re)
	 (e  header-comment-end-re)
	 (lp header-line-prefix-re)
	 (case-fold-search t)
	 (date (current-time-string))
	 pos)
    (setq pos
	  (save-excursion
	    (beginning-of-buffer)
	    (cond ((search-forward-regexp
		    (concat "^" lp " +\\(revision \\)?history:?[ \t]*$")
		    header-max-search t)
		   (forward-line 2)
		   (point))
		  ((search-forward-regexp (concat "^" lp cc "+" e "$"))
		   (forward-line 0)
		   (insert header-line-prefix "\n"
			   header-line-prefix " Revision History:\n"
			   header-line-prefix "\n")
		   (point))
		  (t nil))))
    (if (not pos)
	(message "No header found.")

      (goto-char pos)
      (insert (format "%s   %02d-%s-%02d %s   %s <%s>\n%s     \n%s\n"
		      header-line-prefix
		      (string-to-int (substring date 8 10))
		      (substring date 4 7)
		      (string-to-int (substring date -2))
		      (substring date 11 19)
		      header-full-name
		      header-email-address
		      header-line-prefix header-line-prefix ))
      (forward-line -2)
      (end-of-line)
      (or (eq auto-fill-function 'header-fill-function)
	  (setq header-old-fill-function auto-fill-function))
      (setq auto-fill-function 'header-fill-function)
      )))


;;;;
;;;; Auto filling
;;;;

(defun header-fill-function ()
  "Perform auto-filling in file header.
Old auto-fill function will be restored if calling this function outside
of a file header."
  (let ((fp (save-excursion
	      (beginning-of-line)
	      ;; Decide which fill prefix to use
	      (cond ((or (looking-at (concat "^\\([ \t]*\\)"
					     header-line-prefix-re
					     " +[a-zA-Z_][a-zA-Z0-9_]*: *"))
			 (looking-at (concat "^\\([ \t]*\\)"
					     header-line-prefix-re
					     " [^ \t].*: *")))
		     (concat (if (match-beginning 1)
				 (buffer-substring (match-beginning 1)
						   (match-end 1))
			       "")
			     header-line-prefix
			     (make-string (progn
					    (goto-char (match-end 0))
					    (- (current-column)
					       (length header-line-prefix)))
					  ? )))
		    ((looking-at (concat "^[ \t]*" header-line-prefix-re " +"))
		     (buffer-substring (point) (match-end 0)))
		    (t nil)))))
    (cond ((null fp)
	   ;; Not within header.  Restore old auto-fill-function
	   (setq auto-fill-function header-old-fill-function)
	   (and auto-fill-function (funcall auto-fill-function)))
	  ((> (current-column) fill-column)
	   ;; Do automatic line wrapping
	   (delete-horizontal-space)
	   (if (<= (current-column) fill-column)
	       (insert "\n" fp)
	     (forward-word -1)
	     (delete-horizontal-space)
	     (insert "\n" fp)
	     (end-of-line)
	     (insert " "))))))

(defun header-toggle-autofill ()
  "Toggle `auto-fill-mode' for headers."
  (interactive)
  (setq auto-fill-function
	(if (eq auto-fill-function 'header-fill-function)
	    header-old-fill-function
	  'header-fill-function))
  (and (fboundp 'redraw-modeline)
       (redraw-modeline)))

;;;;
;;;; Function headers
;;;;

(defvar header-function-recognize
  (list
   (cons 'c-mode (concat "^\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			 "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			 "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			 "\\(\\*+[ \t]*\\)?\\([ \t]\\*+\\*\\)?"
			 "\\([a-zA-Z_][a-zA-Z0-9_]*\\)[ \t]*"
			 "(\\([^)]*\\))[ \t\n]*[^;,]"))
   (cons 'c++-mode (concat "^[ \t]*\\(template[ \t\n]*<[^>]*>[ \t\n]*\\)?"
			   "\\(^\\|[a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)"
			   "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			   "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			   "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			   "\\(\\*+[ \t]*\\)?"
			   "\\([ \t]\\*+\\*\\)?"
			   "\\([a-zA-Z_][a-zA-Z0-9_]*"
			     "\\(<[^>]*>\\)?"
			     "\\(::[a-zA-Z_][a-zA-Z0-9_]*"
			       "\\(<[^>]*>\\)?\\)*[ \t\n]+\\)?"
			   "\\([a-zA-Z_][a-zA-Z0-9_]*"
			     "\\(<[^>]*>\\)?"
			     "\\(::[a-zA-Z_][a-zA-Z0-9_]*"
			     "\\(<[^>]*>\\)?\\)*\\)[ \t]*"
			   "(\\([^)]*\\))[ \t\n]*[^;,]"))
   (cons 'java-mode (concat "^[ \t]*"
		    "\\(\\(public\\|private\\|protected\\)+[ \t\n]+\\)*"
			    "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			    "\\([a-zA-Z_][a-zA-Z0-9_]*[ \t\n]+\\)?"
			    "\\(\\*+[ \t]*\\)?\\([ \t]\\*+\\*\\)?"
			    "\\([a-zA-Z_][a-zA-Z0-9_]*\\)[ \t]*"
			    "(\\([^)]*\\))[ \t\n]*[^;,]"))
   (cons 'pascal-mode (concat "^\\(function\\|procedure\\)[ \t\n]+"
			      "\\([^(; \t\n]+\\)[ \t]*(\\([^)]*\\))"))
   (cons 'perl-mode  (concat "^sub[ \t]+\\([[a-zA-Z_][a-zA-Z0-9_]*\\)[ \t]*"
			     "\\(([^)])\\)?[^{]*{"))
   (cons 'python-mode (concat "^[ \t]*def[ \t]+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
			      "[ \t]*\\((\\([^)]*\\))\\)?[ \t]*:"))
   (cons 'metapost-mode (concat "^[ \t]*"
				"\\(var\\|primary\\|secondary\\|tertiary\\)?"
				"def[ \t]+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
				"\\(@#\\)?[ \t]*\\([^=]*\\)="))
   (cons 'scheme-mode (concat "^[ \t]*"
			      "(define[ \t]+(\\(\\S-+\\)[ \t]*"
			      "\\([^()]*\\)?)"))
   (cons 'emacs-lisp-mode (concat "^[ \t]*"
				  "(defun[ \t]+\\(\\S-+\\)[ \t]*"
				  "(\\(&optional[ \t\n]+\\)?\\([^)]*\\)"))
   
    )
  "Alist of major modes and regexps to recognize beginning of functions.")

(defvar header-name-of-function
  '((c-mode . 6)
    (c++-mode . 12)
    (java-mode . 7)
    (pascal-mode . 2)
    (perl-mode . 1)
    (python-mode . 1)
    (metapost-mode . 2)
    (scheme-mode . 1)
    (emacs-lisp-mode . 1)
    ))

(defvar header-find-parmas-func
  '((c-mode . header-find-params-c)
    (c++-mode . header-find-params-c++)
    (java-mode . header-find-params-java)
    (pascal-mode . header-find-params-pascal)
    (python-mode . header-find-params-python)
    (metapost-mode . header-find-params-metapost)
    (scheme-mode . header-find-params-scheme)
    (emacs-lisp-mode . header-find-params-emacs-lisp)
    ))

;;;###autoload
(defun header-make-fnheader ()
  "Make a function header."
  (interactive)
  (if (not (equal header-initialized-mode major-mode))
      (header-init))
  (if (not (assoc major-mode header-function-recognize))
      (message "Function headers are not supported in %s." major-mode)

    (let* ((case-fold-search  t)
	   (funre (cdr (assoc major-mode header-function-recognize)))
	   (funpos (cond ((progn (beginning-of-line) (looking-at funre))
			  (point))
			 ((re-search-backward funre nil t)
			    (if (save-excursion
				  (beginning-of-line 0)
				  (looking-at funre))
				(progn
				  (beginning-of-line 0)
				  (point))
			      (point)))
			 (t nil)))
	   (fi (cdr (assoc major-mode header-name-of-function)))
	   (funcname  (if funpos
			  (buffer-substring (match-beginning fi)
					    (match-end fi))
			"..."))
	   (infunc-mark (set-marker (make-marker) (match-end 0)))
	   (prespc (and (looking-at "^[ \t]*")
			(buffer-substring (match-beginning 0) (match-end 0))))
	   (fpfunc (cdr (assoc major-mode header-find-parmas-func)))
	   (params (if fpfunc (funcall fpfunc) ()))
	   (combeg (if (string= header-comment-end "")
		       (concat prespc header-line-prefix)
		     (concat prespc header-comment-begin)))
	   (linpre (concat prespc header-line-prefix))
	   (no-linepre nil)
	   (comend (if (and (>= (length header-comment-end) 1)
			    (string= (substring header-line-prefix -1)
				     (substring header-comment-end 0 1)))
		       (concat prespc header-line-prefix
			       (substring header-comment-end 1))
		     (concat prespc header-line-prefix header-comment-end))))
      
      (if (not funpos)
	  (message "[%s] Unable to find beginning of function" funre)
	
	(let ((stl header-function-hdrstyle)
	      marker nl se)
	  (goto-char funpos)
	  (insert combeg "\n")
	  (while (setq se (prog1 (car stl) (setq stl (cdr stl))))
	    (and (bolp) (if no-linepre
			    (insert prespc)
			  (insert linpre " ")))
	    (setq nl nil)
	    (insert (cond ((eq se 'blank)
			   ;; Blank entry
			   (if (= (current-column)
				  (1+ (length linpre)))
			       "\n"
			     (concat "\n" linpre "\n")))
			  ((or (eq se 'funcname) (eq se 'funcname-n))
			   ;; Function name
			   (setq nl (eq se 'funcname-n))
			   funcname)
			  ((eq se 'infunc)
			   ;; Skip into function body
			   (setq linpre (concat prespc "")
				 no-linepre t)
			   (insert "\n" comend "\n")
			   (goto-char (marker-position infunc-mark))
			   (and (eolp) (insert "\n"))
			   linpre)
			  ((integerp se)
			   ;; Indent to column
			   (let ((ind (+ se 1 (length linpre)))
				 (cc (current-column)))
			     (if (>= cc ind)
				 ""
			       (make-string (- ind cc) ? ))))
			  ((stringp se)
			   ;; String
			   se)
			  ((consp se)
			   ;; Parameterlist
			   (let ((pl (apply 'append
					    (mapcar
					     '(lambda (p)
						(mapcar
						 '(lambda (e)
						    (cond ((eq e 'param) p)
							  ((eq e 'param-n)
							   (concat p "\n"))
							  (t e)))
						 se))
					     params))))
			     ;; Remove element preceding 'c
			     (setq pl (nreverse pl))
			     (cond ((eq (car pl) 'c)
				    (setq pl (cdr (cdr pl))))
				   ((member 'c pl)
				    (setcdr (member 'c pl)
					    (cdr (cdr (member 'c pl))))))
			     (setq stl (append (delete 'c (nreverse pl)) stl)))
			   "")
			  ((eq se 'mark)
			   ;; Set marker
			   (setq marker (point))
			   "")
			  (t "")))
	    (and nl (insert "\n")))
	  (or no-linepre (insert comend "\n"))
	  (and marker (goto-char marker)))))
    ;; Turn on auto-fill
    (or (eq auto-fill-function 'header-fill-function)
	(setq header-old-fill-function auto-fill-function))
    (setq auto-fill-function 'header-fill-function)))

(defun header-find-params-c ()
  "Find the parameter list of current C function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (parbeg (progn (looking-at funre)
			(match-beginning 7)))
	 (parend (match-end 7))
	 (params (header-split-string (buffer-substring parbeg parend)
				      "[ \t\n]*,[ \t\n]*")))
    (if (and (null (cdr params))
	     (car params)
	     (string-match "\\<void$" (car params)))
	nil
      (mapcar '(lambda (parm)
		 (if (string-match (concat "\\(\\(\\.\\.\\.\\)\\|"
					   "\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\)"
					   "[ \t]*\\(\\[\\]\\)*"
					   "[ \t]*$") parm)
		     (substring parm (match-beginning 1) (match-end 1))
		   ""))
	      params))))

(defun header-find-params-c++ ()
  "Find the parameter list of current C++ function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (parbeg (progn (looking-at funre)
			(match-beginning 16)))
	 (parend (match-end 16))
	 (params (header-split-string (buffer-substring parbeg parend)
				      "[ \t\n]*,[ \t\n]*")))
    (if (and (null (cdr params))
	     (car params)
	     (string-match "\\<void$" (car params)))
	nil
      (mapcar '(lambda (parm)
		 (if (string-match (concat "\\(\\(\\.\\.\\.\\)\\|"
					   "\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\)"
					   "[ \t]*\\(\\[\\]\\)*"
					   "[ \t]*$") parm)
		     (substring parm (match-beginning 1) (match-end 1))
		   ""))
	      params))))


(defun header-find-params-java ()
  "Find the parameter list of current Java function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (parbeg (progn (looking-at funre)
			(match-beginning 8)))
	 (parend (match-end 8))
	 (params (header-split-string (buffer-substring parbeg parend)
				      "[ \t\n]*,[ \t\n]*")))
    (if (and (null (cdr params))
	     (car params)
	     (string-match "\\<void$" (car params)))
	nil
      (mapcar '(lambda (parm)
		 (if (string-match (concat "\\(\\(\\.\\.\\.\\)\\|"
					   "\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\)"
					   "[ \t]*\\(\\[\\]\\)*"
					   "[ \t]*$") parm)
		     (substring parm (match-beginning 1) (match-end 1))
		   ""))
	      params))))

(defun header-find-params-pascal ()
  "Find the parameter list of current Pascal function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (parbeg (progn (looking-at funre)
			(match-beginning 3)))
	 (parend (match-end 3))
	 (params (header-split-string (buffer-substring parbeg parend) ";"))
	 )

    (apply 'append
	   (mapcar '(lambda (somepar)
		      (setq somepar
			    (if (string-match ":.*$" somepar)
				(substring somepar 0 (1- (match-beginning 0)))
			      ""))
		      (if (string-match "[ \t]*\\(var\\)?[ \t]+" somepar)
			  (setq somepar (substring somepar (match-end 0))))
		      (if (string-match "[ \t]+$" somepar)
			  (setq somepar (substring somepar 0
						   (match-beginning 0))))
		      (header-split-string somepar "[ \t]*,[ \t]*"))
		   params)
	   )))

(defun header-find-params-python ()
  "Find the parameter list of current Python function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (parbeg (progn (looking-at funre)
			(match-beginning 3)))
	 (parend (match-end 3))
	 (params (if parbeg
		     (header-split-string (buffer-substring parbeg parend)
					  "[ \t\n]*,[ \t\n]*")
		   nil)))
    (mapcar '(lambda (parm)
	       (if (string-match (concat "[a-zA-Z_][a-zA-Z0-9_]*"
					 "\\(\\s-*=\\s-*"
					 "[a-zA-Z_][a-zA-Z0-9_]*"
					 "\\)?$") parm)
		   (substring parm (match-beginning 0) (match-end 0))
		 ""))
	    params)))

(defun header-find-params-metapost ()
  "Find the parameter list of current MetaPost function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (argbeg (progn (looking-at funre)
			(match-beginning 4)))
	 (argend (match-end 4))
	 (argstr (if argbeg (buffer-substring argbeg argend) ""))
	 (argstr (replace-in-string argstr ")[ \t\n]*" "," t))
	 (argstr (replace-in-string argstr ")[ \t\n]*" "," t))
	 (argstr (replace-in-string argstr "[ \t\n]*(" "" t))
	 (argstr (replace-in-string argstr ",[ \t\n]*$" "" t))
	 (params (header-split-string argstr "[ \t\n]*,[ \t\n]*")))
    (mapcar '(lambda (parm)
	       (if (string-match "\\([a-zA-Z_][a-zA-Z0-9_]*\\)[ \t]*$" parm)
		   (substring parm (match-beginning 1) (match-end 1))
		 ""))
	    params)))

(defun header-find-params-scheme ()
  "Find the parameter list of current Scheme function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (argbeg (progn (looking-at funre)
			(match-beginning 2)))
	 (argend (match-end 2))
	 (args (if argbeg
		   (header-split-string (buffer-substring argbeg argend)
					"[ \t\n]+\\(\\.[ \t\n]+\\)?")
		 nil)))
    args))

(defun header-find-params-emacs-lisp ()
  "Find the parameter list of current Emacs Lisp function."
  (let* ((funre (cdr (assoc major-mode header-function-recognize)))
	 (argbeg (progn (looking-at funre)
			(match-beginning 3)))
	 (argend (match-end 3))
	 (args (if argbeg
		   (header-split-string (buffer-substring argbeg argend)
					"\\([ \t\n]+&optional\\)?[ \t\n]+")
		 nil)))
    args))

;;; auto-header.el ends here
