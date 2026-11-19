#ifndef GC_LWIP_MEM_H
#define GC_LWIP_MEM_H

/* Allocated only when the network starts, so an idle game does not hold these pools. */
#define GC_LWIP_ALIGN32(x)		(((x) + 31) & ~31u)
#define GC_LWIP_RAM_SIZE		16396
#define GC_LWIP_MEMP_SIZE		41376
#define GC_LWIP_NET_STACK_SIZE		32768
#define GC_LWIP_BBA_NUM_BUFFERS		32
#define GC_LWIP_BBA_STACK_SIZE		8192

#ifdef HW_DOL

#include <ogc/bba.h>

#define GC_LWIP_BBA_BUFFERS_SIZE	(GC_LWIP_BBA_NUM_BUFFERS * sizeof(BbaBuffer))
#define GC_LWIP_TOTAL_SIZE		(GC_LWIP_ALIGN32(GC_LWIP_RAM_SIZE) + \
					 GC_LWIP_ALIGN32(GC_LWIP_MEMP_SIZE) + \
					 GC_LWIP_ALIGN32(GC_LWIP_NET_STACK_SIZE) + \
					 GC_LWIP_ALIGN32(GC_LWIP_BBA_BUFFERS_SIZE) + \
					 GC_LWIP_ALIGN32(GC_LWIP_BBA_STACK_SIZE))

extern unsigned char *ram;
extern unsigned char *memp_memory;
extern unsigned char *netthread_stack;
extern BbaBuffer *bba_buffers;
extern unsigned char *bba_thread_stack;

int GC_LwipAlloc(void);
void GC_LwipFree(void);

#endif /* HW_DOL */

#endif /* GC_LWIP_MEM_H */
