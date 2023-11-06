// FINAL PROJECT
// JAMES HICKS
// 11/3/2023

#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>

#define mph            0
#define kmph           1
#define set            2
#define neither        0
#define acc            1
#define brk            2
#define digital_max 4095

extern void display_interrupt_handler(void);

// 0 = mph, 1 = kmph, 2 = set
uint8_t current_display_mode = 0;

// 0 = neither, 1 = acc, 2 = brk
uint8_t current_acc_brk_mode = 0;

uint32_t speed_off_adc;
uint32_t set_off_adc;

uint8_t number_to_display;
uint8_t digit_to_display;

///////////////////////////////////////////////////////////////////////////
// Constantly monitor sensor and set point control for changes

// SEVEN SEGMENT DISPLAY //
    // SHOWS THE INTEGER PORTION OF WHATEVER VALUE IS TO BE DISPLAYED BY
    // THE CURRENTLY SELECTED MODE

    // MPH MODE WILL SHOW THE CURRENT SPEED IN MPH AND THE DISPLAY MODE
    // INDICATOR WILL BE OFF

    // KMPH MODE WILL SHOW THE CURRENT SPEED IN KILOMETERS PER HOUR AND THE
    // DISPLAY MODE INDICATOR WILL BE OFF

    // SET POINT MODE WILL SHOW THE SET POINT IN MPH AND THE DISPLAY MODE
    // INDICATOR WILL BE ON

    // AT STARTUP THE DISPLAY MODE WIL BE MPH
    // PRESSING SWITCH 1
        // MPH  -> KMPH
        // KMPH -> SET
        // SET  -> MPH

    // IF THE DISPLAY MODE IS ANYTHING OTHER THAN MPH AND IT HAS BEEN 5 SEC
    // SINCE THE LAST TIME THE MODE SELECTOR WAS PRESSED THE DISPLAY WILL
    // AUTOMATICALLY CHANGE BACK TO MPH MODE

// BRAKING / ACCELERATING MODE INDICATOR //
    // IF THE BRAKES ARE OFF AND THE SPEED INCREASES MORE THAN 3MPH ABOVE
    // THE SET POINT THE BRAKES WILL TURN ON

    // IF THE BREAKES ARE ON THE SPEED DECREASES TO MEET THE SET POINT
    // THE BRAKES THEN TURN OFF

    // IF ACCELERATION IS OFF AND THE SPEED DECREASED MORE THAN 3 MPG BELOW
    // THE SET POINT ACCELERATION WILL TURN ON

    // IF ACCELERATION IS ON AND THE SPEED INCREASES TO MEET OR EXCEED THE
    // SET POINT THE ACCLERATOR WILL TURN OFF

    // ANY TIME NEITHER THE BRAKES NOR ACCELERATION IS ON THE MODE INDICATOR
    // MUST POSITIVELY INDICATE THIS
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//INTERRUPT SERVICE ROUTINES

void switch_interrupt_handler(void)
{
    // DISABLE GPIO INTERRUPTS
    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);

    // CLEAR GPIO INTERRUPTS
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);

    // DISABLE THE DEBOUNCE TIMER (timer0)
    TimerDisable(TIMER0_BASE, TIMER_BOTH);

    // LOAD THE DEBOUNCE TIMER (only timer "a" for full width)
    TimerLoadSet(TIMER0_BASE,TIMER_A, 0x27100);

    // ENABLE TIMER0 INTERRUPTS
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // START THE TIMER
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
}

// debounce timer
// debounce starts timer 2 (the 5 sec timer)
void debounce_interrupt_handler(void)
{
    // DISABLE TIMER0 INTERRUPTS
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // CLEAR TIMER0 INTERRUPTS
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // HANDLE STATE TRANSITIONS
    current_display_mode++;
    current_display_mode %= 3;

    // DISABLE THE "5 SEC" TIMER
    TimerDisable(TIMER2_BASE, TIMER_BOTH);

    // LOAD THE "5 SEC" TIMER (full width)
    // 80,000,000 cycles / (16,000,000 cycles per sec = 5 sec
    TimerLoadSet(TIMER2_BASE, TIMERA, 0x4C4B400)

    // START THE "5 SEC" timer
    TimerEnable(TIMER2_BASE, TIMER_BOTH);
}

// display mode timer
void timer_2_interrupt_handler(void)
{
    // DISABLE TIMER2 INTERRUPTS
    TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // CLEAR TIMER2 INTERRUPTS
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // RESET CURRENT DISPLAY MODE TO MPH
    current_display_mode = 0;
}

// SPEED ADC
void adc0_interrupt_handler(void)                                          // come back
{
    // DISABLE ADC0 INTERRUPTS
    ADCIntDisable(ADC0_BASE, 3);

    // CLEAR ADC0 INTERRUPTS
    ADCIntClear(ADC0_BASE, 3);

    // GET THE NUMBER OFF OF ADC0
    ADCSequenceDataGet(ADC0_BASE, 3, &speed_off_adc);

    // REENABLE ADC0 INTERRUPTS
    ADCIntEnable(ADC0_BASE, 3);
}

