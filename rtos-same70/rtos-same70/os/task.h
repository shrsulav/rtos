/*
 * task.h
 *
 * Created: 26-Jul-23 4:13:35 PM
 *  Author: Sulav Lal Shrestha
 */ 


#ifndef TASK_H_
#define TASK_H_

#define NUM_TASKS_MAX           4
#define STACK_SIZE              0x100
#define INITIAL_xPSR            0x01000000 /* user process initial xPSR value */
#define TCB_NOT_AVAILABLE       -1

void init_os(void);

struct tcb
{
    uint32_t    *usp;
    uint32_t    *ksp;
    void        (*fn_entry)(void);
    uint32_t    task_id;
    bool        is_free;
};

typedef struct tcb tcb_t;

#endif /* TASK_H_ */