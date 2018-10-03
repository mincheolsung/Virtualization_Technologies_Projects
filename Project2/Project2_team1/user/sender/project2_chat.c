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
	if (!buf)
	{
		printf("buf failed\n");
		return 0;
	}
	/* change the I/O privilege level */
	if (iopl(3))
	{
		printf("failed to iopl, errno: %d\n", errno);
		goto out3;
	}
	/* open the char dev of module */	
	int fd = open("/dev/project2", O_RDWR);
	if (fd < 0)
	{
		printf("failed to open\n");
		goto out3;
	}
	/* map the shared memory */
	shared_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED) 
	{
		printf("mmap failed");
		goto out2;
	}
	/* register the signal handler */
	if (signal(SIGUSR1, catch_function) == SIG_ERR) {
		printf("An error occurred while setting a signal handler.\n");
		goto out1;
	}

	printf("type \"exit\" for termination\n");	
	while(1)
	{
		printf(">>");
		fgets(buf, BUFFER_SIZE, stdin);

		if (strcmp(buf, "exit\n") == 0)
			break;
		
		/* write data to the shared memory */
		strcpy(shared_mem, buf);
		/* send interrupt */
		outl(0, BASEPORT);
	}

out1:
	munmap(shared_mem, PAGE_SIZE);
out2:
	close(fd);
out3:
	free(buf);

	return 0;
}
