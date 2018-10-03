#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/io.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>

#define BASEPORT 0x700
#define PAGE_SIZE (1 << 12)
#define BUFFER_SIZE 100

void *shared_mem;

static void catch_function(int signo) {
	printf(">>%s", (char *)shared_mem);
	printf(">>");
	fflush(stdout);
}

int main()
{
	char *buf;
	buf = (char *)malloc(BUFFER_SIZE);

	if (iopl(3))
	{
		printf("failed to iopl, errno: %d\n", errno);
		return 0;
	}
	
	int fd = open("/dev/project2", O_RDWR);
	if (fd < 0)
		return 0;

	shared_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED) 
	{
		perror("mmap");
		assert(0);
	}

	if (signal(SIGUSR1, catch_function) == SIG_ERR) {
		printf("An error occurred while setting a signal handler.\n");
		return EXIT_FAILURE;
	}

	printf("type \"exit\" for termination\n");
	while(1)
	{
		printf(">>");
		fgets(buf, BUFFER_SIZE, stdin);
		
		if (strcmp(buf,"exit\n") == 0)
			break;

		strcpy(shared_mem, buf);

		/* send interrupt to kvmtool */
		outl(0, BASEPORT);
	}

	munmap(shared_mem, PAGE_SIZE);
	free(buf);

	return 0;
}
