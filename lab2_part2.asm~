; James Hicks Sept 30 2023

        .thumb
        .global main
SYSCTL_RCGCGPIO_R: .field 0x400FE608 ; RCGCGPIO_R (340 data sheet)
GPIO_PORTF_LOCK_R: .field 0x40025520 ; LOCK_R (684 data sheet)
GPIO_UNLOCK_CODE:  .field 0x4C4F434B ; unlock code for CR (684 datasheet)
GPIO_PORTF_CR_R:   .field 0x40025524 ; COMMIT Register (685 data sheet)
GPIO_PORTF_DIR_R:  .field 0x40025400 ; DIR_R GPIO Port F APB (153 book)
GPIO_PORTF_PUR_R:  .field 0x40025510 ; PUR_R GPIO Port F APB (153 book)
GPIO_PORTF_DEN_R:  .field 0x4002551C ; DEN_R GPIO Port F APB (153 book)
oDATA_READ:        .field 0x40025040 ; base is 0x40025000 offset is 0x040
oDATA_WRITE:       .field 0x40025020 ; base is 0x40025000 offset is 0x020

main:
        .asmfunc

    ; ENABLE CLOCK TO PORT F (340 data sheet)
        LDR R0, SYSCTL_RCGCGPIO_R ; load RCGCGPIO_R into R0
        LDR R1, [R0]              ; load what's at RCGCGPIO_R into R1
        ORR R1, #0x20             ; set the 5th bit high
        STR R1, [R0]              ; stores modded bits into RCGCGPIO_R
        NOP                       ; time for the clock to finish
        NOP

    ; UNLOCK GPIO PORT F
        LDR R0, GPIO_PORTF_LOCK_R ; load LOCK_R address into R0
        LDR R1, GPIO_UNLOCK_CODE  ; unlock code for GPIO_CR (684 datasheet)
        STR R1, [R0]              ; unlock GPIO_CR

    ; ENABLE COMMIT FOR PORT F
        LDR R0, GPIO_PORTF_CR_R   ; load CR_R address into R0
        LDR R1, [R0]              ; load what's in CR_R into R1
        ORR R1, #0x10             ; set the 5th bit high
        STR R1, [R0]              ; enable commit for PF4 (bit 5)

    ; SET DIRECTION (663 data sheet)
        LDR R0, GPIO_PORTF_DIR_R  ; load DIR_R address into R0
        LDR R1, [R0]              ; loads what's at DIR_R into R1
        ORR R1, #0x08             ; set the 3rd bit high
        STR R1, [R0]              ; sets the direction of PF3 to output

    ; ENABLE PULL UP RESISTOR FOR PF4
        LDR R0, GPIO_PORTF_PUR_R  ; load PUR_R address into R0
        LDR R1, [R0]              ; load what's at PUR_R into R1
        ORR R1, #0x10             ; set 5th bit high
        STR R1, [R0]              ; enables pull up resistor for PF4

    ; SET DIGITAL ENABLE (682 data sheet)
        LDR R0, GPIO_PORTF_DEN_R  ; load DEN_R address into R0
        LDR R1, [R0]              ; load what's at DEN_r into R1
        ORR R1, #0x18             ; sets the 3rd and 4th bit high
        STR R1, [R0]              ; stores modified DEN_R bits into DEN_R

        AND R1, #0                ; clear R1

loop:
        LDR R0, oDATA_READ
        LDR R1, [R0]
        CMP R1, #0
        BNE LED_OFF

LED_ON:
        AND R1, #0
        LDR R0, oDATA_WRITE
        MOV R1, #0x08
        STR R1, [R0]
        B loop

LED_OFF:
        LDR R0, oDATA_WRITE
        AND R1, #0
        STR R1, [R0]
        B loop

        .endasmfunc
        .end