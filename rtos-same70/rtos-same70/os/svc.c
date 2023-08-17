/*
 * svc.c
 *
 * Created: 17-Aug-23 10:44:41 AM
 *  Author: Sulav Lal Shrestha
 */

#include <stdint.h>

__attribute__ ((noinline)) void run_scheduler(void)
{
    __asm volatile("svc 0x00");
}

__attribute__ ((noinline)) void task_yield(void)
{
    __asm volatile("svc 0x01");
}

__attribute__ ((noinline)) int32_t task_create(void)
{
    __asm volatile("svc 0x02");
}