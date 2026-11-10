#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include <stdlib.h>
#include <string.h>

typedef intptr_t (*gcVmMain_t)(int command, int arg0, int arg1, int arg2, int arg3,
	int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);
typedef void (*gcDllEntry_t)(intptr_t (QDECL *syscallptr)(intptr_t, ...));

#define GC_MODULE_DECLARE(mod) \
	intptr_t vmMain_##mod(int command, int arg0, int arg1, int arg2, int arg3, int arg4, \
		int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11); \
	void dllEntry_##mod(intptr_t (QDECL *syscallptr)(intptr_t, ...)); \
	extern char gcmod_##mod##_data_start[], gcmod_##mod##_data_end[]; \
	extern char gcmod_##mod##_bss_start[], gcmod_##mod##_bss_end[]

GC_MODULE_DECLARE(qagame);
GC_MODULE_DECLARE(cgame);
GC_MODULE_DECLARE(ui);

typedef struct
{
	const char *name;
	gcVmMain_t vmMain;
	gcDllEntry_t dllEntry;
	char *dataStart;
	char *dataEnd;
	char *bssStart;
	char *bssEnd;
	void *pristineData;
} gcModule_t;

#define GC_MODULE(mod) \
	{ #mod, vmMain_##mod, dllEntry_##mod, \
		gcmod_##mod##_data_start, gcmod_##mod##_data_end, \
		gcmod_##mod##_bss_start, gcmod_##mod##_bss_end, NULL }

static gcModule_t gcModules[] =
{
	GC_MODULE(qagame),
	GC_MODULE(cgame),
	GC_MODULE(ui),
};

static gcModule_t *GC_FindModule(const char *name)
{
	int i;

	for (i = 0; i < (int)ARRAY_LEN(gcModules); i++)
	{
		if (!Q_stricmp(name, gcModules[i].name))
			return &gcModules[i];
	}
	return NULL;
}

void *QDECL Sys_LoadGameDll(const char *name, intptr_t (QDECL **entryPoint)(int, ...),
	intptr_t (QDECL *systemcalls)(intptr_t, ...))
{
	gcModule_t *module = GC_FindModule(name);
	size_t dataSize, bssSize;

	if (!module)
		return NULL;

	dataSize = (size_t)(module->dataEnd - module->dataStart);
	bssSize = (size_t)(module->bssEnd - module->bssStart);

	if (!module->pristineData)
	{
		/* First load: the module has never run, so .data is still the linked image. */
		module->pristineData = malloc(dataSize ? dataSize : 1);
		if (!module->pristineData)
		{
			Com_Printf("GC module %s: no memory for the %d byte data snapshot\n",
				module->name, (int)dataSize);
			return NULL;
		}
		memcpy(module->pristineData, module->dataStart, dataSize);
	}
	else
	{
		memcpy(module->dataStart, module->pristineData, dataSize);
		memset(module->bssStart, 0, bssSize);
	}

	Com_Printf("GC module %s: native, data %d bytes, bss %d bytes\n",
		module->name, (int)dataSize, (int)bssSize);

	*entryPoint = (intptr_t (QDECL *)(int, ...))module->vmMain;
	module->dllEntry(systemcalls);
	return module;
}

void Sys_UnloadDll(void *dllHandle)
{
	(void)dllHandle;
}
