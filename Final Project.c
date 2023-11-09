// FINAL PROJECT
// JAMES HICKS
// 11/3/2023 - COMPLETED: 11/9/2023

#include <stdint.h>
#include <stdbool.h>
#include "display.h"
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_adc.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/timer.h>
#include <driverlib/adc.h>

#define mph            0
#define kmph           1
#define set            2
#define neither        0
#define acc            1
#define brk            2
#define digital_max 4095

extern void       display_interrupt_handler(void);

// 0 = mph, 1 = kmph, 2 = set
uint8_t           current_display_mode      = 0;

// 0 = neither, 1 = acc, 2 = brk
uint8_t           current_acc_brk_mode      = 0;

volatile uint32_t speed_off_adc             = 0;
volatile uint32_t set_off_adc               = 0;
bool              speed_adc_ready           = false;
bool              set_adc_ready             = false;

int32_t           current_speed             = 0;
int32_t           current_set               = 0;

bool              breaks                    = false;
bool              acceleration              = false;
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

// debounce timer0
// debounce starts timer 2 (the 5 sec timer)
void debounce_interrupt_handler(void)
{
    // DISABLE TIMER0 INTERRUPTS
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerDisable(TIMER0_BASE, TIMER_BOTH);

    // CLEAR TIMER0 INTERRUPTS
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // HANDLE STATE TRANSITIONS
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4) == 0)
    {
        current_display_mode++;
        current_display_mode %= 3;
    }

    // DISABLE THE "5 SEC" TIMER
    TimerDisable(TIMER2_BASE, TIMER_BOTH);

    // LOAD THE "5 SEC" TIMER (full width)
    // 80,000,000 cycles / (16,000,000 cycles per sec = 5 sec
    TimerLoadSet(TIMER2_BASE, TIMER_A, 0x4C4B400);

    // REENABLE "5 SEC" timer interrupts
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // START THE "5 SEC" timer
    TimerEnable(TIMER2_BASE, TIMER_BOTH);

    // REENABLE GPIO INTERRUPTS
    GPIOIntEnable(GPIO_PORTF_BASE,
                  GPIO_PIN_4);
}

// FIVE SECOND TIMER (TO RESET DISPLAY MODE AT TIMEOUT)
void timer_2_interrupt_handler(void)
{
    // DISABLE TIMER2 INTERRUPTS
    TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // DISABLE TIMER2
    TimerDisable(TIMER2_BASE, TIMER_BOTH);

    // CLEAR TIMER2 INTERRUPTS
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // RESET CURRENT DISPLAY MODE TO MPH
    current_display_mode = 0;
}

// SPEED ADC
void adc0_interrupt_handler(void)
{
    // DISABLE ADC0 INTERRUPTS
    ADCIntDisable(ADC0_BASE, 3);

    // CLEAR ADC0 INTERRUPTS
    ADCIntClear(ADC0_BASE, 3);

    // GET THE NUMBER OFF OF ADC0
    ADCSequenceDataGet(ADC0_BASE, 3, &speed_off_adc);

    speed_adc_ready = true;

    // START THE NEXT SEQUENCE
    ADCProcessorTrigger(ADC0_BASE, 3);

    // REENABLE ADC0 INTERRUPTS
    ADCIntEnable(ADC0_BASE, 3);
}

// SET ADC
void adc1_interrupt_handler(void)
{
    // DISABLE ADC1 INTERRUPTS
    ADCIntDisable(ADC1_BASE, 3);

    // CLEAR ADC1 INTERRUPTS
    ADCIntClear(ADC1_BASE, 3);

    // GET THE NUMBER OFF OF ADC1
    ADCSequenceDataGet(ADC1_BASE, 3, &set_off_adc);

    set_adc_ready = true;

    ADCProcessorTrigger(ADC1_BASE, 3);

    // REENABLE ADC1 INTERRUPTS
    ADCIntEnable(ADC1_BASE, 3);
}

///////////////////////////////////////////////////////////////////////////


/* PORT B Pin: 5        <- ADC current speed
   PORT E Pin: 5        <- ADC set point */
