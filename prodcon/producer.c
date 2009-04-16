#include "prodcon.h"

int rnd(int max) {
	return (int)(random() % max);
}

int main(void) 
{
  int f, l, rc, n = 0;

  /* open file for writing */
  f = open(FIFO, O_WRONLY | O_NONBLOCK);
  if (f < 0) {
	  if (errno == ENXIO) {
		  printf("Waiting for a consumer...\n");
		  do {
			  sleep(1);
			  f = open(FIFO, O_WRONLY | O_NONBLOCK);
		  } while (f < 0);
	  } else {
		  printf("Couldn't open FIFO (%d)\n", errno);
		  printf("Creating FIFO %s\n", FIFO);
		  f = mkfifo(FIFO, FIFO_PERMS);
		  if (f < 0) {
			  printf("Couldn't create FIFO %s (%d)\n", FIFO, errno);
			  return -1;
		  }
		  f = open(FIFO, O_WRONLY | O_NONBLOCK);
		  if (f < 0) {
			  printf("Couldn't open FIFO %s (%d)\n", FIFO, errno);
			  return -1;
		  }
	  }
  }
  /* write integers from 0 to MAX_RND at random intervals until killed */
  while (1) {
	  sem_down(1); /* Wait until there is room, then take it */
	  rc = write(f, &n, sizeof(int));
	  printf("Producing %d\n", n);
	  sem_up(0);   /* Add 1 to the ready semaphore */
	  /* l = number of seconds to sleep until doing work */
	  if (n % DURATION_LENGTH == 0)
		  l = rnd(DURATION_MAX + 1 - DURATION_MIN) + DURATION_MIN;
	  sleep(l);
	  n++;
  }
  return 0;
}
