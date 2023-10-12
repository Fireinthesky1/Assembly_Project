        ;; Messing around with interrupts
        ;; We can attach ISRs tp interrupts bby writing the ISRs as 
        ;; regular assembly functions with no input or output parameters
        ;; and editing the Startup.s file to specify those functions for the
        ;; appropriate interrupt

        ;; We need two timers (in addition to the SysTick timer)
