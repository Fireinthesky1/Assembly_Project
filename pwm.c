// James Hicks Lab 5, November 25 2023

/*
 FOUND THIS IN /inc/GPIO.c

 \note A subset of GPIO pins on Tiva devices, notably those used by the
 JTAG/SWD interface and any pin capable of acting as an NMI input, are
 locked against inadvertent reconfiguration.  These pins must be unlocked
 using direct register writes to the relevant GPIO_O_LOCK and GPIO_O_CR
 registers before this function can be called.  Please see the ``gpio_jtag''
 example application for the mechanism required and consult your part
 datasheet for information on affected pins.

*/

#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_types.h> // <- DANCING WITH THE DEVIL HERE
#include <inc/hw_memmap.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>

// TODO::THE COMPILER IS FORCING ME TO PUT THIS HERE
//       IT'S ALREADY DEFINED IN pin_map.h line 11829
#define GPIO_PF1_M0PWM1         0x00050406

/*
this comes from the ms18 datasheet (20 microseconds)
or 20 millisecond
*/
#define pwm_period 20000

//TODO::BULLET PROOF THIS
/*
this comes from the datasheet
*/
#define min_angle 0.0
#define max_angle 180.0
#define min_pulse 0.5
#define max_pulse 2.5
#define calculate_pulse_width(angle) \
    ((angle - min_angle) / (max_angle - min_angle) * \
    (max_pulse - min_pulse) + min_pulse)

#define sw1 GPIO_PIN_4
#define sw2 GPIO_PIN_0
#define pwm GPIO_PIN_1

// states
#define sw1_pressed     0x01
#define sw2_pressed     0x10
#define both_pressed    0x00
#define neither_pressed 0x11

uint32_t current_state = neither_pressed;
bool move_left  = false;
bool move_right = false;
const float left_rotate = calculate_pulse_width(10.0);
const float right_rotate = calculate_pulse_width(-10.0);

void portf_isr(void)
{
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
  GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
  TimerDisable(TIMER0_BASE, TIMER_BOTH);
  // 15 millisecond delay
  TimerLoadSet(TIMER1_BASE, TIMER_A, 0x3A980);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerEnable(TIMER0_BASE, TIMER_BOTH);
}

void pwm_isr(void)
{
  PWMIntDisable(PWM0_BASE, PWM_INT_GEN_0);
  PWMGenIntClear(PWM0_BASE, PWM_INT_GEN_0, PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD); // TODO BULLETPROOF

  // handle left and right movements
  if(move_left)
    {
      // move servo left
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, left_rotate);
    }
  else if(move_right)
    {
      // move servo right
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, right_rotate);
    }
  else
    {
      // don't move at all TODO::BULLETPROOF THIS
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 0.0);
    }

  // RESET MOVEMENT FLAGS
  move_left = false;
  move_right = false;

  PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
}

// debounce timer
void timer_isr(void)
{
  TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerDisable(TIMER0_BASE, TIMER_BOTH);
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  // CHECK THE STATE OF THE SWITCHES
  uint32_t input = GPIOPinRead(GPIO_PORTF_BASE, sw2 | sw1);

  // TODO:: BULLETPROOF THIS
  switch(current_state)
    {

    case neither_pressed:
      if(input == sw1_pressed)
        {
          move_left  = true;
          current_state = sw1_pressed;
        }
      else if(input == sw2_pressed)
        {
          move_right = true;
          current_state = sw2_pressed;
        }
      break;

    case sw1_pressed:
      if(input == neither_pressed)
        {
          current_state = neither_pressed;
        }
      else if(input == both_pressed)
        {
          move_right = true;
          current_state = both_pressed;
        }
      break;

    case sw2_pressed:
      if(input == neither_pressed)
        {
          current_state = neither_pressed;
        }
      else if(input == both_pressed)
        {
          move_left  = true;
          current_state = both_pressed;
        }
      break;

    case both_pressed:
      if(input == sw1_pressed)
        {
          current_state = sw1_pressed;
        }
      else if(input == sw2_pressed)
        {
          current_state = sw2_pressed;
        }
      break;

    default:
      // very bad if we end up here
      break;
    }

  GPIOIntEnable(GPIO_PORTF_BASE, sw1 | sw2);
}

////////////////////////////////////////////////////////////////////////////

/*
pf0 <- sw2
pf4 <- sw1
pf1 <- pwm output
*/
void gpio_init(void)
{
  // SEND CLOCK TO PORTF
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

  // UNLOCK PORTF pins 0,1,4
  GPIOUnlockPin(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4);

  // SET DIRECTIONS
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

  // SET PIN TYPE PWM
  GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

  // PULL UP RESISTORS
  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_0 | GPIO_PIN_4,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD_WPU);

  // CONFIGURE THE PWM PIN
  GPIOPinConfigure(GPIO_PF1_M0PWM1);

  //ENABLE GPIO INTERRUPTS FOR PF0 and PF4
  GPIOIntEnable(GPIO_PORTF_BASE,
                GPIO_INT_PIN_0 | GPIO_INT_PIN_4);

  // REGISTER THE INTERRUPT HANDLER
  GPIOIntRegister(GPIO_PORTF_BASE, portf_isr);
}


// TODO::THIS FUNCTION NEEDS COMMENTING AND WORK
void pwm_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0))
    {
    }
  // divide by one here
  SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
  PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, pwm_period);
  PWMGenEnable(PWM0_BASE, PWM_GEN_0);
  // THE NEUTRAL POSITION IS 1500 microseconds (does this work?)
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 1500);
  PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);
}

void timer_init(void)
{
  // SEND CLOCK TO TIMER0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }

  // SET THE SOURCE TO PIOSC
  TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_PIOSC);

  // DISABLE THE TIMER FOR INITIALIZATION
  TimerDisable(TIMER0_BASE, TIMER_BOTH);

  // FULL WIDTH PERIODIC
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

  // 15 MILLISECOND DELAY
  TimerLoadSet(TIMER0_BASE, TIMER_A, 0x3A980);

  // ENABLE THE INTERRUPT FOR TIMER0
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  // REGISTER THE INTERRUPT HANDLER
  TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);
}

void main(void)
{
  SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

  gpio_init();
  timer_init();
  pwm_init();

  // ENABLE INTERRUPTS
  IntMasterEnable();
  IntPriorityMaskSet(0x0);
  PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);

  while(1)
    {
      // loop forever
    }
}
