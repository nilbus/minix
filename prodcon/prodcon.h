#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <minix/semaphore.h>
#include <stdlib.h>

#define FIFO "/tmp/prodcon.fifo"
#define FIFO_PERMS 0660
#define MAX_RND 1000
#define DURATION_LENGTH 5 /* each duration happens X times in a row */
#define DURATION_MIN 0 /* seconds */
#define DURATION_MAX 0 /* seconds */
