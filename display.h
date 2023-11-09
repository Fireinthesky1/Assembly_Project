// DISPLAY HEADER FILE

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

extern volatile uint8_t number_to_display;
extern volatile uint8_t digit_to_display;

uint8_t control_converter(void);
uint8_t display_converter(void);
void display_interrupt_handler(void);