// SET ADC                                                                 // come back
void adc1_interrupt_handler(void)
{
    // DISABLE ADC1 INTERRUPTS
    ADCIntDisable(ADC1_BASE, 3);

    // CLEAR ADC1 INTERRUPTS
    ADCIntClear(ADC1_BASE, 3);

    // GET THE NUMBER OFF OF ADC1
    ADCSequenceDataGet(AD1_BASE, 3, &set_off_adc);

    // REENABLE ADC1 INTERRUPTS
    ADCIntEnable(ADC1_BASE, 3);
}

///////////////////////////////////////////////////////////////////////////


/* PORT B Pin: 5        <- ADC current speed
   PORT E Pin: 5        <- ADC set point */
int adc_init(void)
{
    // Port B Pin 5 (ADC0) current speed
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }

    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

    ADCClockConfigSet(ADC0_BASE,
                      ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL,
                      1);

    ADCHardwareOversampleConfigure(ADC0_BASE, 64); // <- be careful here

    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    __asm("ADCCTL_0:      .field      0x40038038  ; pg 850 \n"
          "ADCSAC_0:      .field      0x40038030  ; pg 847 \n"
          "; SET DITHER BIT FOR 0 BASE                     \n"
          "             LDR         R10, ADCCTL_0          \n"
          "             LDR         R11, [R10]             \n"
          "             ORR         R11, 0x40     ; bit 6  \n"
          "             STR         R11, [R10]             \n"
          );

    // UNDERSTAND THIS
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0,
                             ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);

    ADCSequenceEnable(ADC0_BASE, 3);

    ADCIntRegister(ADC0_BASE, 3, adc0_interrupt_handler);

    ADCIntEnable(ADC0_BASE, 3);

    // Port E Pin 5 (ADC 8) Set Point

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1))
    {
    }

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    ADCHardwareOversampleConfigure(ADC1_BASE, 64);

    __asm("ADCCTL_1:      .field      0x40039038  ; pg 850 \n"
          "ADCSAC_1:      .field      0x40039030  ; pg 847 \n"
          "; SET DITHER BIT FOR 1 BASE                     \n"
          "             LDR         R10, ADCCTL_1          \n"
          "             LDR         R11, [R10]             \n"
          "             ORR         R11, 0x40     ; bit 6  \n"
          "             STR         R11, [R10]             \n"
         );

    ADCSequenceStepConfigure(ADC1_BASE, 3, 0,
                             ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH1);

    ADCSequenceEnable(ADC1_BASE, 3);

    ADCIntRegister(ADC1_BASE, 3, adc1_interrupt_handler);

    ADCIntEnable(ADC1_BASE, 3);
}



/*
PORT D Pin: 0,1,2,3  <- display data sseg
PORT E Pin: 1,2,3    <- control lines sseg

PORT B Pin: 0        <- display mode indicator (LED peripherals board)

PORT F Pin: 1,2,3    <- acc/brk indicator (RBG)
                        RED   <- brakes on
                        BLUE  <- continue as is
                        GREEN <- Acceleration on

PORT F Pin: 4        <- display mode selector (sw 1)
*/
int gpio_init(void)
{
//ENABLE CLOCK (D,E,B,F)///////////////////////////////////////////////////

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

    // ENABLE CLOCK TO PORT B
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    // ENABLE CLOCK TO PORT F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

//SET DIRECTIONS (D,E,B,F)/////////////////////////////////////////////////

    // DISPLAY PORT D OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,
                          GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3);

    // CONTROL PORTE OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE,
                          GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3);

    // DISPLAY MODE INDICATOR PORT B OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,
                          GPIO_PIN_0);

    // DISPLAY MODE SELECTOR PORT B INPUT
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    // ACC/BRK MODE INDICATOR PORTF OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3);

//CONFIGURE PADS (D,E,B,F)/////////////////////////////////////////////////

    // CONFIGURE SSEG DISPLAY PORT D
    GPIOPadConfigSet(GPIO_PORTD_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 |
                     GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    // CONFIGURE SSEG CONTROL PORT E
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_1 | GPIO_PIN_2 |
                     GPIO_PIN_3,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    // CONFIGURE DISPLAY MODE INDICATOR PORT B
    GPIOPadConfigSet(GPIO_PORTB_BASE,
                     GPIO_PIN_0,
                     GPIO_STRENGTH_4MA,
                     GPIO_PIN_TYPE_STD);

    // CONFIGURE ACC/BRK INDICATOR PORT F
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_1 | GPIO_PIN_2 |
                     GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, 
                     GPIO_PIN_TYPE_STD);

    // CONFIGURE DISPLAY MODE SELECTOR PORT F
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_4,
                     GPIO_STRENGTH_2MA, 
                     GPIO_PIN_TYPE_WPU);

//MISC INITIALIZATION//////////////////////////////////////////////////////

    // USE THE 16 MHz PIOSC
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

    // ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
    IntMasterEnable();

    // DISABLE PRIORITY MASKING
    IntPriorityMaskSet(0x0);

    // ENABLE INTERRUPTS FOR PF4
    GPIOIntEnable(GPIO_PORTF_BASE,
                  GPIO_INT_PIN_4);

    GPIOIntRegister(GPIO_PORTF_BASE,
                    switch_interrupt_handler);

    return 0;
}

