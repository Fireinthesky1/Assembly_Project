;;; James Hicks Coffee Grinder Project

        .thumb
        .global main

SYSCTRL_RCGCGPIO_R:     .field      0x400FE608  ; (pg 340)
SYSCTRL_RCC_R           .field      0x400FE060  ; (pg 254)

GPIO_PORTB_DIR_R:       .field      0x40005400  ; (pg 663)
GPIO_PORTB_DR4R_R:      .field      0x40005504  ; (pg 674)
GPIO_PORTB_DEN_R:       .field      0x4000551C  ; (pg 682)

main:

        .asmfunc

        AND     R0, #0
        AND     R1, #0

    ; ENABLE CLOCK TO PORT B
        
