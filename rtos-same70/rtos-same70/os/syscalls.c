/*
 * syscalls.c
 *
 * Created: 17-Aug-23 10:49:26 AM
 *  Author: Sulav Lal Shrestha
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "task.h"

void trigger_pendsv(void);

extern tcb_t *g_current_task, *g_next_task;
extern tcb_t g_tcbs[4];

void k_find_next_task(void)
{
    int32_t t_current_tid = -1;
    int32_t t_next_tid = -1;

    if (g_current_task == NULL)
    {
        t_current_tid = 0;
    }
    else
    {
        t_current_tid = g_current_task->task_id;
    }

    for (int32_t task_id = (t_current_tid + 1) % 4; task_id < 4 && task_id != g_current_task->task_id; task_id = (task_id + 1) % 4)
    {
        if (task_id == 0)
        {
            continue;
        }

        if (g_tcbs[task_id].is_free == false)
        {
            g_next_task = &g_tcbs[task_id];
            t_next_tid = g_next_task->task_id;
            break;
        }
    }

    // scheduler could not find the next task to run
    // so, run the null task
    if (t_next_tid == -1)
    {
        g_next_task = &g_tcbs[0];
    }
}
void k_run_scheduler(void)
{
    k_find_next_task();
    if (g_next_task != g_current_task)
    {
        trigger_pendsv();
    }
}

void k_task_yield(void)
{
    k_find_next_task();
    if (g_next_task != g_current_task)
    {
        trigger_pendsv();
    }
}
