/*
 * syscalls.h
 *
 * Created: 17-Aug-23 10:49:00 AM
 *  Author: heisenberg
 */ 


#ifndef SYSCALLS_H_
#define SYSCALLS_H_

void k_run_scheduler(void);
void k_task_yield(void);
void k_task_create(int8_t (*fn_task_entry)(void));

#endif /* SYSCALLS_H_ */