// James Hicks Lab 5, October 5 2023

// portD pins 0 - 3 determine the value to display
// portE pins 1 - 3 detmine which digits will dispay a value
// The display module will show the current value of a count that starts at 0
// without unnecessary leading zeros, maintained in a uint8_t variable
// Use a general purpose timer with an interrupt to switch between digits
// to produce a steady display
// separate display code into its own .c/.h files

// initially, once per second the value of the count will change by 1

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
