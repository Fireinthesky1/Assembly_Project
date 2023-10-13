// James Hicks Lab 5, October 5 2023

// portD pins 0 - 3 determine the value to display
// portE pins 1 - 3 detmine which digits will dispay a value
// The display module will show the current value of a count starting at 0
// without unnecessary leading zeros, maintained in a uint8_t variable
// Use a general purpose timer with an interrupt to switch between digits
// to produce a steady display
// separate display code into its own .c/.h files

// initially, once per second the value of the count will change by 1
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

#include "display.h"

extern void display_interrupt_handler(void);

uint8_t number_to_display = 0;
uint8_t digit_to_display = 1;

int init(void)
{
    // ENABLE CLOCK TO PORT D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }

    // ENABLE CLOCK TO PORT E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
    {
    }

    // ENABLE CLOCK TO PORT F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(STSCTL_PERIPH_GPIOF))
    {
    }

    // SET DIRECTIONS
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,
                          GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE,
                          GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,
                         GPIO_PIN_0 | GPIO_PIN_4);

    // UNLOCK PIN PF0 PF4
    GPIOUnlockPin(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4);

    // ENABLE PUR FOR SWITCHES
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_0 | GPIO_PIN_4,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    // CONFIGURE DISPLAY PORT
    GPIOPadConfigSet(GPIO_PORTD_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 |
                     GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    // CONFIGURE CONTROL PORT
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_1 | GPIO_PIN_2 |
                     GPIO_PIN_3,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    // ENABLE MOST FOR SYSTEM CLOCK
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MZ);

    // ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
    IntMasterEnable();

    // ENABLE THE INTERUPT FOR TIMER1
    IntEnable(INT_TIMER1A);

    // DISABLE PRIORITY MASKING
    IntPriorityMaskSet(0x0);

    // SET THE PRIORITY OF TIMER 1A (HIGHEST PRIORITY)
    IntPrioritySet(INT_TIMER1A, 0x00);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    IntRegister(INT_TIMER1A, display_interrupt_handler);

    return 0;
}

int main()
{
}
