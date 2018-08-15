#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

//#define DRIVER_FILE "/dev/hugepage-driver"
#define HUGEPAGE_FILE "/dev/hugepages1G/random"
#define LENGTH (1024UL * 1024 * 1)   // 1M
#define PROTECTION (PROT_READ | PROT_WRITE)
#define ADDR (void *)(0x0UL)
#define FLAGS (MAP_SHARED)
#define BAD_PHYS_ADDR 0
#define PFN_MASK_SIZE 8

// Set values for each byte in memory range [addr, addr + len)
static void write_bytes(char *addr, unsigned long len);
static int check_bytes(char *addr, unsigned long len);
static void print_bytes(char *addr);
// Get physical address of any mapped virtual address in the current process
uint64_t mem_virt2phy(const void *virtaddr);

int main()
{
	void *addr;
	int hugepage_fd, ret;

	hugepage_fd = open(HUGEPAGE_FILE, O_CREAT | O_RDWR, 0755);
	if (hugepage_fd < 0) {
		perror("Fail to open hugepage memory file");
		exit(1);
	}

	addr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, hugepage_fd, 0);
	if (addr == MAP_FAILED) {
		perror("mmap");
		unlink(HUGEPAGE_FILE);
		exit(1);
	} 

	printf("Virtual address is %p\n", addr);
	printf("Physical address is %llu\n", mem_virt2phy(addr));

	print_bytes(addr);
	//write_bytes(addr, LENGTH);
        print_bytes(addr);
	//ret = check_bytes(addr, LENGTH);

	munmap(addr, LENGTH);
	close(hugepage_fd);
	unlink(HUGEPAGE_FILE);

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

uint64_t mem_virt2phy(const void *virtaddr)
{
	int fd, retval;
	uint64_t page, physaddr;
	unsigned long virt_pfn;	// virtual page frame number
	int page_size;
	off_t offset;

	/* standard page size */
	page_size = getpagesize();

	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "%s(): cannot open /proc/self/pagemap: %s\n", __func__, strerror(errno));
		return BAD_PHYS_ADDR;
	}

	virt_pfn = (unsigned long)virtaddr / page_size;
	printf("Virtual page frame number is %llu\n", virt_pfn);

	offset = sizeof(uint64_t) * virt_pfn;
	if (lseek(fd, offset, SEEK_SET) == (off_t) - 1) {
		fprintf(stderr, "%s(): seek error in /proc/self/pagemap: %s\n", __func__, strerror(errno));
		close(fd);
		return BAD_PHYS_ADDR;
	}

	retval = read(fd, &page, PFN_MASK_SIZE);
	close(fd);
	if (retval < 0) {
		fprintf(stderr, "%s(): cannot read /proc/self/pagemap: %s\n", __func__, strerror(errno));
		return BAD_PHYS_ADDR;
	} else if (retval != PFN_MASK_SIZE) {
		fprintf(stderr, "%s(): read %d bytes from /proc/self/pagemap but expected %d:\n", 
		        __func__, retval, PFN_MASK_SIZE);
		return BAD_PHYS_ADDR;
	}

	printf("%llu\n", page);

	/*
	 * the pfn (page frame number) are bits 0-54 (see
	 * pagemap.txt in linux Documentation)
	 */
	if ((page & 0x7fffffffffffffULL) == 0) {
		fprintf(stderr, "Zero page frame number\n");
		return BAD_PHYS_ADDR;
	}

	physaddr = ((page & 0x7fffffffffffffULL) * page_size) + ((unsigned long)virtaddr % page_size);

	return physaddr;
}
