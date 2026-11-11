#include <gccore.h>

#include "gc_aram.h"

#define GC_ARAM_OWNER	0x4D443300

static int s_initialized;
static u32 s_modelBase;
static u32 s_shaderBase;
static u32 s_soundBase;
static u32 s_soundSize;

static int GC_ARAM_Aligned(u32 aramAddr, const void *ptr, u32 len)
{
	return !(aramAddr & 31) && !((u32)ptr & 31) && !(len & 31);
}

void GC_ARAM_Init(void)
{
	u32 base;

	if (s_initialized)
		return;
	s_initialized = 1;

	base = AR_Init(NULL, 0);
	ARQ_Init();

	s_modelBase = base;
	s_shaderBase = base + GC_ARAM_MODEL_SIZE;
	s_soundBase = s_shaderBase + GC_ARAM_SHADER_SIZE;
	s_soundSize = AR_GetSize() - s_soundBase;

#ifdef GC_DEBUG
	SYS_Report("GC aram: size %u, models 0x%x +%u, shader 0x%x +%u, sound 0x%x +%u\n",
		AR_GetSize(), s_modelBase, (u32)GC_ARAM_MODEL_SIZE, s_shaderBase, (u32)GC_ARAM_SHADER_SIZE, s_soundBase, s_soundSize);
#endif
}

u32 GC_ARAM_ModelBase(void)
{
	return s_modelBase;
}

u32 GC_ARAM_ShaderBase(void)
{
	return s_shaderBase;
}

u32 GC_ARAM_SoundBase(void)
{
	return s_soundBase;
}

u32 GC_ARAM_SoundSize(void)
{
	return s_soundSize;
}

void GC_ARAM_Write(u32 aramAddr, void *src, u32 len)
{
	ARQRequest req;

	if (!GC_ARAM_Aligned(aramAddr, src, len)) {
		SYS_Report("GC aram: write rejected (aram 0x%x, mram 0x%x, len %u)\n",
			aramAddr, (u32)src, len);
		return;
	}

	DCFlushRange(src, len);
	ARQ_PostRequest(&req, GC_ARAM_OWNER, ARQ_MRAMTOARAM, ARQ_PRIO_HI,
		aramAddr, (u32)src, len);
}

void GC_ARAM_Read(void *dst, u32 aramAddr, u32 len)
{
	ARQRequest req;

	if (!GC_ARAM_Aligned(aramAddr, dst, len)) {
		SYS_Report("GC aram: read rejected (aram 0x%x, mram 0x%x, len %u)\n",
			aramAddr, (u32)dst, len);
		return;
	}

	ARQ_PostRequest(&req, GC_ARAM_OWNER, ARQ_ARAMTOMRAM, ARQ_PRIO_HI,
		aramAddr, (u32)dst, len);
	DCInvalidateRange(dst, len);
}
