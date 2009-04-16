#include <minix/semaphore.h>
#include <stdio.h>

PUBLIC int sem_up(int sem_num) {
	message m;
	m.m1_i1 = sem_num;
	return(_syscall(PM, SEM_UP, &m));
}

PUBLIC int sem_down(int sem_num) {
	message m;
	m.m1_i1 = sem_num;
	return(_syscall(PM, SEM_DOWN, &m));
}

PUBLIC int sem_init(int sem_num, int value) {
	message m;
	m.m1_i1 = sem_num;
	m.m1_i2 = value;
	return(_syscall(PM, SEM_INIT, &m));
}

PUBLIC int sem_status(int sem_num, int *value, int *num_blocked) {
	message m;
	int retval;
	m.m1_i1 = sem_num;
	retval = _syscall(PM, SEM_STATUS, &m);
	*value = retval >> 16;
	*num_blocked = retval & 65535;
	return 0;
}
