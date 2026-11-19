#include <malloc.h>
#include <string.h>

#include "gc_lwip_mem.h"

#ifdef HW_DOL

static void *alloc_buf(unsigned int size)
{
	void *p = memalign(32, GC_LWIP_ALIGN32(size));

	if (p)
		memset(p, 0, GC_LWIP_ALIGN32(size));
	return p;
}

int GC_LwipAlloc(void)
{
	ram = alloc_buf(GC_LWIP_RAM_SIZE);
	memp_memory = alloc_buf(GC_LWIP_MEMP_SIZE);
	netthread_stack = alloc_buf(GC_LWIP_NET_STACK_SIZE);
	bba_buffers = alloc_buf(GC_LWIP_BBA_BUFFERS_SIZE);
	bba_thread_stack = alloc_buf(GC_LWIP_BBA_STACK_SIZE);

	if (!ram || !memp_memory || !netthread_stack ||
	    !bba_buffers || !bba_thread_stack) {
		GC_LwipFree();
		return -1;
	}
	return 0;
}

void GC_LwipFree(void)
{
	free(ram);
	ram = NULL;
	free(memp_memory);
	memp_memory = NULL;
	free(netthread_stack);
	netthread_stack = NULL;
	free(bba_buffers);
	bba_buffers = NULL;
	free(bba_thread_stack);
	bba_thread_stack = NULL;
}

#endif /* HW_DOL */
