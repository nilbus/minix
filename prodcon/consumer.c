#include "prodcon.h"

int rnd(int max) {
	return (int)(random() % max);
}

int main(void) 
{
  int f, l, rc, n = 0;

  /* open file for reading */
  f = open(FIFO, O_RDONLY | O_NONBLOCK);
  if (f < 0) {
	  printf("Couldn't open FIFO %s (%d)\n", FIFO, errno);
	  return -1;
  }
  /* output integers from FIFO */
  while (1) {
	  sem_down(0); /* wait until something is there to read */
	  rc = read(f, &n, sizeof(int));
	  if (rc != sizeof(int)) {
		  printf("Failed to read from FIFO (%d)\n");
		  return -1;
	  }
	  sem_up(1);   /* increase the amount of free space */
	  printf("Consumed %d\n", n);
	  /* l = number of seconds to sleep until doing work */
	  if (n % DURATION_LENGTH == 0)
		  l = rnd(DURATION_MAX + 1 - DURATION_MIN) + DURATION_MIN;
	  sleep(l);
	  n++;
  }
  return 0;
}
