#include "pm.h"
#include <signal.h>
#include "mproc.h"
#include <minix/semaphore.h>

/* semaphore values */
int sem[NUM_SEMAPHORES];
/* an array of queues of processes blocking */
int block_list[NUM_SEMAPHORES][MAX_BLOCKED];
int block_head[NUM_SEMAPHORES];
int block_tail[NUM_SEMAPHORES]; /* tail > head, except when wrapping */

int block_count(int sem_num) {
	return (block_tail[sem_num] - block_head[sem_num]) % MAX_BLOCKED;
}

/* enqueue at tail */
void block_enqueue(int sem_num, int proc_num) {
	if (block_count(sem_num) >= MAX_BLOCKED - 1)
		panic(__FILE__, 
			"Blocked queue overflow. You must have hit the wrong 'any' key.",
			sem_num);

	if (SEM_DEBUG)
		printf("Enqueue sem[%d][%d] = %d\n",
				sem_num, block_tail[sem_num], proc_num);

	block_list[sem_num][block_tail[sem_num]] = proc_num;

	block_tail[sem_num]++;
	block_tail[sem_num] %= MAX_BLOCKED;
}

/* dequeue from head */
int block_dequeue(int sem_num) {
	int proc_num;

	if (block_count(sem_num) <= 0)
		panic(__FILE__, 
			"Blocked dequeue when nothing was waiting, likely due to "
			"global warming.", sem_num);

	/* return value at head, and increment head */
	proc_num = block_list[sem_num][block_head[sem_num]];

	if (SEM_DEBUG)
		printf("Dequeue sem[%d][%d] = %d\n",
				sem_num, block_head[sem_num], proc_num);

	block_head[sem_num]++;
	block_head[sem_num] %= MAX_BLOCKED;
	return proc_num;
}

int do_sem_init() {
	int sem_num = m_in.m1_i1;
	int value = m_in.m1_i2;

	if (SEM_DEBUG)
		printf("sem_init: %d, %d\n", sem_num, value);

	if (sem_num >= NUM_SEMAPHORES || sem_num < 0 || value < 0)
		return -1;

	sem[sem_num] = value;      /* Initialize semaphore value */
	block_head[sem_num] = 0;   /* Initialize head of blocked process queue */
	block_tail[sem_num] = 0;   /* Initialize tail of blocked process queue */

	return OK;
}

int do_sem_up() {
	int sem_num = m_in.m1_i1;
	int proc_num;
	message msg;

	if (SEM_DEBUG)
		printf("sem_up: %d.\n", sem_num);

	if (sem_num >= NUM_SEMAPHORES || sem_num < 0)
		return -1;

	/* Check to see if anyone was waiting */
	if (block_count(sem_num) > 0) {
		/* Let them freeeeeeee! */
		if (SEM_DEBUG)
			printf("resuming process %d.\n",
					block_list[sem_num][block_head[sem_num]-1]);
		/* Resume the blocked process and dequeue it */
		proc_num = block_dequeue(sem_num);
		msg.m_type = OK;
		msg.m_source = proc_num;
		send(proc_num, &msg);
	} else {
		/* Increment the semaphore to make the resource available. */
		sem[sem_num]++;
	}

	if (SEM_DEBUG)
		printf("new value: %d\n", sem[sem_num]);

	return OK;
}

int do_sem_down() {
	int sem_num = m_in.m1_i1;

	if (SEM_DEBUG)
		printf("sem_down: %d\n", sem_num);

	if (sem_num >= NUM_SEMAPHORES || sem_num < 0)
		return -1;

	/* Decrement the semaphore if the resource is free */
	if (sem[sem_num] > 0) {
		sem[sem_num]--;
		if (SEM_DEBUG)
			printf("new value: %d\n", sem[sem_num]);
		return OK;
	/* otherwise give it sleeping pills until it's free */
	} else {
		if (SEM_DEBUG)
			printf("placing on queue for sem %d.\n", sem_num);

		/* Fail if already too many blocking */
		if (block_count(sem_num) >= MAX_BLOCKED - 1)
			return -1;

		/* place on this semaphore's block_list and increment its counter */
		block_enqueue(sem_num, m_in.m_source);

		if (SEM_DEBUG)
			printf("Suspending process %d.\n", m_in.m_source);

		/* suspend the process */
		return SUSPEND;
	}
	
	/* should never get here */
	panic(__FILE__, 
"Fluorescent lights are generating negative ions. If turning them off doesn't\n"
"work, take them out and put tin foil on the ends.", NO_NUM);
	return OK;
}

/* 
 * Get the status of semaphore sem_num.
 * Retrieves either its value or the number of processes blocked,
 * depending on the value of requested_value (boolean).
 */
int do_sem_status() {
	int sem_num = m_in.m1_i1;
	int retval;

	if (SEM_DEBUG)
		printf("sem_status: %d ", sem_num);

	if (sem_num >= NUM_SEMAPHORES || sem_num < 0)
		return -1;

	retval = sem[sem_num];
	retval <<= 16;
	retval |= (block_count(sem_num) & 65535);

	if (SEM_DEBUG)
		printf("returning: %d and %d.\n", retval >> 16, retval & 65535);

	return retval;
}
