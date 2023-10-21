(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(custom-enabled-themes '(gruvbox-dark-soft))
 '(custom-safe-themes
   '("b1a691bb67bd8bd85b76998caf2386c9a7b2ac98a116534071364ed6489b695d" "2ff9ac386eac4dffd77a33e93b0c8236bb376c5a5df62e36d4bfa821d56e4e20" "72ed8b6bffe0bfa8d097810649fd57d2b598deef47c992920aef8b5d9599eefe" "d80952c58cf1b06d936b1392c38230b74ae1a2a6729594770762dc0779ac66b7" default))
 '(package-selected-packages '(gruvbox-theme)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

; Stop Emacs from losing undo information by
; setting very high limits for undo buffers
(setq undo-limit 20000000)
(setq undo-strong-limit 40000000)

;; Whitespace customization
(setq whitespace-style '(face tabs spaces space-mark lines-tail))
(setq whitespace-display-mappings
      '((newline-mark 0 [8729 10])
	(space-mark 32 [183] [46])
	(tab-mark 9 [9655 9] [92 9])))
(global-whitespace-mode 1)

;; use space instead of tabs
(setq-default indent-tabs-mode nil)

; 4-space tabs
(setq tab-width 4)
(setq indent-line-function 'insert-tab)

;; set whitespace mode
(global-whitespace-mode 1)

;; Enable the visual line mode
(global-visual-line-mode 1)

;; Set the visual line mode to wraps lines at 80
(setq visual-line-fringe-indicators '(nil right-curly-arrow))
(setq visual-line-column 76)

(setq whitespace-line-column 76)

; no screwing with my middle mouse button
(global-unset-key [mouse-2])

; Startup windowing
(setq next-line-add-newlines nil)
(setq-default truncate-lines t)
(setq truncate-partial-width-windows nil)
(split-window-horizontally)

;; Enable line numbers
(global-display-line-numbers-mode t)

;; move text up and down
(defun move-text-internal (arg)
  (cond
   ((and mark-active transient-mark-mode)
    (if (> (point) (mark))
        (exchange-point-and-mark))
    (let ((column (current-column))
          (text (delete-and-extract-region (point) (mark))))
      (forward-line arg)
      (move-to-column column t)
      (set-mark (point))
      (insert text)
      (exchange-point-and-mark)
      (setq deactivate-mark nil)))
   (t
    (let ((column (current-column)))
      (beginning-of-line)
      (when (or (> arg 0) (not (bobp)))
        (forward-line)
        (when (or (< arg 0) (not (eobp)))
          (transpose-lines arg))
        (forward-line -1))
      (move-to-column column t)))))

(defun move-text-down (arg)
  "Move region (transient-mark-mode active) or current line
  arg lines down."
  (interactive "*p")
  (move-text-internal arg))

(defun move-text-up (arg)
  "Move region (transient-mark-mode active) or current line
  arg lines up."
  (interactive "*p")
  (move-text-internal (- arg)))

(provide 'move-text)


(global-set-key [M-up] 'move-text-up)
(global-set-key [M-down] 'move-text-down)

; Bright-red TODOs
;; Define a font-lock rule to highlight comments containing TODO in C/C++
(font-lock-add-keywords
 'c++-mode
 '(("\\<\\(TODO\\)\\>" 1 font-lock-warning-face t)))

;; You can also add the rule to the generic c-mode
(font-lock-add-keywords
 'c-mode
 '(("\\<\\(TODO\\)\\>" 1 font-lock-warning-face t)))

;; Define a font-lock rule to highlight comments containing TODO in assembly
(add-hook 'asm-mode-hook
          (lambda ()
            (font-lock-add-keywords
             nil
             '(("\\<\\(TODO\\)\\>" 1 font-lock-warning-face t)))))

; No hungry backspace
(c-toggle-auto-hungry-state -1)


