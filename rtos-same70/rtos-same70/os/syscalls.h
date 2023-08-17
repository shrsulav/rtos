/*
 * syscalls.h
 *
 * Created: 17-Aug-23 10:49:00 AM
 *  Author: heisenberg
 */ 


#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <stdint.h>

void k_run_scheduler(void);
void k_task_yield(void);
int32_t k_task_create(void (*fn_task_entry)(void));

#endif /* SYSCALLS_H_ */