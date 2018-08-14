#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define DRIVER_FILE "/dev/hugepage-driver"
#define MEM_FILE "/dev/hugepages1G/random"
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
	int mem_fd, driver_fd, ret;
	int i;

	driver_fd = open(DRIVER_FILE, O_RDWR);
	if (driver_fd < 0) {
		perror("Fail to open driver file");
		exit(1);		
	}

	mem_fd = open(MEM_FILE, O_CREAT | O_RDWR, 0755);
	if (mem_fd < 0) {
		perror("Fail to open hugepage memory file");
		exit(1);
	}

	addr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, mem_fd, 0);
	if (addr == MAP_FAILED) {
		perror("mmap");
		unlink(MEM_FILE);
		exit(1);
	} 

	printf("Returned address is %p\n", addr);

	print_bytes(addr);

	i = 0;
	while (i++ < 10) {
		// Communicate with the driver
		write(driver_fd, addr, 1);
		print_bytes(addr);
	}

	/*write_bytes(addr, LENGTH);
        print_bytes(addr);
	ret = check_bytes(addr, LENGTH);*/
	munmap(addr, LENGTH);
	close(mem_fd);
	unlink(MEM_FILE);
	close(driver_fd);

	return 0;
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