/* GX window / surface management declarations. */
#pragma once
#include <gccore.h>
#include "../qcommon/q_shared.h"

qboolean    GC_GX_Init(void);
void        GC_GX_EndFrame(void);
void        GC_GX_Shutdown(void);
GXRModeObj *GC_GX_GetRMode(void);
int         GC_GX_GetEFBHeight(void);
int         GC_GX_GetXFBHeight(void);

/* Height the engine sees under native GX. tr_gx.c maps it onto the EFB. */
#define GC_GX_LOGICAL_HEIGHT 480
