#ifndef __SYS_GC_H
#define __SYS_GC_H

#include <ogc/gx.h>
#include "../qcommon/q_shared.h"

void GC_InitConsole(void);
qboolean GC_InitStorage(int argc, char **argv);
void GC_WaitForStorage(int argc, char **argv);
GXRModeObj *GC_GetVideoMode(void);
void *GC_GetConsoleXFB(void);
void GC_SetConsoleHidden(qboolean hidden);

#endif
