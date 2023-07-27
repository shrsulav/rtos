/*
 * delay.c
 *
 * Created: 26-Jul-23 2:47:33 PM
 *  Author: Sulav Lal Shrestha
 */ 

#include <stdint.h>

void delay(const uint32_t delay_count) 
{
    for(uint32_t counter = 0; counter < delay_count; counter++){}
}