; James Hicks Lab 3 October 2

        .thumb
        .global main

GPIO_UNLOCK_CODE:       .field      0x4C4F434B  ; (pg 684)
SYSCTRL_RCGCGPIO_R:     .field      0x400FE608  ; (pg 340)
SYSCTRL_RCC_R           .field      0x400FE060  ; (pg 254)

GPIO_PORTB_DIR_R:       .field      0x40005400  ; (pg 663)
GPIO_PORTB_DR4R_R:      .field      0x40005504  ; (pg 674)
GPIO_PORTB_DEN_R:       .field      0x4000551C  ; (pg 682)

GPIO_PORTF_LOCK_R:      .field      0x40025520  ; (pg 684)
GPIO_PORTF_CR:          .field      0x40025524  ; (pg 685)
GPIO_PORTF_PUR_R:       .field      0x40025510  ; (pg 677)
GPIO_PORTF_DEN_R:       .field      0x4002551C  ; (pg 682)

oDATA_READ_BOTH         .field      0x40025044  ; (pg 662) bit 0,4
oDATA_WRITE:            .field      0x4000503C  ; (pg 662) bit 0-4

; FOUR STATES OF STATE MACHINE
NEITHER_PRESSED:        .equ        0x00000011
ONLY_SW1_PRESSED:       .equ        0x00000001
ONLY_SW2_PRESSED:       .equ        0x00000010
BOTH_PRESSED:           .equ        0x00000000

main:
        .asmfunc

        AND     R0, #0
        AND     R1, #0

    ; ENABLE CLOCK TO PORT B,F (bit 1, 5)
        LDR     R0, SYSCTRL_RCGCGPIO_R
        LDR     R1, [R1]
        ORR     R1, #0x22                       ; (bit 1 and 5 high)
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; UNLOCK GPIO PORT F
        LDR     R0, GPIO_PORTF_LOCK_R
        LDR     R1, GPIO_UNLOCK_CODE
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; ENABLE COMMIT FOR PORT F
        LDR     R0, GPIO_PORTF_CR
        LDR     R1, [R0]
        ORR     R1, #0x11                       ; enable commit for pin 0,4
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; SET DIRECTION (PB0-PB4 outputs)
        LDR     R0, GPIO_PORTB_DIR_R
        LDR     R1, [R0]
        ORR     R1, #0x0F                       ; set PB0-4 as output
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; SET DRIVE STRENGTH FOR PORTB to 4-mA
        LDR     R0, GPIO_PORTB_DR4R_R
        LDR     R1, [R0]
        ORR     R1, #0x0F                       ; pins 0-4 have 4-mA drive
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; ENABLE PULL UP RESISTOR FOR PF0, PF4
        LDR     R0, GPIO_PORTF_PUR_R
        LDR     R1, [R0]
        ORR     R1, #0x11                       ; set PUR for PF0 and PF4
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; SET DIGITAL ENABLE (PB0-4, PF0, PF4)
        LDR     R0, GPIO_PORTB_DEN_R
        LDR     R1, [R0]
        ORR     R1, #0x0F                       ; enable for pin 0-4
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

        LDR     R0, GPIO_PORTF_DEN_R
        LDR     R1, [R0]
        ORR     R1, #0x11                       ; enable for pin 0,4
        STR     R1, [R0]

        AND     R0, #0
        AND     R1, #0

    ; ENABLE MOSC FOR SYSTEM CLOCK (254; bit 5 and 4 need to be 0)
        LDR     R0, SYSCTRL_RCC_R
        LDR     R1, [R0]
        EOR     R1, #0x30                       ; set bit 5 and 4 to 0
        STR     R1, [R0]

    ; USE 16 MHz CLOCK (set bits 10-6 to 0x15)
        LDR     R0, SYSCTRL_RCC_R
        LDR     R1, [R0]
        ORR     R1, #0x540
        BIC     R1, #0x0280
        STR     R1, [R0]

    ; CLEAR R1, R2, R3 AND INITIALIZE R0 FOR DISPLAY
        AND     R0, #0
        AND     R1, #0
        AND     R2, #0
        AND     R3, #0

DISPLAY:
        LDR     R1, oDATA_WRITE
        UBFX    R2, R0, #0, #4
        STR     R2, [R1]

IDLE:                                           ; HANDLE STATE TRANSITIONS
        LDR     R1, oDATA_READ_BOTH
        LDR     R2, [R1]
        CMP     R2, #ONLY_SW1_PRESSED
        BEQ     FLIP
        CMP     R2, #ONLY_SW2_PRESSED
        BEQ     SHIFT
        B       IDLE

FLIP:
        BL      wait                            ; WAIT
        MOVT    R1, #0xFFFF                     ; DO A FLIP
        MOVB    R1, #0xFFFF
        EOR     R0, R1
        LDR     R1, oDATA_WRITE                 ; DISPLAY OUTPUT
        UBFX    R2, R0, #0, #4
        STR     R2, [R1]
