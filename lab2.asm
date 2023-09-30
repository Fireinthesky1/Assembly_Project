; James Hicks Sept 29 2023
; STEPS
; Enable clock to port F by setting bit 5 in RCGCGPIO register
; Set direction of port by programming GPIODIR register (1 out, 0 in)
; Enable GPIO pins as digital IO by setting appropriate DEN pin in GPIODEN
; write a 1 to pin 3 turning on the green LED

; load R0 with the base address for RCGCGPIO
; set 5th bit of RCGCGPIO
; load R0 with the APB base for GPIO for port F 0x40025000
; set 3rd bit of GPIODIR (1 out)
; set 3rd bit of GPIODEN (1 Digital functions enabled)

        .thumb
        .global main
SYSCTL_RCGCGPIO_R: .field 0x400FE608   ; RCGCGPIO_R (340 data sheet)
GPIO_PORTF_DATA_R: .field 0x40025020   ; Base is 0x40025000 offset is 0x020
GPIO_PORTF_DIR_R: .field 0x40025400    ; DIR_R GPIO Port F APB (153 book)
GPIO_PORTF_DEN_R: .field 0x4002551C    ; DEN_R GPIO Port F APB (153 book)

main:
        .asmfunc

    ; CLEAR REGISTERS
        AND R0, #0                ; clear R0 (addresses)
        AND R1, #0                ; clear R1 (bits to store)

    ; ENABLE CLOCK TO PORT F (340 data sheet)
        LDR R0, SYSCTL_RCGCGPIO_R ; load RCGCGPIO_R into R0
        LDR R1, [R0]              ; load what's at RCGCGPIO_R into R1
        ORR R1, #0x20             ; set the 5th bit high
        STR R1, [R0]              ; stores modded bits into RCGCGPIO_R
        NOP                       ; time for the clock to finish
        NOP

    ; CLEAR REGISTERS
        AND R0, #0                ; clear R0
        AND R1, #0                ; clear R1

    ; SET DIRECTION (663 data sheet)
        LDR R0, GPIO_PORTF_DIR_R  ; load DIR_R address into R0
        LDR R1, [R0]              ; loads what's at DIR_R into R1
        ORR R1, #0x08             ; set the 3rd bit high
        STR R1, [R0]              ; stores modified DIR_R bits into DIR_R

    ; CLEAR REGISTERS
        AND R0, #0                ; clear R0
        AND R1, #0                ; clear R1

    ; SET DIGITAL ENABLE (682 data sheet)
        LDR R0, GPIO_PORTF_DEN_R  ; load DEN_R address into R0
        LDR R1, [R0]              ; load what's at DEN_r into R1
        ORR R1, #0x08             ; sets the 3rd bit high
        STR R1, [R0]              ; stores modified DEN_R bits into DEN_R

    ; CLEAR REGISTERS
        AND R0, #0                ; clear R0
        AND R1, #0                ; clear R1

    ; SET DATA (662 data sheet)
        LDR R0, GPIO_PORTF_DATA_R
        MOV R1, #0x08
        STR R1, [R0]

loop:   MOV R0, R0
        B loop
        .endasmfunc
        .end