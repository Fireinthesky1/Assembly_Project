// James Hicks Lab 5, October 5 2023

// portD pins 0 - 3 determine the value to display
// portE pins 1 - 3 detmine which digits will dispay a value
// The display module will show the current value of a count starting at 0
// without unnecessary leading zeros, maintained in a uint8_t variable
// Use a general purpose timer with an interrupt to switch between digits
// to produce a steady display
// separate display code into its own .c/.h files

// TODO(JAMES): THIS CODE IS UNFINISHED

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

//GPIO======================================================================

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

    // ENABLE MOSC FOR SYSTEM CLOCK
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MZ);

    // ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
    IntMasterEnable();

    // DISABLE PRIORITY MASKING
    IntPriorityMaskSet(0x0);

//TIMER1->Display Timer=====================================================

    // ENABLE THE TIMER1 PERIPHERAL
    SysCtlPeripheralEnable(STSCTL_PERIPH_TIMER1);

    // WAIT FOR TIMER1 MODULE TO BE READY
    while(!SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1))
    {
    }

    // SET CLOCK SOURCE FOR TIMER 1
    TimerClockSourceSet(TIMER1_BASE,
                        TIMER_CLOCK_SYSTEM);

    // CONFIGURE TIMER1
    TimerConfigure(TIMER1_BASE,
                   TIMER_CFG_ONE_SHOT);

    // SET THE COUNT TIME FOR TIMER1 (1 second) <- ASK PROF ABOUT THIS
    TimerLoadSet(TIMER1_BASE,
                 TIMER_A,
                 16000000);

    // CONGIFURE TIMER1 TO COUNT RISING EDGES
    TimerControlEvents(TIMER1_BASE,
                       TIMER_A,
                       TIMER_EVENT_POS_EDGE);

    // ENABLE THE INTERUPT FOR TIMER1
    TimerIntEnable(TIMER1_BASE,
                   TIMER_TIMA_TIMEOUT);

    // SET THE PRIORITY OF TIMER 1A (HIGHEST PRIORITY)
    IntPrioritySet(INT_TIMER1A, 0x00);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    TimerIntRegister(TIMER1_BASE,
                     TIMERA,
                     display_interrupt_handler);

// TIMER2->Increment Timer==================================================

    // ENABLE THE TIMER2 PERIPHERAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    // WAIT FOR TIMER2 MODULE TO BE READY
    while(!SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2))
    {
    }

    // SET THE CLOCK SOURCE FOR TIMER 2
    TimerClockSourceSet(TIMER2_BASE,
                        TIMER_CLOCK_SYSTEM);

    // CONFIGURE TIMER2
    TimerConfigure(TIMER1_BASE);

    // SET THE COUNT TIME FOR TIMER2

    // CONFIGURE TIMER 2 TO COUNT RISING EDGES
    TimerControlEvent(TIMER2_BASE,
                      TIMER_A,
                      TIMER_EVENT_POS_EDGE);
    return 0;
}

int main()
{
}
