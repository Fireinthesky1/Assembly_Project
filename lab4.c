// James Hicks Lab 4, October 4 2023

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>

// (pg 682 ds)
    __asm("GPIO_PORTB_DEN_R:       .field      0x4000551C\n");

// return 0 on success
// set up guards for bad initialization
int init()
{
    // ENABLE CLOCK TO PORT B
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }

    // ENABLE CLOCK TO PORT F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    // UNLOCK PIN PF4 (SW2) <= Does this also enable commit?
    GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_4);

    // SET DIRECTIONS (PB0-PB4, PF0, PF4)
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 |
                         GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_0, GPIO_PIN_4);

    // SET DRIVE STRENGTH (PB0-PB4) (pg 266 tw)
    GPIOPadConfigSet(GPIO_PORTB_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 |
                     GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_4MA,
                     GPIO_PIN_TYPE_STD);

    // ENABLE PUR (PF0, PF4) (pg 266 tw)
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_0 | GPIO_PIN_4,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    // ENABLE MOSC FOR SYSTEM CLOCK (pg 254 ds)
    // Do I need to choose the crystal?
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN);

return 0;

}

void wait()
{
    // 16MHz Clock
    // 3 cycles per delay
    uint32_t num_calls = 53333;
    SysCtlDelay(num_calls);
    return 0;
}

void flip(bool flip, uint32_t *number_to_display)
{
    wait();
    if(flip)
    {
        *number_to_display = ~(*number_to_display);
    }
    uint32 state = GPIOPinRead(GPIO_PORTF_BASE,
                               GPIO_PIN_0 | GPIO_PIN_4);
    return 0;
}

void shift(uint32_t *number_to_display)
{
    
}

void display(uint32_t *number_to_display)
{
    wait();
    GPIOPinWrite();
}

int main(void)
{
    init();
    uint_32 number_to_display = 0;
    uint_32 state = GPIOPinRead(GPIO_PORTF_BASE,
                                GPIO_PIN_0 | GPIO_PIN_4);
    const uint_32 only_sw1 = 
    const uint_32 only_sw2 =
    const uint_32 neither = 
    const uint_32 both = 

    return 0;
}