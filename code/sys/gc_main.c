#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_local.h"
#include "sys_gc.h"
#include "gc_aram.h"

#ifdef GC_DEBUG
#include <ogc/system.h>
#endif

/* libogc's 128 KB main stack overflows in map load; 448 KB: measured peak 369816 bytes (Matt, QGC-47). */
static unsigned char s_gcMainStack[448 * 1024] __attribute__((aligned(8)));
void *__ppc_main_sp __attribute__((section(".sdata"))) = &s_gcMainStack[sizeof(s_gcMainStack)];

#ifdef GC_DEBUG
int GC_StackSize( void ) {
	return sizeof( s_gcMainStack );
}

int GC_StackPeak( void ) {
	int size = sizeof( s_gcMainStack );
	int i;

	for ( i = 0; i < size; i++ ) {
		if ( s_gcMainStack[i] != 0xA5 ) {
			return size - i;
		}
	}
	return 0;
}
#endif

int main(int argc, char **argv)
{
	char commandLine[MAX_STRING_CHARS] = { 0 };
	int i;

#ifdef GC_DEBUG
	unsigned char *sp = (unsigned char *)__builtin_frame_address( 0 );

	/* Fill the unused stack so GC_StackPeak can find the deepest use (meminfo). */
	if ( sp >= s_gcMainStack && sp < s_gcMainStack + sizeof( s_gcMainStack ) ) {
		int fill = ( sp - s_gcMainStack ) - 4096;

		if ( fill > 0 ) {
			memset( s_gcMainStack, 0xA5, fill );
		}
	}
#endif

	GC_WaitForStorage(argc, argv);

	GC_ARAM_Init();

	Sys_Milliseconds();

	for (i = 1; i < argc; i++) {
		const qboolean containsSpaces = strchr(argv[i], ' ') != NULL;

		if (containsSpaces)
			Q_strcat(commandLine, sizeof(commandLine), "\"");
		Q_strcat(commandLine, sizeof(commandLine), argv[i]);
		if (containsSpaces)
			Q_strcat(commandLine, sizeof(commandLine), "\"");
		Q_strcat(commandLine, sizeof(commandLine), " ");
	}

#ifdef GC_DEBUG
	SYS_Report("GC mem: boot arena1 %u bytes\n", SYS_GetArena1Size());
#endif

	Com_Init(commandLine);
	IN_Init();
	NET_Init();

#ifdef GC_DEBUG
	Com_Printf("GC mem: after Com_Init arena1 %u bytes\n", SYS_GetArena1Size());
#endif

	for (;;) {
		IN_Frame();
		Com_Frame();
	}

	return 0;
}
