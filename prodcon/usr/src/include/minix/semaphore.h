#include <lib.h>
#include <unistd.h>
#include <minix/com.h>
#include <minix/callnr.h>

#define SEM_DEBUG 0
#define PM PM_PROC_NR
#define NUM_SEMAPHORES 10
#define MAX_BLOCKED 2048

PUBLIC int sem_up(int sem_num);
PUBLIC int sem_down(int sem_num);
PUBLIC int sem_init(int sem_num, int value);
PUBLIC int sem_status(int sem_num, int *value, int *num_blocked);
