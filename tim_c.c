#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>

#define LED_ON  0x00000008

uint32_t current_state;

void systick_interrupt_handler(void)
{
    SysTickIntDisable();
    SysTickDisable();
    current_state = GPIOPinRead (GPIO_PORTF_BASE, GPIO_PIN_3);
    current_state ^= LED_ON;
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, current_state);
    SysTickPeriodSet(0x00FFFFFF);
    SysTickIntEnable();
    SysTickEnable();
}

void init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    // SET DIRECTIONS
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
                          GPIO_PIN_3);

    // SET DEN, and drive strength
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_3,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);

    SysTickDisable();
    SysTickPeriodSet(0x00FFFFFF);
    SysTickIntEnable();
    SysTickIntRegister(systick_interrupt_handler);
}

int main(void)
{
    init();
    SysTickEnable();
    while(1)
    {
    }
}
