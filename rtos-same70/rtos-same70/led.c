/*
 * led.c
 *
 * Created: 26-Jul-23 2:39:33 PM
 *  Author: Sulav Lal Shrestha
 */ 
#include "atmel_start_pins.h"

void set_led_on(void)
{
    gpio_set_pin_level(LED0, true);
}

void set_led_off(void)
{
    gpio_set_pin_level(LED0, false);
}