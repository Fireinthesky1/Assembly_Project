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

; NEED TO FIGURE OUT HOW TO READ FROM GPIO REGISTERS TO DEBUG

        .thumb
        .global main
RCGCGPIO:  .field 0x400FE000    ; Base address for RCGCGPIO
GPIO:      .field 0x40025000    ; Base address for GPIO Port F APB
oRCGCGPIO: .equ 0x608           ; offset for RCGCGPIO
oGPIODIR:  .equ 0x400           ; offset for GPIODIR
oGPIODEN:  .equ 0x51C           ; offset for GPIODEN

main:
        .asmfunc

        AND R0, #0              ; clear R0
        AND R1, #0              ; clear R1

        LDR R0, RCGCGPIO        ; base address for RCGCGPIO
        MOV R1, #0x10           ; 5th bit high
        STR R1, [R0, #oRCGCGPIO]; enables the clock to port F
        NOP                     ; time for the clock to finish
        NOP


        AND R1, #0              ; clear R1

        LDR R0, GPIO            ; base address for GPIO
        MOV R1, #0x4            ; 3rd bit of R1 high
        STR R1, [R0, #oGPIODIR] ; sets 3rd pin to output

        STR R1, [R0, #oGPIODEN] ; enables digital IO on 3rd pin [THIS CAUSES A FAULT]

        STR R1, [R0, #0x003]    ; sets 3rd pin high

loop:   MOV R0, R0
        B loop
        .endasmfunc
        .end