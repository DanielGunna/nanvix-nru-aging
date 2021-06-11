/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>
/* Priority range from 0 to 80 */
#define MAX_PRIORITY 0
#define MIN_PRIORITY 80
/* Increment/decrement constant */
#define PRIORITY_MODIFIER 5

/**
 * @brief Schedules a process to execution.
 * 
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/**
 * @brief Resumes a process.
 * 
 * @param proc Process to be resumed.
 * 
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{	
	/* Resume only if process has stopped. */
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/**
 * @brief Increase the process priority.
 * 
 * @param proc Process to increase the priority.
 * 
 * @note The priority will increase up to MAX_PRIORITY
 */
PRIVATE void increasePriority(struct process *proc){
	if(proc->priority + PRIORITY_MODIFIER > MAX_PRIORITY)
		proc->priority -= PRIORITY_MODIFIER;
	
}

/**
 * @brief Decrease the process priority.
 * 
 * @param proc Process to decrease the priority.
 * 
 * @note The priority will decrease down to MIN_PRIORITY
 */
PRIVATE void decreasePriority(struct process *proc){
	if(proc->priority - PRIORITY_MODIFIER < MIN_PRIORITY)
		proc->priority += PRIORITY_MODIFIER;
	
}

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
	struct process *p;    /* Working process.     */
	struct process *next; /* Next process to run. */

	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
		sched(curr_proc);

	/* Remember this process. */
	last_proc = curr_proc;

	/* Check alarm. */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip invalid processes. */
		if (!IS_VALID(p))
			continue;
		
		/* Alarm has expired. */
		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}

	/* Choose a process to run next. */
	next = IDLE;
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip non-ready process. */
		if (p->state != PROC_READY)
			continue;
		
		/*
		 * Process with higher
		 * priority found.
		 */
		if (p->priority < next->priority)
		{
			increasePriority(next);
			next = p;
			
		}
			
		/*
		 * Increment priority of process.
		 */
		else
			increasePriority(p);
	}
	
	/* Switch to next process. */
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;
	switch_to(next);
	
	/* Decrease the priority of current process */
	decreasePriority(next);

}
