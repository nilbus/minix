#include "prodcon.h"

void main(void) {
	sem_init(0, 0);  /* block when empty */
	sem_init(1, 1); /* block when full  */
	remove(FIFO);
	if (mkfifo(FIFO, FIFO_PERMS) == 0)
		printf("Initialized.\n");
	else
		printf("Error initializing FIFO buffer (%d).\n", errno);
}
