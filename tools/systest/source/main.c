#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "q_shared.h"
#include "qcommon.h"
#include "sys_gc.h"

int main(int argc, char **argv)
{
	char home[MAX_OSPATH];
	char path[MAX_OSPATH];
	char **names;
	qboolean have;
	int t0, t1, n, i;

	GC_InitConsole();
	printf("ioQuake3-GC systest\n");

	Sys_Init();

	t0 = Sys_Milliseconds();
	Sys_Sleep(500);
	t1 = Sys_Milliseconds();
	printf("sleep 500 -> %d ms\n", t1 - t0);

	printf("LittleLong(0x11223344) = 0x%08x\n",
	       (unsigned int)LittleLong(0x11223344));

#ifdef SYSTEST_WAIT
	GC_WaitForStorage(argc, argv);
	have = qtrue;
#else
	have = GC_InitStorage(argc, argv);
	printf("storage: %s\n", have ? "yes" : "no");
#endif

	if (have) {
		strncpy(home, Sys_DefaultHomePath(), sizeof(home) - 1);
		home[sizeof(home) - 1] = '\0';
		printf("%s\n", home);

		Com_sprintf(path, sizeof(path), "%s/systest", home);
		printf("Sys_Mkdir(%s) = %d\n", path, Sys_Mkdir(path));

		names = Sys_ListFiles(home, "/", NULL, &n, qfalse);
		printf("dirs: %d\n", n);
		for (i = 0; names && names[i]; i++)
			printf("  %s\n", names[i]);
		Sys_FreeFileList(names);

		Com_sprintf(path, sizeof(path), "%s/" BASEGAME, home);
		names = Sys_ListFiles(path, ".pk3", NULL, &n, qfalse);
		printf("pk3: %d\n", n);
		for (i = 0; names && names[i]; i++)
			printf("  %s\n", names[i]);
		Sys_FreeFileList(names);
	}

	Sys_Error("systest: Sys_Error test %d", 42);

	return 0;
}
