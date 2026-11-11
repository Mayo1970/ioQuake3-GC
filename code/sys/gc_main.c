#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_local.h"
#include "sys_gc.h"
#include "gc_aram.h"

#ifdef GC_DEBUG
#include <ogc/system.h>
#endif

/* libogc's 128 KB main stack overflows in map load; 512 KB as in the Wii port. */
static unsigned char s_gcMainStack[512 * 1024] __attribute__((aligned(8)));
void *__ppc_main_sp __attribute__((section(".sdata"))) = &s_gcMainStack[sizeof(s_gcMainStack)];

int main(int argc, char **argv)
{
	char commandLine[MAX_STRING_CHARS] = { 0 };
	int i;

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
