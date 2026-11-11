#ifndef __GC_ARAM_H
#define __GC_ARAM_H

#include <gctypes.h>

#define GC_ARAM_MODEL_SIZE  (10 * 1024 * 1024)   /* Matt, QGC-45; sound gets the rest */

void GC_ARAM_Init(void);
u32  GC_ARAM_ModelBase(void);
u32  GC_ARAM_SoundBase(void);
u32  GC_ARAM_SoundSize(void);
void GC_ARAM_Write(u32 aramAddr, void *src, u32 len);
void GC_ARAM_Read(void *dst, u32 aramAddr, u32 len);

#endif
