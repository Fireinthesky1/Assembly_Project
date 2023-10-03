; James Hicks Lab 3 October 2

        .thumb
        .global main

GPIO_UNLOCK_CODE:       .field      0x4C4F434B
SYSCTRL_RCGCGPIO_R:     .field      0x400FE608

GPIO_PORTB_LOCK_R:      .field      0x40005520
GPIO_PORTB_CR:          .field      0x40005524
GPIO_PORTB_DIR_R:       .field      0x40005400
GPIO_PORTB_DR4R_R:      .field      0x40005504
GPIO_PORTB_DEN_R:       .field      0x4000551C

GPIO_PORTF_LOCK_R:      .field      0x40025520
GPIO_PORTF_CR:          .field      0x40025524
GPIO_PORTF_PUR_R:       .field      0x40025510
GPIO_PORTF_DEN_R:       .field      0x4002551C

oDATA_READ_SW1:         .field      0x40025040
oDATA_READ_SW2:         .field      0x40025004
oDATA_WRITE:            .field      0x4000503C

main:
        .asmfunc

    ; ENABLE CLOCK TO PORT B,F (bit 1, 5)
        LDR     R0, SYSCTRL_RCGCGPIO_R
        LDR     R1, [R1]
        ORR     R1, #0x22
        STR     R1, [R0]

    ; UNLOCK GPIO PORT B,F
        LDR     R0, GPIO_PORTB_LOCK_R
        LDR     R1, GPIO_UNLOCK_CODE
        STR     R1, [R0]
        LDR     R0, GPIO_PORTF_LOCK_R
        STR     R1, [R0]

    ; ENABLE COMMIT FOR PORT B, F
        LDR     R0, GPIO_PORTB_CR
        LDR     R1, [R0]
        ORR     R1, #0x0F
        STR     R1, [R0]
        LDR     R0, GPIO_PORTF_CR
        LDR     R1, [R0]
        ORR     R1, #0x11
        STR     R1, [R0]

    ; SET DIRECTION (PB0-4 outputs)
        LDR     R0, GPIO_PORTB_DIR_R
        LDR     R1, [R0]
        ORR     R1, #0x0F
        STR     R1, [R0]

    ; SET DRIVE STRENGTH FOR PORTB to 4-mA
        LDR     R0, GPIO_PORTB_DR4R_R
        LDR     R1, [R0]
        ORR     R1, #0x0F
        STR     R1, [R0]

    ; ENABLE PULL UP RESISTOR FOR PF0, PF4
        LDR     R0, GPIO_PORTF_PUR_R
        LDR     R1, [R0]
        ORR     R1, #0x11
        STR     R1, [R0]

    ; SET DIGITAL ENABLE (PB0-4, PF0, PF4)
        LDR     R0, GPIO_PORTB_DEN_R
        LDR     R1, [R0]
        ORR     R1, #0x0F
        STR     R1, [R0]
        LDR     R0, GPIO_PORTF_DEN_R
        LDR     R1, [R0]
        ORR     R1, #0x11
        STR     R1, [R0]

    ; CLEAR REGISTERS
        AND     R0, #0
        AND     R1, #0

    ; INITIALIZE NUMBER TO DISPLAY
        MOVT    R0, #0x0000
        MOVB    R0, #0x0000

DISPLAY:
        LDR     R1, oDATA_WRITE
        MOVB    R2, R0
        STR     R2, [R1]
        AND     R1, #0
        AND     R2, #0
        B POLL_SWITCHES

POLL_SWITCHES:
        LDR     R1, oDATA_READ_SW1
        LDR     R2, [R1]
        CMP     R2, #0
        BEQ     COMPLEMENT
        LDR     R1, oDATA_READ_SW2
        LDR     R2, [R1]
        CMP     R2, #0
        BEQ     SHIFT_LEFT
        B       DISPLAY

COMPLEMENT:
        MOVT    R1, #0xFFFF
        MOVB    R1, #0xFFFF
        EOR     R0, R1
        B DISPLAY

SHIFT_LEFT:
        LSL     R0, R0, #1
        ORR     R0, #0x1
        B DISPLAY

        .endasmfunc
        .align

wait:   .asmfunc

waitLoop:
        BEQ waitLoop
        BX LR

        .endasmfunc
        .end