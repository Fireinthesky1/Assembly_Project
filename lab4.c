// James Hicks Lab 4, October 4 2023

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>

// (pg 682 ds)
    __asm("GPIO_PORTB_DEN_R:       .field      0x4000551C\n"
          "GPIO_PORTF_DEN_R:       .field      0x4002551C");

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
                     GPIO_PIN_TYPE_STD); // ask prof is GPIO_PIN_TYPE_STD is correct

    // ENABLE PUR (PF0, PF4) (pg 266 tw)
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_0 | GPIO_PIN_4,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    // SET DIGITAL ENABLE (PB0-PB4)
    __asm("         AND         R0, #0\n"
          "         AND         R1, #0\n"
          "         LDR         R0, GPIO_PORTB_DEN_R\n"
          "         LDR         R1, [R0]\n"
          "         ORR         R1, #0xF\n"
          "         STR         R1, [R0]"

    // SET DIGITAL ENABLE (PF0, PF4)
    __asm("         AND         R0, #0\n"
          "         AND         R1, #0\n"
          "         LDR         R0, GPIO_PORTB_DEN_F\n"
          "         LDR         R1, [R0]\n"
          "         ORR         R1, #0x11\n"
          "         STR         R1, [R0]");

    // ENABLE MOSC FOR SYSTEM CLOCK (pg 254 ds)
    __asm("         AND         R0, #0\n"
          "         AND         R1, #0\n"
          "         LDR         R0, SYSCTRL_RCC_R\n"
          "         LDR         R1, [R0]\n"
          "         EOR         R1, #0x30\n"
          "         STR         R1, [R0]");

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

void complement(uint32_t *number_to_display)
{
    *number_to_display = ~(*number_to_display);
    return 0;
}

void shift_left(uint32_t *number_to_display)
{
    
}

// 0 if none, 1 if sw1, 2 if sw2
int poll_switches(uint32_t *number_to_display)
{
    
}

void display(uint32_t *number_to_display)
{
    GPIOPinWrite();
}

int main(void)
{
    uint_32 number_to_display = 0;
    init();
    while(1)
    {
        poll_switches();
        display(*number_to_display);
    }
    return 0;
}