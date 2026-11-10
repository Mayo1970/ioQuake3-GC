#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_local.h"
#include "sys_gc.h"

int main(int argc, char **argv)
{
	char commandLine[MAX_STRING_CHARS] = { 0 };
	int i;

	GC_WaitForStorage(argc, argv);

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

	Com_Init(commandLine);
	NET_Init();

	for (;;) {
		IN_Frame();
		Com_Frame();
	}

	return 0;
}