// DEBOUNCE TIMER
int timer0_init(void)
{
    // ENABLE THE TIMER1 PERIPHERAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // WAIT FOR TIMER1 MODULE TO BE READY
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    
    // SET CLOCK SOURCE FOR TIMER 1
    TimerClockSourceSet(TIMER0_BASE,
                        TIMER_CLOCK_PIOSC);

    // DISABLE THE TIMER FOR INITIALIZATION
    TimerDisable(TIMER0_BASE,
                 TIMER_BOTH);

    // full width periodic
    TimerConfigure(TIMER0_BASE,
                   TIMER_CFG_PERIODIC);

    // 10 millisecond delay
    TimerLoadSet(TIMER0_BASE,
                 TIMER_A,
                 0x27100);

    // ENABLE THE INTERUPT FOR TIMER1
    TimerIntEnable(TIMER0_BASE,
                   TIMER_TIMA_TIMEOUT);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    TimerIntRegister(TIMER0_BASE,
                     TIMER_A,
                     debounce_interrupt_handler);

  return 0;
}

// DISPLAY TIMER
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

    // 60 fps per display
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

/*
THIS TIMER IS TRIGGERED WHENEVER THE DISPLAY MODE SELECTOR
SWITCH IS PRESSED. IF THIS TIMER COMPLETES ITS COUNT IT TRIGGERS
AN INTERRUPT TO RESET THE DISPLAY MODE
THIS TIMERS COUNT SHOULD BE 5 SECONDS LONG
*/
int timer2_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    // WAIT FOR TIMER1 MODULE TO BE READY
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
    {
    }

    // SET CLOCK SOURCE FOR TIMER 1
    TimerClockSourceSet(TIMER2_BASE,
                        TIMER_CLOCK_PIOSC);

    // DISABLE TIMER FOR INITIALIZATION
    TimerDisable(TIMER2_BASE, TIMER_BOTH);

    // FULL WIDTH PERIODIC
    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);

    // 5 seconds
    TimerLoadSet(TIMER2_BASE, TIMERA, 0x4C4B400);

    // ENABLE THE INTERUPT FOR TIMER1
    TimerIntEnable(TIMER2_BASE,
                   TIMER_TIMA_TIMEOUT);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    TimerIntRegister(TIMER2_BASE,
                     TIMER_A,
                     timer_2_interrupt_handler);

    return 0;
}

int main(void)
{
    // INITIALIZE GPIO
    gpio_init();

    // INITIALIZE TIMERS
    timer0_init();
    timer1_init();
    timer2_init();

    // INITIALIZE ADCS
    adc_init();

    // START TIMERS
    TimerEnable(TIMER1_BASE, TIMER_BOTH);

    // LOOP FOREVER

    while(1)
    {
        // COLLECT ADC DATA
        ADCProcessorTrigger(ADC0_BASE, 3);
        ADCProcessorTrigger(ADC1_BASE, 3);

        // DISPLAY MODE STATE
        switch (current_display_mode)
        {
            case mph:
                GPIODataWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00);
                number_to_display = (speed_off_adc / digital_max) * 100;   // make sure this works
                break;
            case kmph:
                GPIODataWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00);
                number_to_display =
                             (speed_off_adc / digital_max) * 100 * 1.6093; // make sure this works
                break;
            case set:
                GPIODataWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x01);
                number_to_display = (set_off_adc / digital_max) * 60 + 20; // make sure this works
                break;
            default: 
                return -1;
                break; // <- VERY BAD IF WE END UP HERE
        }

        // UPDATE ACC/BRK STATE
        current_speed = (speed_off_adc / 4095) * 100;
        current_set = (set_off_adc / 4095) * 60 + 20;
        switch(current_acc_brk_mode)
        {
            case neither: // BLUE
                GPIODataWrite(GPIO_PORTF_BASE,
                              GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                              0x04);
                if(current_speed - current_set >= 3)
                    current_acc_brk_mode = brk;
                else if(current_speed - current_set <= -3)
                    current_acc_brk_mode = acc;
                else
                    current_acc_brk_mode = neither;
                break;
            case acc: // GREEN
                GPIODataWrite(GPIO_PORTF_BASE,
                              GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                              0x08);
                if(current_speed - current_set >= 3)
                    current_acc_brk_mode = brk;
                else if(current_speed - current_set <= -3)
                    current_acc_brk_mode = acc;
                else
                    current_acc_brk_mode = neither;
                break;
            case brk: // RED
                GPIODataWrite(GPIO_PORTF_BASE,
                              GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                              0x02);
                if(current_speed - current_set >= 3)
                    current_acc_brk_mode = brk;
                else if(current_speed - current_set <= -3)
                    current_acc_brk_mode = acc;
                else
                    current_acc_brk_mode = neither;
                break;
        }
    }

    return 0;
}