FLOOP:                                          ; HANDLE STATE TRANSITIONS
        LDR     R1, oDATA_READ_BOTH
        LDR     R2, [R1]
        CMP     R2, #NEITHER_PRESSED            ; IF NEITHER IDLE
        BEQ     IDLE
        CMP     R2, #BOTH_PRESSED               ; IF BOTH IN_FLIP_DO_SHIFT
        BEQ     IN_FLIP_DO_SHIFT
        B       FLOOP

IN_FLIP_DO_SHIFT:
        BL      wait                            ; WAIT
        LSL     R0, R0, #1                      ; DO A SHIFT
        ORR     R0, #1
        LDR     R1, oDATA_WRITE                 ; DISPLAY OUTPUT
        UBFX    R2, R0, #0, #4
        STR     R2, [R1]
IFLOOP:                                         ; HANDLE STATE TRANSITIONS
        LDR     R1, oDATA_READ_BOTH
        LDR     R2, [R1]
        CMP     R2, #NEITHER_PRESSED            ; IF NEITHER IDLE
        BEQ     IDLE
        CMP     R2, #ONLY_SW1_PRESSED           ; IF ONLY SW1 FLIP LOOP
        BEQ     FLOOP
        CMP     R2, #ONLY_SW2_PRESSED           ; IF ONLY SW2 SHIFT LOOP
        BEQ     SLOOP
        B       IFLOOP

SHIFT:
        BL      wait                            ; WAIT
        LSL     R0, R0, #1                      ; DO A SHIFT
        ORR     R0, #1
        LDR     R1, oDATA_WRITE                 ; DISPLAY OUTPUT
        UBFX    R2, R0, #0, #4
        STR     R2, [R1]
SLOOP:                                          ; HANDLE STATE TRANSITIONS
        LDR     R1, oDATA_READ_BOTH
        LDR     R2, [R1]
        CMP     R2, #NEITHER_PRESSED            ; IF NEITHER IDLE
        BEQ     IDLE
        CMP     R2, #BOTH_PRESSED               ; IF BOTH IN_SHIFT_DO_FLIP
        BEQ     IN_SHIFT_DO_FLIP
        B       SLOOP

IN_SHIFT_DO_FLIP:
        BL      wait                            ; WAIT
        MOVT    R1, #0xFFFF                     ; DO A FLIP
        MOVB    R1, #0xFFFF
        EOR     R0, R1
        LDR     R1, oDATA_WRITE                 ; DISPLAY OUTPUT
        UBFX    R2, R0, #0, #4
        STR     R2, [R1]
ISLOOP:                                         ; HANDLE STATE TRANSITIONS
        LDR     R1, oDATA_READ_BOTH
        LDR     R2, [R1]
        CMP     R2, #NEITHER_PRESSED            ; IF NEITHER IDLE
        BEQ     IDLE
        CMP     R2, #ONLY_SW1_PRESSED           ; IF ONLY SW1 FLIP LOOP
        BEQ     FLOOP
        CMP     R2, #ONLY_SW2_PRESSED           ; IF ONLY SW2 SHIFT LOOP
        BEQ     SLOOP
        B       ISLOOP
        .endasmfunc
        .align

; We want to drive the system clock directly from MOSC
; We need to change bits in System control register RCC
; R4, R5, R6 are used here

STCTRL:     .field      0xE000E010              ; (pg 138)
STRELOAD:   .field      0xE000E014              ; (pg 140)
STCURRENT:  .field      0xE000E018              ; (pg 141)

wait:
        .asmfunc

        AND     R4, #0
        AND     R5, #0
        AND     R6, #0

    ; DISABLE THE COUNTER (bit 0 low)
        LDR     R4, STCTRL
        LDR     R5, [R4]
        EOR     R5, #1                          ; set bit 0 low
        STR     R5, [R4]

        AND     R4, #0
        AND     R5, #0

    ; USE SYSTEM CLOCK (bit 2 high)
        LDR     R4, STCTRL
        LDR     R5, [R4]
        ORR     R5, #4                          ; set bit 4 high
        STR     R5, [R4]

        AND     R4, #0
        AND     R5, #0

    ; SET STRELOAD TO 0x27100
    ; be careful here ORR r5, R6 might not be correct
        LDR     R4, STRELOAD
        LDR     R5, [R4]
        MOVT    R6, #0x0002
        MOVB    R6, #0x7100
        ORR     R5, R6
        STR     R5, [R4]

        AND     R4, #0
        AND     R5, #0
        AND     R6, #0

    ; CLEAR THE COUNTER
        LDR     R4, STCURRENT
        AND     R5, #0
        STR     R5, [R4]                        ; clear reg clear count bit

        AND     R4, #0
        AND     R5, #0

    ; START THE TIMER
        LDR     R4, STCTRL
        LDR     R5, [R4]
        ORR     R5, #1                          ; sets enable bit of STCTRL
        STR     R5, [R4]

        AND     R4, #0
        AND     R5, #0

waitLoop:
        LDR     R4, STCTRL
        LDR     R5, [R4]
        MOVS    R5, R5, LSR #16                 ; just look at the 16th bit
        CMP     R5, #1
        BNE     waitLoop                        ; If not bit 16
        BX LR

        .endasmfunc
        .end