// HOMEWORK 2
// PROBLEM 2
// James Hicks

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

const uint8_t LED_RED            = 0x02; // one
const uint8_t LED_RED_BLUE       = 0x06; // two
const uint8_t LED_BLUE           = 0x04; // three
const uint8_t LED_GREEN          = 0x08; // four
const uint8_t LED_RED_GREEN_BLUE = 0x0E; // five

uint8_t current_state = 0x11;
uint8_t current_color = 0x02;

uint8_t none = 0x00000011;
uint8_t one  = 0x00000001;
uint8_t two  = 0x00000010;
uint8_t both = 0x00000000;

// 1) disable timer interrupts
// 2) clear timer interrupt flag
// 3) handle state transitions
// 4) enable gpio interrupts
void timer1_interrupt_handler(void)
{

   // CLEAR THE TIMER INT FLAG
   TimerIntClear(TIMER1_BASE,
                 TIMER_TIMA_TIMEOUT);

   // DISABLE TIMER INTERRUPTS
   TimerIntDisable(TIMER1_BASE,
                   TIMER_BOTH);

   // DISABLE THE TIMER
   TimerDisable(TIMER1_BASE,
                TIMER_BOTH);

   uint8_t input = GPIOPinRead(GPIO_PORTF_BASE,
                               GPIO_PIN_0 | GPIO_PIN_4);

   // STATE TRANSITIONS
   switch(current_color)
   {

      case LED_RED:
         if(input == 0x01)             // sw 1 (back)
         {
           current_color = LED_RED_GREEN_BLUE;
         }
         else if(input == 0x10)        // sw2 (forward)
         {
            current_color = LED_RED_BLUE;
         }
         else if(input == 0x00)
         {
            if(current_state == 0x01) // sw 2 (forward)
            {
               current_color = LED_RED_BLUE;
            }
            else if(input == 0x10)    // sw1 (backward)
            {
               current_color = LED_RED_GREEN_BLUE;
            }
         }
         break;

      case LED_RED_BLUE:
         if(input == 0x01)             // sw 1 (back)
         {
            current_color = LED_RED;
         }
         else if(input == 0x10)        // sw2 (forward)
         {
            current_color = LED_BLUE;
         }
         else if(input == 0x00)
         {
            if(current_state == 0x01) // sw 2 (forward)
            {
               current_color = LED_BLUE;
            }
            else if(input == 0x10)    // sw1 (backward)
            {
               current_color = LED_RED;
            }
         }
         break;

      case LED_BLUE:
         if(input == 0x01)             // sw 1 (back)
         {
            current_color = LED_RED_BLUE;
         }
         else if(input == 0x10)        // sw2 (forward)
         {
            current_color = LED_GREEN;
         }
         else if(input == 0x00)
         {
            if(current_state == 0x01) // sw 2 (forward)
            {
               current_color = LED_GREEN;
            }
            else if(input == 0x10)    // sw1 (backward)
            {
               current_color = LED_RED_BLUE;
            }
         }
         break;

      case LED_GREEN:
         if(input == 0x01)             // sw 1 (back)
         {
            current_color = LED_BLUE;
         }
         else if(input == 0x10)        // sw2 (forward)
         {
            current_color = LED_RED_GREEN_BLUE;
         }
         else if(input == 0x00)
         {
            if(current_state == 0x01) // sw 2 (forward)
            {
               current_color = LED_RED_GREEN_BLUE;
            }
            else if(input == 0x10)    // sw1 (backward)
            {
               current_color = LED_BLUE;
            }
         }
         break;

      case LED_RED_GREEN_BLUE:
         if(input == 0x01)             // sw 1 (back)
         {
            current_color = LED_GREEN;
         }
         else if(input == 0x10)        // sw2 (forward)
         {
            current_color = LED_RED;
         }
         else if(input == 0x00)
         {
            if(current_state == 0x01) // sw 2 (forward)
            {
               current_color = LED_RED;
            }
            else if(input == 0x10)    // sw1 (backward)
            {
               current_color = LED_GREEN;
            }
         }

         break;

      default:                           // bad if we get here
         break;
   }

   current_state = input;

   // WRITE THE CURRENT STATE
   GPIOPinWrite(GPIO_PORTF_BASE,
                GPIO_PIN_1 | GPIO_PIN_2 |
                GPIO_PIN_3,
                current_color);

   // ENABLE GPIO INTERRUPTS
   GPIOIntEnable(GPIO_PORTF_BASE,
                 GPIO_PIN_0 | GPIO_PIN_4);

}

