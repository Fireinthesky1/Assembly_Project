// HOMEWORK 2
// PROBLEM 2
// James Hicks

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

const uint8_t LED_RED            = 0x02; // one
const uint8_t LED_PURPLE         = 0x06; // two
const uint8_t LED_BLUE           = 0x04; // three
const uint8_t LED_GREEN          = 0x08; // four
const uint8_t LED_WHITE          = 0x0E; // five

uint8_t current_state = 0x11;
int     current_color = 0;

const uint8_t none = 0x11;
const uint8_t one  = 0x01;
const uint8_t two  = 0x10;
const uint8_t both = 0x00;

int fuck_c_gimme_back_my_mod_operator(int a, int b)
{
   int intermediate = a % b;
   if(intermediate < 0)
   {
      return 5 + intermediate; // <- EVIL VUDU MAGIC
   }
   return intermediate; // HOW?
}

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

   switch(current_state)
   {
      case 0x11:
         if(input == two)
         {
            current_color++;
         }
         else if(input == one)
         {
            current_color--;
         }
         break;
      case 0x10:
         if(input == 0x00)
         {
            current_color--;
         }
         break;
      case 0x01:
         if(input == 0x00)
         {
            current_color++;
         }
         break;
      default:
         break;
   }

   current_color = fuck_c_gimme_back_my_mod_operator(current_color, 5);

   uint8_t display;

   switch(current_color)
   {
      case 0:
         display = LED_RED;
         break;
      case 1:
         display = LED_PURPLE;
         break;
      case 2:
         display = LED_BLUE;
         break;
      case 3:
         display = LED_GREEN;
         break;
      case 4:
         display = LED_WHITE;
         break;
      default:
         display = 0x00;
         break;
   }

   // FIND NEXT STATE
   current_state = input;

   // WRITE
   GPIOPinWrite(GPIO_PORTF_BASE,
                GPIO_PIN_1 | GPIO_PIN_2 |
                GPIO_PIN_3,
                display);

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
                  GPIO_BOTH_EDGES);

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


void main(void)
{

   // INITIALIZE GPIO
   gpio_init();

   // INITIALIZE TIMER
   timer1_init();

   // SET INITIAL STATE FOR LED
   GPIOPinWrite(GPIO_PORTF_BASE,
                GPIO_PIN_1 | GPIO_PIN_2 |
                GPIO_PIN_3,
                LED_RED);

   while(1)
   {
   }

}
