#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

#define only_sw1 0x00000001
#define only_sw2 0x00000010
#define neither  0x00000011
#define both     0x00000000

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

    // SET DIRECTIONS (PB0-PB4, PF0, PF4)
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,
                          GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,
                         GPIO_PIN_0 | GPIO_PIN_4);

    // UNLOCK PIN PF4 (SW1,SW2)
    GPIOUnlockPin(GPIO_PORTF_BASE,
                  GPIO_PIN_0 | GPIO_PIN_4);

    // ENABLE PUR (PF0, PF4) (pg 266 tw)
    GPIOPadConfigSet(GPIO_PORTF_BASE,
                     GPIO_PIN_0 | GPIO_PIN_4,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    // SET DRIVE STRENGTH (PB0-PB4) (pg 266 tw)
    GPIOPadConfigSet(GPIO_PORTB_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 |
                     GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_4MA,
                     GPIO_PIN_TYPE_STD);

    // ENABLE MOSC FOR SYSTEM CLOCK (pg 254 ds)
    // Do I need to choose the crystal?
    SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

return 0;

}

/* uint8_t extract_bottom_8_bits(uint32_t val)
{
  uint8_t result = (uint8_t)(val & 0xFF);
  return result;
} */

void write(uint32_t* number_to_display)
{
  //uint8_t val_to_write = extract_bottom_8_bits(*number_to_display);
  GPIOPinWrite(GPIO_PORTB_BASE,
               GPIO_PIN_0 | GPIO_PIN_1 |
               GPIO_PIN_2 | GPIO_PIN_3,
               *number_to_display);
}

void wait()
{
  // 16 MHz clock
  // 3 cycles per clock
  uint32_t num_calls = 53333;
  SysCtlDelay(num_calls);
}

// actions
void flip(uint32_t* number_to_display)
{
  wait();
  *number_to_display ^=  0xFFFFFFFF;
  write(number_to_display);
}

void shift(uint32_t* number_to_display)
{
  wait();
  *number_to_display = (*number_to_display) << 1;
  *number_to_display |= 0x00000001;
  write(number_to_display);
}

// state 1
uint32_t idle(uint32_t input, uint32_t* number_to_display)
{
  wait();
  switch(input)
  {
  case only_sw1:
    flip(number_to_display);
    return 2; // flip read
  case only_sw2:
    shift(number_to_display);
    return 4; // shift read
  default:
    return 1; // stay in idle
  }
}

// state 2
uint32_t fread(uint32_t input, uint32_t* number_to_display)
{
  wait();
  switch(input)
  {
  case neither:
    return 1; // idle
  case both:
    shift(number_to_display);
    return 3; // in flip do shift read
  default:
    return 2; // stay in flip read
  }
}

// state 3
uint32_t ifread(uint32_t input)
{
  wait();
  switch(input)
  {
  case neither:
    return 1; // idle
  case only_sw1:
    return 2; // flip read
  case only_sw2:
    return 4; // shift read
  default:
    return 3; // stay in "in flip do shift read" 
  }
}

// state 4
uint32_t sread(uint32_t input, uint32_t *number_to_display)
{
  wait();
  switch(input)
  {
  case neither:
    return 1; // idle
  case both:
    flip(number_to_display);
    return 5; // in shift do flip read
  default:
    return 4; // stay in shift read
  }
}

// state 5
uint32_t isread(uint32_t input)
{
  wait();
  switch(input)
  {
  case neither:
    return 1; // idle
  case only_sw1:
    return 2; // flip read
  case only_sw2:
    return 4; // shift read
  default:
    return 5; // stay in "in shift do flip read"
  }
}

int main(void)
{
    init();
    uint32_t number_to_display = 0;
    uint32_t input = GPIOPinRead(GPIO_PORTF_BASE,
                                 GPIO_PIN_0 | GPIO_PIN_4);
    uint32_t state = idle(input, &number_to_display); // start in idle
    while(1)
    {
      switch (state)
        {
        case 1: // idle
           state = idle(input, &number_to_display);
           break;
        case 2: // flip read
           state = fread(input, &number_to_display);
           break;
        case 3: // in flip do shift read
           state = ifread(input);
           break;
        case 4: // shift read
           state = sread(input, &number_to_display);
           break;
        case 5: // in shift do flip read
           state = isread(input);
           break;
         default: // ERROR IN STATE MACHINE
           return 1;
        }
      write(&number_to_display);
      input = GPIOPinRead(GPIO_PORTF_BASE,
                          GPIO_PIN_0 | GPIO_PIN_4);
    }
}
