/*
 * svcall_handler.c
 *
 * Created: 17-Aug-23 12:31:38 PM
 *  Author: Sulav Lal Shrestha
 */ 

#include "svc.h"
#include "syscalls.h"

void SVCall_Handler_Main(unsigned int *svc_args)
{
    unsigned int svc_number;

    svc_number = ((char *)svc_args[6])[-2];
    int32_t ret = -1;
    switch(svc_number)
    {
        case SVC_RUN_SCHEDULER:
            k_run_scheduler();
            break;
        case SVC_TASK_YIELD:
            k_task_yield();
            break;
        case SVC_TASK_CREATE:
            ret = k_task_create(svc_args[0]);
            break;
        default:
        break;
    }
    svc_args[0] = ret;
}

void __attribute__ (( naked )) SVCall_Handler(void)
{
    asm volatile(
    "tst lr, #4                                         \n"
    "ite eq                                             \n"
    "mrseq r0, msp                                      \n"
    "mrsne r0, psp                                      \n"
    "b %[SVCall_Handler_Main]                           \n"
    :                                                   /* no output */
    : [SVCall_Handler_Main] "i" (SVCall_Handler_Main)   /* input */
    : "r0"                                              /* clobber */
    );
}

__attribute__((optimize("O0")))
void trigger_pendsv(void) {
    volatile uint32_t *icsr = (void *)0xE000ED04;
    // Pend a PendSV exception using by writing 1 to PENDSVSET at bit 28
    *icsr = 0x1 << 28;
    // flush pipeline to ensure exception takes effect before we
    // return from this routine
    __asm("isb");
}
