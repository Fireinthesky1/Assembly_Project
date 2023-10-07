// James Hicks Lab 4, October 4 2023

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

const uint32_t only_sw1 = 0x00000001;
const uint32_t only_sw2 = 0x00000010;
const uint32_t neither  = 0x00000011;
const uint32_t both     = 0x00000000;

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
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 |
                         GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_0 |GPIO_PIN_4);

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
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

return 0;

}

void wait()
{
    // 16MHz Clock
    // 3 cycles per delay
    uint32_t num_calls = 53333;
    SysCtlDelay(num_calls);
}

// this function returns 3
uint32_t flip(uint32_t *number_to_display)
{
    wait();
    *number_to_display = (*number_to_display) ^ 0xFFFFFFFF;
    return 3;
}

// This function returns 2
uint32_t shift(uint32_t *number_to_display)
{
    wait();
    *number_to_display = (*number_to_display) << 1; // hopefully this works
    *number_to_display = *number_to_display | 0x00000001;
    return 2;
}

// This function returns 1
uint32_t display(uint32_t *number_to_display)
{
    wait();
    uint8_t val = *number_to_display & 0x000000FF;   // hopefully this works
    GPIOPinWrite(GPIO_PORTB_BASE,
                 GPIO_PIN_0 | GPIO_PIN_1 |
                 GPIO_PIN_2 | GPIO_PIN_3,
                 val);
    return 1;
}

int main(void)
{
    init();
    uint32_t number_to_display = 0;
    uint32_t input;
    uint32_t state = display(&number_to_display); // start in display state
    while(1)
    {
        input = GPIOPinRead(GPIO_PORTF_BASE,
                            GPIO_PIN_0 | GPIO_PIN_4);
        switch (state)
        {
            case 1: // display state
                if (input == only_sw1)
                {
                    state = flip(&number_to_display);

                }
                else if (input == only_sw2)
                {
                    state = flip(&number_to_display);
                }
                break;
            case 2: // shift state
                if (input == neither)
                {
                    state = display(&number_to_display);
                }
                else if (input == only_sw1)
                {
                    state = flip(&number_to_display);
                }
                break;
            case 3: // flip state
                if (input == neither)
                {
                    state = display(&number_to_display);
                }
                else if (input == only_sw2)
                {
                    state = shift(&number_to_display);
                }
                break;
            default: // something's wrong if we're here
                break;
        }
    }

    return 0;
}
