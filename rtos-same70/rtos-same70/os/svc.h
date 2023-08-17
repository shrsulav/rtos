/*
 * svc.h
 *
 * Created: 17-Aug-23 10:47:03 AM
 *  Author: Sulav Lal Shrestha
 */ 


#ifndef SVC_H_
#define SVC_H_

#include <stdint.h>

#define SVC_RUN_SCHEDULER   0
#define SVC_TASK_YIELD      1
#define SVC_TASK_CREATE     2

__attribute__((noinline)) void run_scheduler(void);
__attribute__((noinline)) void task_yield(void);
__attribute__((noinline)) void task_create(int8_t (*fn_task_entry)(void));

#endif /* SVC_H_ */