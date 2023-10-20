// HOMEWORK 2
// PROBLEM 2
// James Hicks

#include <stdool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/interrupt.h>

uint8_t LED_RED = 0x2;
uint8_t LED_RED_BLUE = 0x6;
uint8_t LED_BLUE = 0x4;
uint8_t LED_GREEN = 0x8;
uint8_t LED_RED_GREEN_BLUE = 0xE;



// 1) disable timer interrupts
// 2) clear timer interrupt flag
// 3) handle state transitions
// 4) enable gpio interrupts
void timer1_interrupt_handler(void)
{

   // DISABLE TIMER INTERRUPTS
   TimerIntDisable(TIMER1_BASE,
                   TIMER_BOTH);

   // CLEAR THE TIMER INT FLAG
   TimerIntClear(TIMER1_BASE,
                 TIMER_TIMA_TIMEOUT);

   // STATE TRANSITIONS
}

// 1) disable portf interrupts
// 2) clear portf interrupt flag
// 3) enable timer interrupts
// 3) start the timer
void portf_interrupt_handler(void)
{

   // DISABLE GPIO INTERRUPTS
   GPIOIntDisable(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4);

   // CLEAR GPIO INTERRUPTS
   GPIOIntClear(GPIO_PORTF_BASE,
                GPIO_PIN_0 | GPIO_PIN_4);

   // ENABLE TIMER1 INTERRUPTS
   TimerIntEnable(TIMER1_BASE,
                  TIMER_TIMA_TIMEOUT);

   // LOAD THE TIMER (.010 seconds)
   TimerLoadSet(TIMER1_BASE,
                TIMER_A,
                0x27100);

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
                 GPIO_PIN_0);

   // ENABLE PUR FOR SWITCHES
   GPIOPadConfigSet(GPIO_PORTF_BASE,
                    GPIO_PIN_0 | GPIO_PIN_4,
                    GPIO_STRENGTH_2MA,
                    GPIO_PIN_TYPE_STD);

   // SET DRIVE STRENGTH FOR LEDS
   GPIOPadConfigSet(GPIO_PORTF_BASE,
                    GPIO_PIN_1 | GPIO_PIN_2 |
                    GPIO_PIN_3,
                    GPIO_STRENGTH_2MA
                    GPIO_PIN_TYPE_STD);

   // USE THE 16Mhz PIOSC
   SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

   // GLOBAL ENABLE INTERRUPTS
   IntMasterEnable();

   // DISABLE PRIORITY MASKING
   IntPriorityMaskSet(0x0);

   // ENABLE INTERRUPTS FOR PF0 and PF4
   GPIOIntEnable(GPIO_PORTF_BASE,
                 GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

   // REGISTER THE HANDLER
   GPIOIntRegister(GPIO_PORTF_BASE,
                   portf_interrupt_handler);

   // ONLY TRIGGER ON FALLING EDGE
   GPIOIntTypeSet(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4,
                  GPIO_FALLING_EDGE);

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
                TIMERA,
                0x27100x);

   // INTERRUPT TRIGGERS ON TIMEOUT
   TimerIntEnable(TIMER1_BASE,
                  TIMER_TIMEA_TIMEOUT);

   // REGISTER THE HANDLER
   TimerIntRegister(TIMER1_BASE,
                    TIMERA,
                    timer1_interrupt_handler);

}


int main(void)
{

   // INITIALIZE GPIO
   gpio_init(void);

   // INITIALIZE TIMER
   timer1_init(void);

   // SET INITIAL STATE FOR LED
   GPIODataWrite();

   while(1)
   {
   }

   return 0;

}
