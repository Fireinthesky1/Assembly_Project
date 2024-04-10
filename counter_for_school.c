// Displays and incrementing count
// Pressing sw1 will pause the count
// pressing sw2 will reset the count to zero

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
  if(incrementing)
  {
      number_to_display++;
  }
  // CLEAR THE TRIGGER FLAG
  TimerIntClear(TIMER2_BASE,
                TIMER_TIMA_TIMEOUT);
}

void systick_interrupt_handler(void)
{
  // DISABLE SYSTICK INTERRUPTS
  SysTickIntDisable();

  // DISABLE THE TIMER
  SysTickDisable();

  // DEAL WITH STATES
  uint32_t current_state = GPIOPinRead(GPIO_PORTF_BASE,
                                       GPIO_PIN_0 | GPIO_PIN_4);

  if(current_state == 0x10)//sw2
  {
    if(incrementing)
    {
      incrementing = false;
    }
    else
    {
      incrementing = true;
    }
  }
  else if (current_state == 0x01)//sw1
  {
    number_to_display = 0;
  }

  // ENABLE GPIO INTERRUPT
  GPIOIntEnable(GPIO_PORTF_BASE,
                GPIO_PIN_0 | GPIO_PIN_4);
}

void portf_interrupt_handler(void)
{

  // CLEAR GPIO INTERRUPTS
  GPIOIntClear(GPIO_PORTF_BASE,
               GPIO_PIN_4 | GPIO_PIN_0);

  // DISABLE GPIO INTERRUPTS
  GPIOIntDisable(GPIO_PORTF_BASE,
                 GPIO_PIN_0 | GPIO_PIN_4);

  // DISABLE THE TIMER
  SysTickDisable();

  // LOAD THE TIMER
  SysTickPeriodSet(0x27100);

  // ENABLE SYSTICK INTERRUPTS
  SysTickIntEnable();

  // START SYSTICK
  SysTickEnable();
}


// debounce timer
int systick_init(void)
{
  // 16 MHz, we want to debounce for
  // 10 milliseconds
  SysTickDisable();
  SysTickPeriodSet(0x27100);
  SysTickIntRegister(systick_interrupt_handler);
  return 0;
}

// display timer
int timer1_init(void)
{
  // ENABLE THE TIMER1 PERIPHERAL
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

  // WAIT FOR TIMER1 MODULE TO BE READY
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
  {
  }

  // SET CLOCK SOURCE FOR TIMER 1
  TimerClockSourceSet(TIMER1_BASE,
                      TIMER_CLOCK_PIOSC);

  // DISABLE THE TIMER FOR INITIALIZATION
  TimerDisable(TIMER1_BASE,
               TIMER_BOTH);

  // full width periodic
  TimerConfigure(TIMER1_BASE,
                 TIMER_CFG_PERIODIC);

  // ONLY TIMER_A (pg 554)
  // 16 Mhz timer 16,000,000 cycles (1 second)
  TimerLoadSet(TIMER1_BASE,
               TIMER_A,
               0x15B38);

  // ENABLE THE INTERUPT FOR TIMER1
  TimerIntEnable(TIMER1_BASE,
                 TIMER_TIMA_TIMEOUT);

  // REGISTER THE INTERRUPT FOR THE DISPLAY
  TimerIntRegister(TIMER1_BASE,
                   TIMER_A,
                   display_interrupt_handler);

  return 0;
}

// increment timer
int timer2_init(void)
{

  // ENABLE THE TIMER2 PERIPHERAL
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

  // WAIT FOR TIMER2 MODULE TO BE READY
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
  {
  }

  // SET THE CLOCK SOURCE FOR TIMER 2
  TimerClockSourceSet(TIMER2_BASE,
                      TIMER_CLOCK_PIOSC);

  // disable timer for initialization
  TimerDisable(TIMER2_BASE,
               TIMER_BOTH);

  // FULL WIDTH PERIODIC
  TimerConfigure(TIMER2_BASE,
                 TIMER_CFG_PERIODIC);

  // SET THE COUNT TIME FOR TIMER2
  TimerLoadSet(TIMER2_BASE,
               TIMER_A,
               0xF42400);

  TimerIntEnable(TIMER2_BASE,
                 TIMER_TIMA_TIMEOUT);

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

  // USE THE 16Mhz PIOSC
  SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

  // ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
  IntMasterEnable();

  // DISABLE PRIORITY MASKING
  IntPriorityMaskSet(0x0);

  // ENABLE INTERRUPTS FOR PF0, PF4
  GPIOIntEnable(GPIO_PORTF_BASE,
                GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

  GPIOIntRegister(GPIO_PORTF_BASE,
                  portf_interrupt_handler);

  return 0;
}

int main()
{
  // INITIALIZE GPIO
  gpio_init();

  // INITIALIZE SYSTICK
  systick_init();

  // INITIALIZE TIMER 1 and TIMER 2
  timer1_init();
  timer2_init();

  // ENABLE TIMERS
  TimerEnable(TIMER1_BASE,
              TIMER_BOTH);
  TimerEnable(TIMER2_BASE,
              TIMER_BOTH);

  // LOOP FOREVER
  while(1)
  {
  }

}