// 1) disable portf interrupts
// 2) clear portf interrupt flag
// 3) enable timer interrupts
// 3) start the timer
void portf_interrupt_handler(void)
{

   // CLEAR GPIO INTERRUPTS
   GPIOIntClear(GPIO_PORTF_BASE,
                GPIO_PIN_0 | GPIO_PIN_4);

   // DISABLE GPIO INTERRUPTS
   GPIOIntDisable(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4);

   // DISABLE THE TIMER
   TimerDisable(TIMER1_BASE,
                TIMER_BOTH);

   // LOAD THE TIMER (.010 seconds)
   TimerLoadSet(TIMER1_BASE,
                TIMER_A,
                0x27100); // 27100 = .01 sec

   // ENABLE TIMER1 INTERRUPTS
   TimerIntEnable(TIMER1_BASE,
                  TIMER_TIMA_TIMEOUT);

   // START TIMER1
   TimerEnable(TIMER1_BASE,
               TIMER_BOTH);
}

void gpio_init(void)
{

   // ENABLE CLOCK TO PORTF
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
   {
   }

   // SET DIRECTIONS
   GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                         GPIO_PIN_1 | GPIO_PIN_2 |
                         GPIO_PIN_3);

   GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,
                        GPIO_PIN_0 | GPIO_PIN_4);

   // UNLOCK PF0
   GPIOUnlockPin(GPIO_PORTF_BASE,
                 GPIO_PIN_0 | GPIO_PIN_4);

   // ENABLE PUR FOR SWITCHES
   GPIOPadConfigSet(GPIO_PORTF_BASE,
                    GPIO_PIN_0 | GPIO_PIN_4,
                    GPIO_STRENGTH_2MA,
                    GPIO_PIN_TYPE_STD_WPU);

   // SET DRIVE STRENGTH FOR LEDS
   GPIOPadConfigSet(GPIO_PORTF_BASE,
                    GPIO_PIN_1 | GPIO_PIN_2 |
                    GPIO_PIN_3,
                    GPIO_STRENGTH_2MA,
                    GPIO_PIN_TYPE_STD);

   // USE THE 16Mhz PIOSC
   SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

   // GLOBAL ENABLE INTERRUPTS
   IntMasterEnable();

   // DISABLE PRIORITY MASKING
   IntPriorityMaskSet(0x0);

   // ONLY TRIGGER ON FALLING EDGE
   GPIOIntTypeSet(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4,
                  GPIO_FALLING_EDGE);

   // ENABLE INTERRUPTS FOR PF0 and PF4
   GPIOIntEnable(GPIO_PORTF_BASE,
                 GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

   // REGISTER THE HANDLER
   GPIOIntRegister(GPIO_PORTF_BASE,
                   portf_interrupt_handler);

}

void timer1_init(void)
{

   // ENABLE THE TIMER1 PERIPHERAL
   SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

   // WAIT FOR TIMER1 MODULE TO BE READY
   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
   {
   }

   // SET THE CLOCK SOURCE FOR TIMER1
   TimerClockSourceSet(TIMER1_BASE,
                       TIMER_CLOCK_PIOSC);

   // DISABLE THE TIMER FOR INITIALIZATION
   TimerDisable(TIMER1_BASE,
                TIMER_BOTH);

   // FULL WIDTH PERIODIC
   TimerConfigure(TIMER1_BASE,
                  TIMER_CFG_PERIODIC);

   // 16 Mhz 16,000,000 cycles per second ; .010 seconds
   TimerLoadSet(TIMER1_BASE,
                TIMER_A,
                0x27100);

   // INTERRUPT TRIGGERS ON TIMEOUT
   TimerIntEnable(TIMER1_BASE,
                  TIMER_TIMA_TIMEOUT);

   // REGISTER THE HANDLER
   TimerIntRegister(TIMER1_BASE,
                    TIMER_A,
                    timer1_interrupt_handler);

}


int main(void)
{

   // INITIALIZE GPIO
   gpio_init();

   // INITIALIZE TIMER
   timer1_init();

   // SET INITIAL STATE FOR LED
   GPIOPinWrite(GPIO_PORTF_BASE,
                GPIO_PIN_1 | GPIO_PIN_2 |
                GPIO_PIN_3,
                current_color);

   while(1)
   {
   }

   return 0;

}
