#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

static void child_proc(int *fd)
{
	char rev[16];
	char *send = "hello parent";

	close(fd[1]);
	for (;;) {
		int n = read(fd[0], rev, sizeof(rev));
		rev[n] = '\0';
		printf("child: %s\n", rev);

		n = write(fd[0], send, strlen(send));
		printf("child: writen %d bytes\n", n);

		sleep(1);
	}
}

static void parent_proc(int *fd)
{
	char rev[16];
	char *send = "hello child";

	close(fd[0]);
	for (;;) {
		int n = write(fd[1], send, strlen(send));
		printf("parent: writen %d bytes\n", n);

		n = read(fd[1], rev, sizeof(rev));
		rev[n] = '\0';
		printf("parent: %s\n", rev);

		sleep(1);
	}
}

int main(int argc, char **argv)
{
	int fd[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
		perror("socketpair");
		return -1;
	}

	pid_t pid = fork();

	if (pid < 0) {
		perror("fork");
		return -1;
	} else if (pid == 0) {	/* child */
		child_proc(fd);
	} else {	/* parent */
		parent_proc(fd);
	}

	return 0;
}
