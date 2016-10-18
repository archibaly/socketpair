#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_PROCESSES	4

typedef struct {
	pid_t pid;
	int channel[2];
} proc_t;

static void child_proc(proc_t *proc)
{
	int fd = -1;
	char rev[16];

	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (proc[i].pid == getpid()) {
			fd = proc[i].channel[1];
			break;
		}
	}

	if (fd == -1)
		exit(-1);

	for (;;) {
		int n = read(fd, rev, sizeof(rev));
		rev[n] = '\0';
		printf("child(%d): %s\n", getpid(), rev);
	}
}

static void parent_proc(proc_t *proc)
{
	int i, n;
	char *send = "hello child";

	for (;;) {
		for (i = 0; i < MAX_PROCESSES; i++) {
			n = write(proc[i].channel[0], send, strlen(send));
			printf("parent: writen %d bytes\n", n);
		}
		sleep(1);
	}
}

void init_proc(proc_t *proc, int size)
{
	for (int i; i < size; i++) {
		proc[i].pid = -1;
		proc[i].channel[0] = -1;
		proc[i].channel[1] = -1;
	}
}


int main(int argc, char **argv)
{
	int i;
	proc_t proc[MAX_PROCESSES];

	init_proc(proc, MAX_PROCESSES);

	for (i = 0; i < MAX_PROCESSES; i++) {
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, proc[i].channel) < 0) {
			perror("socketpair");
			exit(-1);
		}

		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(-1);
		} else if (pid == 0) {
			proc[i].pid = getpid();
			close(proc[i].channel[0]);
			proc[i].channel[0] = -1;
			child_proc(proc);
		} else {
			proc[i].pid = pid;
			close(proc[i].channel[1]);
			proc[i].channel[1] = -1;
		}
	}

	parent_proc(proc);

	return 0;
}
