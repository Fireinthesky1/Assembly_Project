; James Hicks Sept 27 2023

; 1) Increment the pointer
; 2) Store the number
; 3) Increment the number
; 4) Increment the loop index

; R0 holds the loop index
; R1 holds the memory address of the array
; R2 holds the values
; R3 holds the sum
; I want the chunks of memory to be 1 byte each

        .thumb
        .global main
a_p: .word 0x200004F0 ; array pointer
cnt: .equ 10          ; size of the array in bytes
main:
        .asmfunc
    ; here we initialize the registers
        AND         R0, #0            ; clear R0 (loop index)
        AND         R1, #0            ; clear R1 (array pointer)
        AND         R2, #0            ; clear R2 (holds values to be stored)
        AND         R3, #0            ; clear R3 (holds the sum)

        LDR         R1, a_p           ; store the array pointer into R1

        MOV         R2, #10           ; move 10 into the low bits of R2

    ; this loop initializes the array
loop_i:
        STRB        R2, [R1], #1      ; *R1=R2, R1=R1+1 (1 byte further down)
        ADD         R2, #3            ; R2=R2+3
        ADD         R0, #1            ; increment R0 (the loop index)
        CMP         R0, #cnt
        BNE         loop_i

    ; here we reset the register for the sum loop
        LDR         R1, a_p           ; store the array pointer into R1 again
        AND         R0, #0            ; clear loop index
        AND         R2, #0            ; clear R2

    ; this loop calculates the sum
loop_s:
        LDRB        R2, [R1]          ; load the value pointed to by R1 into R2
        ADD         R3, R2            ; add R2 to R3
        ADD         R1, #1            ; R1 = R1 + 1
        ADD         R0, #1            ; increment loop index
        CMP         R0, #cnt
        BNE         loop_s
    ; R3 now holds 0x000000EB
loop:	mov			R0, R0
		B			loop
        .endasmfunc
        .end
