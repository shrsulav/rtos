/*
 * task.c
 *
 * Created: 26-Jul-23 4:12:52 PM
 *  Author: Sulav Lal Shrestha
 */ 

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NUM_TASKS_MAX       4
#define STACK_SIZE          0x100
#define INITIAL_xPSR        0x01000000        /* user process initial xPSR value */
#define TCB_NOT_AVAILABLE   -1

struct tcb
{
    uint32_t    *stack_ptr;
    struct tcb  *next_task;
    void        (*fn_entry)(void);
    uint32_t    task_id;
    bool        is_free;
};

typedef struct tcb tcb_t;

uint32_t g_stacks[NUM_TASKS_MAX][STACK_SIZE];
tcb_t g_tcbs[NUM_TASKS_MAX];

tcb_t* g_current_task;
tcb_t* g_next_task;

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

    __asm("LDR     R3, [R2,#4]");       // R3 now points to the TCB of next task

    __asm("STR     R3, [R1]");            // g_current_task is not updated

    __asm("LDR     R4, [R3]");          // R4 points to the stack of updated current stack
    __asm("MOV     SP, R4");            // SP now holds the pointer to stack of the current task
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
    for(uint8_t task_counter=0; task_counter < NUM_TASKS_MAX; task_counter++)
    {
        memset(g_stacks[task_counter], 0xAA, STACK_SIZE);
    }
}
void init_tcbs(void)
{
    for(uint8_t task_counter=0; task_counter < NUM_TASKS_MAX; task_counter++)
    {
         g_tcbs[task_counter].task_id = task_counter;
         g_tcbs[task_counter].fn_entry = NULL;
         g_tcbs[task_counter].is_free = true;
         g_tcbs[task_counter].stack_ptr = g_stacks[task_counter] + STACK_SIZE;
         g_tcbs[task_counter].next_task = NULL;
    }
}

int8_t create_task(void* fn_entry)
{
    // Disable interrupts
    
    uint8_t free_task_id = NUM_TASKS_MAX+1;
    bool is_tcb_free = false;
    
    for(uint8_t task_counter = 0; task_counter<NUM_TASKS_MAX; task_counter++)
    {
        if(g_tcbs[task_counter].is_free == true)
        {
            free_task_id = task_counter;
            is_tcb_free = true;
            break;
        }
    }
    
    if(is_tcb_free == false)
    {
        return TCB_NOT_AVAILABLE;
    }
    
    g_tcbs[free_task_id].fn_entry = fn_entry;
    g_tcbs[free_task_id].is_free = false;
    g_tcbs[free_task_id].stack_ptr = g_stacks[free_task_id] + STACK_SIZE;
    
        /// At this point the processor has already pushed PSR, PC, LR, R12, R3, R2, R1 and R0
        /// onto the stack. We need to push the rest(i.e R4, R5, R6, R7, R8, R9, R10 & R11) to save the
        /// context of the current task.
    *(--(g_tcbs[free_task_id].stack_ptr)) = INITIAL_xPSR; // PSR
    *(--(g_tcbs[free_task_id].stack_ptr)) = fn_entry; // PC
    
    for(uint8_t i=0; i<6; i++)
    {
        *(--(g_tcbs[free_task_id].stack_ptr)) = 0xDEADAAA0 + i;
    }
    
    for(uint8_t i=0; i<8; i++)
    {
        *(--(g_tcbs[free_task_id].stack_ptr)) = 0xDEADBBB0 + i;
    }
    
    
    // Enable interrupts
    
    return free_task_id;
}

void fn_null_task(void)
{
    while(1);
}

void init_os(void)
{
    init_stacks();
    init_tcbs();
    if(create_task(fn_null_task) != 0)
    {
        while(1); // some error
    }
    g_current_task = &g_tcbs[0];
}


    __attribute__((naked)) void run_scheduler(void)
    {
        /// R0 contains the address of currentPt
        __asm("LDR     R0, =g_current_task");
        /// R2 contains the address in currentPt(value of currentPt)
        __asm("LDR     R2, [R0]");
        /// Load the SP reg with the stacked SP value
        __asm("LDR     R4, [R2]");
        __asm("MOV     SP, R4");
        /// Pop registers R8-R11(user saved context)
        __asm("POP     {R4-R7}");
        __asm("MOV     R8, R4");
        __asm("MOV     R9, R5");
        __asm("MOV     R10, R6");
        __asm("MOV     R11, R7");
        /// Pop registers R4-R7(user saved context)
        __asm("POP     {R4-R7}");
        ///  Start popping the stacked exception frame.
        __asm("POP     {R0-R3}");
        __asm("POP     {R4}");
        __asm("MOV     R12, R4");
        /// Skip the saved LR
        __asm("ADD     SP,SP,#4");
        /// POP the saved PC into LR via R4, We do this to jump into the
        /// first task when we execute the branch instruction to exit this routine.
        __asm("POP     {R4}");
        __asm("MOV     LR, R4");
        __asm("ADD     SP,SP,#4");
        /// Enable interrupts
        __asm("CPSIE   I ");
        __asm("BX      LR");
    }