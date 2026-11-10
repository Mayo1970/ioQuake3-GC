/* Test shims for systest; not engine code. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "q_shared.h"
#include "qcommon.h"

#ifdef ZONE_DEBUG
void *Z_MallocDebug(int size, char *label, char *file, int line)
#else
void *Z_Malloc(int size)
#endif
{
	void *p = malloc(size);

	if (p)
		memset(p, 0, size);
	return p;
}

void Z_Free(void *ptr)
{
	free(ptr);
}

char *CopyString(const char *in)
{
	size_t len = strlen(in) + 1;
	char *out = malloc(len);

	memcpy(out, in, len);
	return out;
}

int Com_FilterPath(char *filter, char *name, int casesensitive)
{
	return qtrue;
}

void QDECL Com_Printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void QDECL Com_Error(int code, const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, fmt);
	Q_vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	Sys_Error("%s", buf);
}

void Cvar_Set(const char *var_name, const char *value)
{
	printf("%s = %s\n", var_name, value);
}
