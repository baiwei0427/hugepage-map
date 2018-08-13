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

//Set values for each byte in memory range [addr, addr + len)
static void write_bytes(char *addr, unsigned long len);
static int check_bytes(char *addr, unsigned long len);
static void print_bytes(char *addr);

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

	print_bytes(addr);
	write_bytes(addr, LENGTH);
        
        print_bytes(addr);
	ret = check_bytes(addr, LENGTH);

	munmap(addr, LENGTH);
	close(fd);
	unlink(FILE_NAME);

	return ret;
}

static void write_bytes(char *addr, unsigned long len)
{
	unsigned long i;

	for (i = 0; i < len; i++) {
		*(addr + i) = (char)i;
        }
}

static int check_bytes(char *addr, unsigned long len)
{
	unsigned long i;

	for (i = 0; i < len; i++) {	
                if (*(addr + i) != (char)i) {
			printf("Mismatch at %lu\n", i);
			return 1;
		}
        }

	return 0;
}

static void print_bytes(char *addr)
{
	printf("First hex is %x\n", *((unsigned int *)addr));
}