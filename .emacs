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

;; Whitespace customization
(setq whitespace-style '(face tabs spaces space-mark lines-tail))
(setq whitespace-display-mappings
      '((newline-mark 0 [8729 10])
	(space-mark 32 [183] [46])
	(tab-mark 9 [9655 9] [92 9])))
(global-whitespace-mode 1)

;; remove automatic indenting
(electric-indent-mode -1)

;; use space instead of tabs
(setq-default indent-tabs-mode nil)

;; set whitespace mode
(global-whitespace-mode 1)

;; Enable the visual line mode
(global-visual-line-mode 1)

;; Set the visual line mode to wraps lines at 80
(setq visual-line-fringe-indicators '(nil right-curly-arrow))
(setq visual-line-column 80)

(setq whitespace-line-column 80)