void adc_init(void)
{
    // Port B Pin 5 (ADC0) current speed
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }

    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

    ADCClockConfigSet(ADC0_BASE,
                      ADC_CLOCK_SRC_PIOSC |
                      ADC_CLOCK_RATE_FULL,
                      1);

    ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
                                                                           // FIX INLINE ASSEMBLY WITH VALVANO C
                                                                           // ADC_0_CTL
                                                                           // ADC0_BASE, ADC1_BASE

    __asm("ADCCTL_0:      .field      0x40038038  ; pg 850 \n"
          "; SET DITHER BIT FOR 0 BASE                     \n"
          "             LDR         R10, ADCCTL_0          \n"
          "             LDR         R11, [R10]             \n"
          "             ORR         R11, #0x40     ; bit 6 \n"
          "             STR         R11, [R10]             \n"
          );

    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE  |
                                              ADC_CTL_END |
                                              ADC_CTL_CH11);

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
          "; SET DITHER BIT FOR 1 BASE                     \n"
          "             LDR         R10, ADCCTL_1          \n"
          "             LDR         R11, [R10]             \n"
          "             ORR         R11, #0x40     ; bit 6 \n"
          "             STR         R11, [R10]             \n"
         );

    ADCSequenceStepConfigure(ADC1_BASE, 3, 0,
                             ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH8);

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
void gpio_init(void)
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
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
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
                     GPIO_PIN_TYPE_STD_WPU);

//MISC INITIALIZATION//////////////////////////////////////////////////////

    // USE THE 16 MHz PIOSC
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

    //// ALLOW THE PROCESSOR TO RESPOND TO INTERRUPTS
    // IntMasterEnable();

    // DISABLE PRIORITY MASKING
    //IntPriorityMaskSet(0x0);

    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);

    // ENABLE INTERRUPTS FOR PF4
    GPIOIntEnable(GPIO_PORTF_BASE,
                  GPIO_INT_PIN_4);

    GPIOIntRegister(GPIO_PORTF_BASE,
                    switch_interrupt_handler);
}

// DEBOUNCE TIMER
void timer0_init(void)
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
}

// DISPLAY TIMER
void timer1_init(void)
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
                 0x15B38); //0x15B38

    // ENABLE THE INTERUPT FOR TIMER1
    TimerIntEnable(TIMER1_BASE,
                   TIMER_TIMA_TIMEOUT);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    TimerIntRegister(TIMER1_BASE,
                     TIMER_A,
                     display_interrupt_handler);
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

    // 5 seconds (16000000*5)
    TimerLoadSet(TIMER2_BASE, TIMER_A, 0x4C4B400);

    // ENABLE THE INTERUPT FOR TIMER1
    TimerIntEnable(TIMER2_BASE,
                   TIMER_TIMA_TIMEOUT);

    // REGISTER THE INTERRUPT FOR THE DISPLAY
    TimerIntRegister(TIMER2_BASE,
                     TIMER_A,
                     timer_2_interrupt_handler);

    return 0;
}

uint8_t get_next_acc_brk_state(void)
{
    int difference = current_speed - current_set;
    switch(current_acc_brk_mode)
    {
        case neither:
            if(!breaks && difference > 3)
            {
                return 2;
            }
            if(!acceleration && difference < -3)
            {
                return 1;
            }
            break;
        case acc:
            if(difference >= 0)
            {
                return 0;
            }
            break;
        case brk:
            if(difference <= 0)
            {
                return 0;
            }
            break;
        default:
            return current_acc_brk_mode;
    }
    return current_acc_brk_mode;
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

    // INITIAL PROCESSOR TRIGGER
    ADCProcessorTrigger(ADC0_BASE, 3);
    ADCProcessorTrigger(ADC1_BASE, 3);

    while(1)
    {

        // Never update current speed until new value is ready
        // Never Update unless the current digit being displayed is 1

        if(speed_adc_ready && display_ready)
        {
            current_speed =  100 * speed_off_adc / digital_max;
            speed_adc_ready = false;
            display_ready = false;
        }

        // Never update current set until new value is ready
        if(set_adc_ready && display_ready)
        {
            current_set = 60 * set_off_adc / digital_max + 20;
            speed_adc_ready = false;
            display_ready = false;
        }

        switch (current_display_mode)
        {
            case mph:
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00);
                number_to_display = current_speed;
                break;
            case kmph:
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x00);
                number_to_display = current_speed * 1.6093;
                break;
            case set:
                GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0x01);
                number_to_display = current_set;
                break;
            default:
                return -1;
        }

        current_acc_brk_mode = get_next_acc_brk_state();

        switch(current_acc_brk_mode)
        {
            case neither: // BLUE
                GPIOPinWrite(GPIO_PORTF_BASE,
                            GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                            0x04);
                break;
            case acc: // GREEN
                GPIOPinWrite(GPIO_PORTF_BASE,
                            GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                            0x08);
                break;
            case brk: // RED
                GPIOPinWrite(GPIO_PORTF_BASE,
                            GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                            0x02);
                break;
            default:
                return -1;
        }
    }
}
