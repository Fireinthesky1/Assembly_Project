// James Hicks Lab 5, October 5 2023

// use a general purpose timer with an interrupt to make
// the count change every second

// At startup the count wil be increasing by 1 each second

// Pressing sw1 will pause the count
// pressing sw2 will reset the count to zero

// Use SysTick with an interrupt to handle all switch debouncing needs

// Debounce has highest priority
// all others have lower and equal priority

// triggering a switch interrupt immediately triggers a systick interrupt

// TODO(James): ASK PROF WHY "this assembly directive potentially unsafe inside a function"

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

#include "display.h"

extern void display_interrupt_handler(void);

uint8_t number_to_display = 0;
uint8_t digit_to_display = 1;
bool incrementing = true;


void increment_interrupt_handler(void)
{
  number_to_display++;
  // CLEAR THE TRIGGER FLAG
  TimerIntClear(TIMER2_BASE,
                TIMER_TIMA_TIMEOUT);
}

void debounce(void)
{

  // load R0 with 0xE000E010 (STCTRL)
  // load R2 with FFFEFFFF
  // This is so we can use R2 to clear all
  // but 16th bit (count flag) and compare that with 0
  __asm("STCTRL:    .field    0xE000E010    \n"
        "GRABBIT16  .field    0xFFFEFFFF    \n"
        "           LDR       R0, STCTRL    \n"
        "           LDR       R2, GRABBIT16   ");

  // START THE COUNTER
  SysTickEnable();

  // constantly check the count flag to see
  // if systick finished
  __asm("wait:                              \n"
        "           LDR       R1, [R0]      \n"
        "           BIC       R1, R2        \n"
        "           CMP       R1, #0        \n"
        "           BNE       wait            ");
  // Disable the timer
  SysTickDisable();

}

void sw1_interrupt_handler(void)
{
  debounce();
  if(incrementing)
  {
    // disable increment interrupt
    TimerIntDisable(TIMER1_BASE,
                    TIMER_TIMA_TIMEOUT);
    incrementing = false;
  }
  else
  {
    // enable increment interrupt
    TimerIntEnable(TIMER1_BASE,
                  TIMER_TIMA_TIMEOUT);
    incrementing = true;
  }

  GPIOIntClear(GPIO_PORTF_BASE,
               GPIO_PIN_4);
}

void sw2_interrupt_handler(void)
{
  debounce();
  number_to_display = 0;
  GPIOIntClear(GPIO_PORTF_BASE,
               GPIO_PIN_0);
}

int timer_init(void)
{

//SYSTICK->Debounce Timer===================================================

  // 16 MHz, we want to debounce for
  // 10 milliseconds
  SysTickDisable();
  SysTickPeriodSet(0x27100);

//TIMER1->Display Timer=====================================================

  // ENABLE THE TIMER1 PERIPHERAL
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

  // WAIT FOR TIMER1 MODULE TO BE READY
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
  {
  }

  // SET CLOCK SOURCE FOR TIMER 1
  TimerClockSourceSet(TIMER1_BASE,
                      TIMER_CLOCK_SYSTEM);

  // CONFIGURE TIMER1
  // HALF WIDTH (16 bits)
  // PERIODIC (repeating)
  TimerConfigure(TIMER1_BASE,
                 TIMER_CFG_A_PERIODIC);

  // SET THE COUNT TIME FOR TIMER1 (1 second ~ 160,000,000 - 1 cycles)
  TimerLoadSet(TIMER1_BASE,
               TIMER_A,
               0xF423FF);

  // CONGIFURE TIMER1 TO COUNT RISING EDGES
  TimerControlEvent(TIMER1_BASE,
                     TIMER_A,
                     TIMER_EVENT_POS_EDGE);

  // ENABLE THE INTERUPT FOR TIMER1
  TimerIntEnable(TIMER1_BASE,
                 TIMER_TIMA_TIMEOUT);

  // SET THE PRIORITY OF TIMER 1A (HIGHEST PRIORITY)
  IntPrioritySet(INT_TIMER1A, 0x00);

  // REGISTER THE INTERRUPT FOR THE DISPLAY
  TimerIntRegister(TIMER1_BASE,
                   TIMER_A,
                   display_interrupt_handler);

// TIMER2->Increment Timer==================================================

  // ENABLE THE TIMER2 PERIPHERAL
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

  // WAIT FOR TIMER2 MODULE TO BE READY
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
  {
  }

  // SET THE CLOCK SOURCE FOR TIMER 2
  TimerClockSourceSet(TIMER2_BASE,
                      TIMER_CLOCK_SYSTEM);

  // CONFIGURE TIMER2
  // HALF WIDTH
  // PERIODIC
  TimerConfigure(TIMER2_BASE,
                 TIMER_CFG_A_PERIODIC);

  // SET THE COUNT TIME FOR TIMER2
  TimerLoadSet(TIMER2_BASE,
               TIMER_A,
               0xF423FF);

  // CONFIGURE TIMER 2 TO COUNT RISING EDGES
  TimerControlEvent(TIMER2_BASE,
                    TIMER_A,
                    TIMER_EVENT_POS_EDGE);

  TimerIntEnable(TIMER2_BASE,
                 TIMER_TIMA_TIMEOUT);

  IntPrioritySet(INT_TIMER2A_TM4C123, 0x00);

  TimerIntRegister(TIMER2_BASE,
                   TIMER_A,
                   increment_interrupt_handler);

  return 0;
}


int gpio_init(void)
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
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
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
  // ASK PROF ABOUT PAGE 494 data sheet <=========================================
  // ARE WE USING MAIN OSCILLATOR OR EXTERNAL CRYSTAL?
  // WHAT'S THE DIFFERENCE?
  SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_16MHZ);

  // ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
  IntMasterEnable();

  // DISABLE PRIORITY MASKING
  IntPriorityMaskSet(0x0);

  // ENABLE INTERRUPTS FOR PF0, PF4
  GPIOIntEnable(GPIO_PORTF_BASE,
                GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

  // REGISTER INTERRUPT HANDLER FOR PF4 (sw1)
  GPIOIntRegisterPin(GPIO_PORTF_BASE,
                     GPIO_PIN_4,
                     sw1_interrupt_handler);

  // REGISTER INTERRUPT HANDLER FOR PF0 (sw2)
  GPIOIntRegisterPin(GPIO_PORTF_BASE,
                     GPIO_PIN_0,
                     sw2_interrupt_handler);

  // ASK PROF IF WE NEED TO CALL GPIOIntTypeSet() here <==========================

  return 0;
}

int main()
{
  // INITIALIZE GPIO
  gpio_init();

  // INITIALIZE TIMER 1 and TIMER 2
  timer_init();

  // ENABLE TIMERS
  TimerEnable(TIMER1_BASE,
              TIMER_A);
  TimerEnable(TIMER2_BASE,
              TIMER_A);

  // LOOP FOREVER
  while(1)
  {
  }

}
