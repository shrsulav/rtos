/*
 * task.c
 *
 * Created: 26-Jul-23 4:12:52 PM
 *  Author: Sulav Lal Shrestha
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "task.h"

uint32_t g_ustacks[NUM_TASKS_MAX][STACK_SIZE];
uint32_t g_kstacks[NUM_TASKS_MAX][STACK_SIZE];
tcb_t g_tcbs[NUM_TASKS_MAX];

tcb_t *g_current_task;
tcb_t *g_next_task;

__attribute__((naked)) void SysTick_Handler(void)
{
    /// STEP 1 - SAVE THE CURRENT TASK CONTEXT

    /// At this point the processor has already pushed PSR, PC, LR, R12, R3, R2, R1 and R0
    /// onto the stack. We need to push the rest(i.e R4, R5, R6, R7, R8, R9, R10 & R11) to save the
    /// context of the current task.
    /// Disable interrupts
    __asm("CPSID   I");
    /// Push registers R4 to R7
    __asm("PUSH    {R4-R7}");
    /// Push registers R8-R11
    __asm("MOV     R4, R8");
    __asm("MOV     R5, R9");
    __asm("MOV     R6, R10");
    __asm("MOV     R7, R11");
    __asm("PUSH    {R4-R7}");
    /// Load R0 with the address of pCurntTcb
    __asm("LDR     R0, =g_current_task");
    /// Load R1 with the content of pCurntTcb(i.e post this, R1 will contain the address of current TCB).
    __asm("LDR     R1, [R0]");
    /// Move the SP value to R4
    __asm("MOV     R4, SP");
    /// Store the value of the stack pointer(copied in R4) to the current tasks "stackPt" element in its TCB.
    /// This marks an end to saving the context of the current task.
    __asm("STR     R4, [R1]");

    /// STEP 2: LOAD THE NEW TASK CONTEXT FROM ITS STACK TO THE CPU REGISTERS, UPDATE pCurntTcb.

    /// Load the address of the next task TCB onto the R1.
    __asm("LDR     R1, =g_current_task");
    /// Load the contents of the next tasks stack pointer to pCurntTcb, equivalent to pointing pCurntTcb to
    /// the newer tasks TCB. Remember R1 contains the address of pCurntTcb.
    __asm("LDR     R2, [R1]");

    __asm("LDR     R3, [R2,#4]"); // R3 now points to the TCB of next task

    __asm("STR     R3, [R1]"); // g_current_task is not updated

    __asm("LDR     R4, [R3]"); // R4 points to the stack of updated current stack
    __asm("MOV     SP, R4");   // SP now holds the pointer to stack of the current task
    /// Pop registers R8-R11
    __asm("POP     {R4-R7}");
    __asm("MOV     R8, R4");
    __asm("MOV     R9, R5");
    __asm("MOV     R10, R6");
    __asm("MOV     R11, R7");
    /// Pop registers R4-R7
    __asm("POP     {R4-R7}");
    __asm("CPSIE   I ");
    __asm("BX      LR");
}

void init_stacks(void)
{
    for (uint8_t task_counter = 0; task_counter < NUM_TASKS_MAX; task_counter++)
    {
        memset(g_ustacks[task_counter], 0xAA, STACK_SIZE);
        memset(g_kstacks[task_counter], 0xBB, STACK_SIZE);
    }
}
void init_tcbs(void)
{
    for (uint8_t task_counter = 0; task_counter < NUM_TASKS_MAX; task_counter++)
    {
        g_tcbs[task_counter].task_id = task_counter;
        g_tcbs[task_counter].fn_entry = NULL;
        g_tcbs[task_counter].is_free = true;
        g_tcbs[task_counter].usp = g_ustacks[task_counter] + STACK_SIZE;
        g_tcbs[task_counter].ksp = g_kstacks[task_counter] + STACK_SIZE;
    }
}

int8_t k_task_create(void (*fn_entry)(void))
{
    // Disable interrupts

    uint8_t free_task_id = NUM_TASKS_MAX + 1;
    bool is_tcb_free = false;

    for (uint8_t task_counter = 0; task_counter < NUM_TASKS_MAX; task_counter++)
    {
        if (g_tcbs[task_counter].is_free == true)
        {
            free_task_id = task_counter;
            is_tcb_free = true;
            break;
        }
    }

    if (is_tcb_free == false)
    {
        return TCB_NOT_AVAILABLE;
    }

    g_tcbs[free_task_id].fn_entry = fn_entry;
    g_tcbs[free_task_id].is_free = false;
    g_tcbs[free_task_id].usp = g_ustacks[free_task_id] + STACK_SIZE;
    g_tcbs[free_task_id].ksp = g_kstacks[free_task_id] + STACK_SIZE;

    /// At this point the processor has already pushed PSR, PC, LR, R12, R3, R2, R1 and R0
    /// onto the stack. We need to push the rest(i.e R4, R5, R6, R7, R8, R9, R10 & R11) to save the
    /// context of the current task.
    *(--(g_tcbs[free_task_id].usp)) = INITIAL_xPSR; // PSR
    *(--(g_tcbs[free_task_id].usp)) = fn_entry;     // PC

    for (uint8_t i = 0; i < 6; i++)
    {
        *(--(g_tcbs[free_task_id].usp)) = 0xDEADAAA0 + i;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        *(--(g_tcbs[free_task_id].ksp)) = 0xDEADBBB0 + i;
    }

    // Enable interrupts

    return free_task_id;
}

void fn_null_task(void)
{
    while (1)
        ;
}

void init_os(void)
{
    init_stacks();
    init_tcbs();
    if (k_task_create(fn_null_task) != 0)
    {
        while (1)
            ; // some error
    }
    g_current_task = &g_tcbs[0];
}

__attribute__((naked)) void PendSV_Handler(void)
{
    __asm volatile(
        "LDR R0, =g_current_task                \n"
        "LDR R1, [R0]                           \n" // R1 points to the TCB
        "CBZ R1, LOAD_CONTEXT                   \n"

        "SAVE_CONTEXT:                          \n"
        "PUSH {R4-R11}                          \n" // Push R4-R11 onto MSP
        "MRS R2, PSP                            \n" 
        "STR R2, [R1]                           \n" // update usp in the TCB
        "MRS R2, MSP                            \n" 
        "STR R2, [R1,4]                         \n" // update ksp in the TCB

        "LOAD_CONTEXT:                          \n"
        "LDR R1, =g_next_task                   \n"
        "LDR R2, [R1]                           \n" // R2 points to g_next_task
        "LDR R4, [R2]                           \n" // R4 points to g_next_task->usp
        "MSR PSP, R4                            \n"
        "LDR R4, [R2, #4]                       \n" // R4 points to g_next_task->ksp
        "MSR MSP, R4                            \n"
        "POP {R4-R11}                           \n"
        "STR R2, [R0]                           \n" // update g_current_task to be same as g_next_task
        "MVN LR, #2                             \n" // load 0xFFFFFFFD onto LR
        "BX  LR                                 \n");
}