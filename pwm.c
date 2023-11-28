// James Hicks Lab 5, November 25 2023

#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>

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
  PWMGenIntClear(PWM0_BASE, PWM_INT_GEN_0);

  // handle left and right movements
  if(move_left)
    {
      // move servo left
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, left_rotate);
    }
  else if(move_right)
    {
      // move sero right
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, right_rotate);
    }
  else
    {
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
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
  GPIOPinTypeGPIOPWM(GPIO_PORTF_BASE, GPIO_PIN_1);
  GPIOPinConfigure(GPIO_PF1_M0PWM1);
}

void pwm_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0))
    {
    }
  // divide by one here
  SysCtlPwmClockSet(SYSCTL_PWMDIV_1);
  PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, pwm_period);
  PWMGenEnable(PWM0_BASE, PWM_GEN_0);
  // THE NEUTRAL POSITION IS 1500 microseconds (does this work?)
  PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 1500);
  PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);
}

void timer_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }

  TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_PIOSC);

  TimerDisable(TIMER0_BASE, TIMER_BOTH);

  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

  // 15 MILLISECOND DELAY
  TimerLoadSet(TIMER0_BASE, TIMER_A, 0x3A980);

  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER0_BASE, TIMER_A, timer_isr);
}

void main(void)
{
  SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

  gpio_init();
  timer_init();
  pwm_init();

  // ENABLE INTERRUPTS
  PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
  GPIOIntEnable(GPIO_PORTF_BASE, sw1 | sw2);

  while(1)
    {
      // loop forever
    }
}
