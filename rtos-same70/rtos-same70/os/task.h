/*
 * task.h
 *
 * Created: 26-Jul-23 4:13:35 PM
 *  Author: Sulav Lal Shrestha
 */ 


#ifndef TASK_H_
#define TASK_H_

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