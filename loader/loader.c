#include "loader.h"
#include "exec_parser.h"
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static so_exec_t *exec;
static struct sigaction default_signal_action;
static int page_size;
static int fd;

struct segment_info {
	int *pages_sgn;
	int pages_num;
};

int load_segment_info(so_exec_t *seg);
int set_signal_intercept(void);
void sigsegv_handler(int signum, siginfo_t *info, void *context);

int so_init_loader(void)
{
	page_size = getpagesize();
	int ret = set_signal_intercept();

	return ret;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	fd = open(path, O_RDONLY);

	if (fd == -1)
		return -1;

	int ret = load_segment_info(exec);

	if (ret == -1)
		return -1;

	so_start_exec(exec, argv);

	ret = close(fd);
	return ret;
}

int load_segment_info(so_exec_t *seg)
{
	for (int i = 0; i < seg->segments_no; i++) {
		struct segment_info *info = malloc(sizeof(*info));

		if (info == NULL)
			return -1;

		info->pages_num = ceil(seg->segments[i].mem_size / page_size);
		info->pages_sgn = calloc(info->pages_num, sizeof(int));
		if (info->pages_sgn == NULL)
			return -1;

		seg->segments[i].data = info;
	}
	return 0;
}

int set_signal_intercept(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sigsegv_handler;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGSEGV);

	int ret = sigaction(SIGSEGV, &sa, &default_signal_action);

	return ret;
}

int find_container_segment(uintptr_t addr)
{
	for (int i = 0; i < exec->segments_no; i++) {
		if (addr >= exec->segments[i].vaddr &&
		    addr <=
			exec->segments[i].vaddr + exec->segments[i].mem_size) {
			return i;
		}
	}
	return -1;
}

int check_page_mapped(int page_idx, struct segment_info *s_info)
{
	return s_info->pages_sgn[page_idx] == 1;
}

int map_page(int page_idx, so_seg_t seg, struct segment_info *s_info)
{
	s_info->pages_sgn[page_idx] = 1;
	uintptr_t page_addr = page_idx * page_size + seg.vaddr;
	char *page_ptr = mmap((void *)page_addr, page_size, PROT_WRITE,
			      MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);

	if (page_ptr == MAP_FAILED)
		return -1;
	int bytes_size = page_size;

	if ((int)seg.file_size - page_idx * page_size < page_size)
		bytes_size = (int)seg.file_size - page_idx * page_size;

	int bytes_read = 0;

	if (bytes_size > 0) {
		uintptr_t page_addr_file = page_idx * page_size + seg.offset;
		off_t pos = lseek(fd, page_addr_file, SEEK_SET);

		if (pos == -1)
			return -1;
		bytes_read = read(fd, page_ptr, bytes_size);
		if (bytes_read == -1)
			return -1;
	}

	if (bytes_read < page_size) {
		int size = page_size - bytes_read;

		if ((page_idx + 1) * page_size > seg.mem_size)
			size = seg.mem_size - bytes_read - page_size * page_idx;
		memset(page_ptr + bytes_read, 0, size);
	}

	int ret = mprotect(page_ptr, page_size, seg.perm);

	return ret;
}

int treat_faulted_address(uintptr_t addr)
{
	int segment_idx = find_container_segment(addr);

	if (segment_idx == -1)
		return -1;

	so_seg_t segment = exec->segments[segment_idx];
	struct segment_info *s_info = (struct segment_info *)segment.data;

	int page_idx = ceil((uintptr_t)(addr - segment.vaddr) / page_size);

	if (check_page_mapped(page_idx, s_info))
		return -1;

	int ret = map_page(page_idx, segment, s_info);

	return ret;
}

void sigsegv_handler(int signum, siginfo_t *info, void *context)
{
	if (signum == SIGSEGV) {
		int ret = treat_faulted_address((uintptr_t)info->si_addr);

		if (ret == 0)
			return;
	}
	default_signal_action.sa_sigaction(signum, info, context);
}
