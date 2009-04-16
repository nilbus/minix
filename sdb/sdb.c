#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <ctype.h>
#include "mdb.h"

#define MAX_CMD_LEN 1024

char regs[] = "fs gs ds es di si bp    bx dx cx ax    ip cs ps sp ss";
int pid = 0;

int print_usage(char *progname) {
	printf("Usage: %s command [args...]\n");
	return -1;
}

void print_commands(void) {
	printf("[s]tep   [f]inish   [p]rint <n>th register   [h]elp   [q]uit\n");
	return;
}

int run(int argc, char *argv[]) {
	int procid;
	if ((procid = fork()) == 0) {
		ptrace(T_OK, 0, 0, 0); 
		execvp(argv[0], argv);
		printf("Couldn't run %s.\n", argv[0]);
		return -1;
	}
	return procid;
}

long get_reg(int pid, int k) {
	long off;
	long val;
	int reg_size;

	/* Calculate size of register */
	reg_size = (k < N_REG16 * 2) ? 2 : sizeof(unsigned);

	/* Adjust offset */
	off = k - (k & (sizeof(long) - 1));

	val = ptrace(T_GETUSER, pid, off, 0L);

	if (k & (sizeof(long) - 1))
		val >>= BITSIZE(reg_size);
	else
		val &= MASK(reg_size);
	return val;
}

long reg_addr(char *s) {
	long val;
	char *t;
	char *send;
	char q[3];

	if (*s == ' ') {
		printf("Invalid syntax\n");
		return -1;
	}
	q[0] = tolower(*s);
	q[1] = tolower(*++s);
	q[2] = '\0';

	t = regs;
	send = regs + sizeof(regs);
	while (t < send) {
		if (strncmp(q, t, 2) == 0) {
			val = (long) (t - regs);
			val /= 3L;
			if (val < N_REG16 - 1)
				val = val * 2;
			else
				val = (N_REG16 - 1) * 2 +
					(val - N_REG16 + 1) * sizeof(unsigned);
			return val;
		}
		t += 3;
	}
	printf("Unknown register: %s\n", q);
	return -1;
}

void readcommands(int pid) {
	char cmd[MAX_CMD_LEN];
	int reg;

	printf("> ");
	while (gets(cmd)) { /* TODO: While the child is still alive */
		if (0 == strlen(cmd) || 0 == strncmp(cmd, "s", MAX_CMD_LEN)) {
			printf("stepping 1 instruction\n");
#ifdef SDB_DEBUG
			for (reg = 0; reg < 30000; reg++) {
#endif
				ptrace(T_STEP, pid, 0, 0);
#ifdef SDB_DEBUG
				printf("Register %s(%d): %ld\n",
						"ip", reg_addr("ip"), get_reg(pid, reg_addr("ip")));
			ptrace(T_STEP, pid, 0, 0);
			}
#endif
		} else if (0 == strncmp(cmd, "f", MAX_CMD_LEN)) {
			printf("running to completion\n");
			ptrace(T_RESUME, pid, 0, 0);
			waitpid(pid, &reg, 0); /* ignore the reg return value */
			break;
		} else if (0 == strncmp(cmd, "p ", 2)) {
			reg = atoi(cmd + 2); /* skip the first 2 characters */
			if (strlen(cmd + 2) == 0 || reg == 0 && cmd[2] != '0' || reg < 0) {
				/* Try searching by register name */
				reg = reg_addr(cmd + 2);
				if (reg > -1)
					printf("Register %s(%d): %ld\n",
							cmd + 2, reg, get_reg(pid, reg));
			} else {
				printf("Register %d: %ld\n", reg, get_reg(pid, reg));
			}
		} else if (0 == strncmp(cmd, "p", MAX_CMD_LEN)) {
			printf("must specify register number\n");
		} else if (0 == strncmp(cmd, "q", MAX_CMD_LEN)) {
			break;
		} else if (0 == strncmp(cmd, "?", MAX_CMD_LEN) ||
				   0 == strncmp(cmd, "h", MAX_CMD_LEN)) {
			print_commands();
		} else {
			printf("unrecognized command.\n");
		}
		printf("> ");
	}
	ptrace(T_EXIT, pid, 0, 0);
}

int main(int argc, char *argv[]) {
	char *command = malloc(sizeof(char) * MAX_CMD_LEN);
	char **execargv;
	int execargc, i, procid;

	if (argc < 2)
		return print_usage(argv[0]);

	strncpy(command, argv[1], MAX_CMD_LEN);

	execargv = argv + 1;
	execargc = argc - 1;

#ifdef SDB_DEBUG
	printf("Forking: ");
	for (i = 0; i < execargc; i++)
		printf("%s ", execargv[i]);
	printf("\n");
#endif

	procid = run(execargc, execargv);
	if (procid < 0) {
		return -1;
	}

	print_commands();
	readcommands(procid);
	puts("");
}
