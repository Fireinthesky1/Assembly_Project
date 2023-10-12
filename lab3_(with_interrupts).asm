; James Hicks Lab3 10/11/2023
; Here we will implement Lab3 using interrupts

        .thumb
        .global main

GPIO_UNLOCK_CODE:       .field          0x4C4F434B ; (pg 684)
SYSCTRL_RCGCGPIO_R:     .field          0x400FE608 ; (pg 340)
SYSCTRL_RCC_R:          .field          0x400FE060 ; (pg 254)

GPIO_PORTB_DIR_R:       .field          0x40005400 ; (pg 663)
GPIO_PORTB_DR4D_R:      .field          0x40005504 ; (pg 674)
GPIO_PORTB_DEN_R:       .field          0x4000551C ; (pg 682)

GPIO_PORTF_LOCK_R:      .field          0x40025520 ; (pg 684)
GPIO_PORTF_CR:          .field          0x40025524 ; (pg 685)
GPIO_PORTF_PUR_R:       .field          0x40025510 ; (pg 677)
GPIO_PORTF_DEN_R:       .field          0x4002551C ; (pg 682)

oDATA_READ_BOTH:        .field          0x40025044
oDATA_WRITE:            .field          0x4000403C

; FOUR STATES OF STATE MACHINE
NEITHER_PRESSED:        .equ            0x00000011
ONLY_SW1_PRESSED:       .equ            0x00000001
ONLY_SW2_PRESSED:       .equ            0x00000010
BOTH_PRESSED:           .equ            0x00000000

main:
        .asmfunc
; CLEAR REGISTERS
        AND     R0, #0
        AND     R1, #0
        AND     R2, #0
; ENABLE CLOCK TO PORT B,F (bit 1 5)
        LDR     R0, SYSCTRL_RCGCGPIO
        LDR     R1, [R0]
        ORR     R1, 0x022
        STR     R1, [R0]
; CLEAR REGISTERS
        AND     R0, #0
        AND     R1, #0
; UNLOCK GPIO PORT F
        LDR     R0, GPIO_PORTF_LOCK_R
        LDR     R1, GPIO_UNLOCK_CODE
        STR     R1, [R0]
; CLEAR REGISTERS
        AND     R0, #0
        AND     R1, #0
; ENABLE COMMIT TO PORTF
; only for PF0
        LDR     R0, GPIO_PORTF_CR
        LDR     R1, [R0]
        ORR     R1, #0x1
        STR     R1, [R0]
