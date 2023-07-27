/*
 * task.h
 *
 * Created: 26-Jul-23 4:13:35 PM
 *  Author: Sulav Lal Shrestha
 */ 


#ifndef TASK_H_
#define TASK_H_

uint8_t create_task(void* fn_entry);
void init_os(void);

struct tcb
{
    uint32_t    *stack_ptr;
    struct tcb  *next_task;
    void        (*fn_entry)(void);
    uint32_t    task_id;
    bool        is_free;
};

typedef struct tcb tcb_t;

#endif /* TASK_H_ */