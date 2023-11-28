; James Hicks SYSTICK PRACTICE
; NOV 5 2023

;;; INITIALIZATION SEQUENCE FOR SYSTICK
;;; 1) Program the value in STRELOAD register
;;; 2) Clear the STCURRENT register by writing to it with any value
;;; 3) Configure the STCTRL register for the required operation

        .thumb
        .global main

SYSCTL_BASE:    .field      0x400FE000  ; (pg)
SYSTICK_BASE:   .field      0xE000E000  ; (pg 138)
PORTF_BASE:     .field      0x40025000  ; (pg)

oRCGCGPIO:      .equ        0x608       ; (pg 340)
oSTCTRL:        .equ        0x010       ; (pg 138)
oSTCURRENT:     .equ        0x018       ; (pg 141)
oSTRELOAD:      .equ        0x014       ; (pg 140)
oGPIODEN:       .equ        0x51C       ; (pg 682)
oGPIODIR:       .equ        0x400       ; (pg 663)
oGPIODATA:      .equ        0x020       ; HOPEFULLY THIS WORKS

main:
        .asmfunc

        AND     R0, #0x0
        AND     R1, #0x0

    ; ENABLE CLOCK TO PORT F
        LDR     R0, SYSCTL_BASE
        MOV     R1, #0x20
        STR     R1, [R0, #oRCGCGPIO]

        NOP
        NOP
        NOP

        AND R1, #0x0

    ; SET DIRECTION
        LDR     R0, PORTF_BASE
        MOV     R1, #0x08
        STR     R1, [R0, #oGPIODIR]

    ; SET DIGITAL ENABLE
        STR     R1, [R0, #oGPIODEN]

    ; INITIALIZE GREEN LED ON
        STR     R1, [R0, #oGPIODATA]

    ; INITIALIZE SYSTICK
init:
    ;;; PROGRAM THE VALUE INTO STRELOAD
        LDR     R0, SYSTICK_BASE
        MOVW    R1, #0xFFFF
        MOVT    R1, #0x00FF
        STR     R1, [R0, #oSTRELOAD]

    ;;; CLEAR STCURRENT BY WRITING ANY VALUE TO IT
        STR     R1, [R0, #oSTCURRENT]

        AND     R1, #0x0

    ;;; CONGIFURE STCTRL
        MOV     R1, #0x05
        STR     R1, [R0, #oSTCTRL]

loop:
        LDR     R1, [R0, #oSTCTRL]
        MOVW    R2, #0xFFFF
        MOVT    R2, #0xFFFE
        BIC     R1, R2
        CMP     R1, #0x0
        BEQ     loop
        LDR     R0, PORTF_BASE
        LDR     R1, [R0, #oGPIODATA]
        EOR     R1, #0x08
        STR     R1, [R0, #oGPIODATA]

        LDR     R0, SYSTICK_BASE
        LDR     R1, [R0, #oSTCTRL]
        MOVW    R2, #0x0000
        MOVT    R2, #0x0001
        BIC     R1, R2
        STR     R1, [R0, #oSTCTRL]
        B       init

        .endasmfunc
        .end
