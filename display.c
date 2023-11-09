// DISPLAY C FILE
#pragma once

#include "display.h"
#include <driverlib/timer.h>

volatile uint8_t number_to_display=0;
volatile uint8_t digit_to_display=0;
volatile bool display_ready;

uint8_t control_converter(void)
{
  double base = 2;
  double power = (double)digit_to_display;
  return (uint8_t)pow(base, power);
}

uint8_t display_converter(void)
{

  uint8_t result = number_to_display;
  switch(digit_to_display)
  {
  case 1:
    display_ready = true;
    return result % 10;
  case 2:
    result /= 10;
    return result % 10;
  case 3:
    result /= 100;
    return result % 10;
  default:  // VERY BAD IF WE END UP HERE
    return 0;
  }

}

void display_interrupt_handler(void)
{
  if(number_to_display <= 99 && digit_to_display == 3)
  {
      digit_to_display %= 3;
      digit_to_display++;
  }
  else if(number_to_display <= 9 && digit_to_display == 2)
  {
      digit_to_display %= 3;
      digit_to_display += 2;
      digit_to_display %= 3;
  }


  // GET THE CORRECT CONTROL VALUE
  uint8_t control = control_converter();

  // WRITE TO PORT E (control)
  GPIOPinWrite(GPIO_PORTE_BASE,
               GPIO_PIN_1 | GPIO_PIN_2 |
               GPIO_PIN_3,
               control);

  // GET THE CORRECT DISPLAY VALUE
  uint8_t display = display_converter();

  // WRITE TO PORT D (display)
  GPIOPinWrite(GPIO_PORTD_BASE,
               GPIO_PIN_0 | GPIO_PIN_1 |
               GPIO_PIN_2 | GPIO_PIN_3,
               display);

  // INCREMENT CONTROL
  digit_to_display %= 3;
  digit_to_display++;

  // CLEAR THE TRIGGER FLAG
  TimerIntClear(TIMER1_BASE,
                TIMER_TIMA_TIMEOUT);

}
