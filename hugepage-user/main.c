#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define FILE_NAME "/dev/hugepages1G/random"
#define LENGTH (1024UL * 1024 * 1024)   // 1G
#define PROTECTION (PROT_READ | PROT_WRITE)
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_SHARED)

static void check_bytes(char *addr)
{
	printf("First hex is %x\n", *((unsigned int *)addr));
}

static void write_bytes(char *addr)
{
	unsigned long i;

	for (i = 0; i < LENGTH; i++) {
		*(addr + i) = (char)i;
        }
}

static int read_bytes(char *addr)
{
	unsigned long i;

	for (i = 0; i < LENGTH; i++) {	
                if (*(addr + i) != (char)i) {
			printf("Mismatch at %lu\n", i);
			return 1;
		}
        }

	return 0;
}

int main()
{
	void *addr;
	int fd, ret;

	fd = open(FILE_NAME, O_CREAT | O_RDWR, 0755);
	if (fd < 0) {
		perror("Open failed");
		exit(1);
	}

	addr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, fd, 0);
	if (addr == MAP_FAILED) {
		perror("mmap");
		unlink(FILE_NAME);
		exit(1);
	} 

	printf("Returned address is %p\n", addr);

	check_bytes(addr);
	write_bytes(addr);
        
        check_bytes(addr);
	ret = read_bytes(addr);

	munmap(addr, LENGTH);
	close(fd);
	unlink(FILE_NAME);

	return ret;
}