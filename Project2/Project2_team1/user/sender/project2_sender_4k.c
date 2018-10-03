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
#include <time.h>

#define BASEPORT 0x700
#define PAGE_SIZE (1 << 12)
#define BUFFER_SIZE (1 << 12)

struct timeval tv;
void *shared_mem;
int out;
unsigned long long start;
unsigned long long stop;

static inline unsigned long long get_real_time(void)
{
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &time);

	return time.tv_sec * 1000000000L + time.tv_nsec;
}

static void catch_function(int signo) 
{
	/* time when notification arrives */
	stop = get_real_time();
	/* elapsed time */
	printf("time:%llu\n", stop - start);
	
	out = 0;
}

int main(int argc, char *argv[])
{
	char *buf;
	char c = 'A';
	if (argc > 1)
		memcpy(&c, argv[1], 1);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (!buf)
	{
		printf("buf failed\n");
		return 0;
	}
	/* init buffer */
	memset(buf, c, BUFFER_SIZE);
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
	shared_mem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED) 
	{
		printf("mmap failed\n");
		goto out2;
	}
	/* register the signal handler */
	if (signal(SIGUSR1, catch_function) == SIG_ERR) {
		printf("An error occurred while setting a signal handler.\n");
		goto out1;
	}

	out = 1;
	/* start timestamp */
	start = get_real_time();
	/* write data to the shared memory */
	memcpy(shared_mem, buf, BUFFER_SIZE);
	/* send interrupt */
	outl(0, BASEPORT);
	/* waiting for interrupt */
	while(out){}

out1:
	munmap(shared_mem, BUFFER_SIZE);
out2:
	close(fd);
out3:
	free(buf);
	return 0;
}
