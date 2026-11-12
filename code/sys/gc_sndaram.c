/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <gccore.h>
#include <ogc/arqueue.h>
#include <ogc/cache.h>
#include <ogc/lwp_watchdog.h>
#include <malloc.h>
#include <string.h>

#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../client/snd_local.h"
#include "gc_aram.h"
#include "gc_sndaram.h"

#define GC_SNDA_SLOTS 16
#define GC_SNDA_SLOT_BYTES (GC_SND_CHUNK_SAMPLES * 2)
#define GC_SNDA_CHUNK_BYTES SND_CHUNK_SIZE_BYTE
#define GC_SNDA_PREFETCH 4
#define GC_SNDA_OWNER 0x534E4400

static sndBuffer *s_pool;
static int s_count;
static short *s_slotData;
static int s_slotKey[GC_SNDA_SLOTS], s_slotUsed[GC_SNDA_SLOTS], s_stamp;
static byte *s_stage;
static ARQRequest s_pfReq[GC_SNDA_PREFETCH];
static int s_pfKey[GC_SNDA_PREFETCH];
static volatile int s_pfPending[GC_SNDA_PREFETCH];
#ifdef GC_DEBUG
static int s_hits, s_misses, s_pfHits, s_pfWaits, s_syncReads;
static u64 s_missTicks, s_maxMissTicks;

static void GC_SndAram_Stats( void )
{
	Com_Printf("GC snd aram: hits %d misses %d pf hits %d pf waits %d sync %d miss avg %u us max %u us\n",
		s_hits, s_misses, s_pfHits, s_pfWaits, s_syncReads,
		(unsigned int)(s_misses ? ticks_to_microsecs(s_missTicks / s_misses) : 0),
		(unsigned int)ticks_to_microsecs(s_maxMissTicks));
	s_hits = s_misses = s_pfHits = s_pfWaits = s_syncReads = 0;
	s_missTicks = s_maxMissTicks = 0;
}
#endif

/* The interrupt callback only signals completion. */
static void GC_SndAram_PrefetchDone( ARQRequest *req )
{
	int i;
	for (i = 0; i < GC_SNDA_PREFETCH; i++)
		if (&s_pfReq[i] == req) { s_pfPending[i] = 0; return; }
}

int GC_SndAram_ChunkCount( void )
{
	return GC_ARAM_SoundSize() / GC_SNDA_CHUNK_BYTES;
}

int GC_SndAram_Init( sndBuffer *pool, int count )
{
	int i;
	s_slotData = memalign(32, GC_SNDA_SLOTS * GC_SNDA_SLOT_BYTES);
	if (!s_slotData) {
		Com_Printf("GC snd aram: allocation failed (%d bytes)\n", GC_SNDA_SLOTS * GC_SNDA_SLOT_BYTES);
		return qfalse;
	}
	s_stage = memalign(32, (GC_SNDA_PREFETCH + 2) * GC_SNDA_CHUNK_BYTES);
	if (!s_stage) {
		Com_Printf("GC snd aram: allocation failed (%d bytes)\n", (GC_SNDA_PREFETCH + 2) * GC_SNDA_CHUNK_BYTES);
		free(s_slotData); s_slotData = NULL;
		return qfalse;
	}
	s_pool = pool; s_count = count; s_stamp = 0;
	for (i = 0; i < GC_SNDA_SLOTS; i++) { s_slotKey[i] = -1; s_slotUsed[i] = 0; }
	for (i = 0; i < GC_SNDA_PREFETCH; i++) { s_pfKey[i] = -1; s_pfPending[i] = 0; }
#ifdef GC_DEBUG
	Cmd_AddCommand("gc_sndaram", GC_SndAram_Stats);
#endif
	Com_Printf("GC snd aram: %d chunks at 0x%x, cache %d x %d bytes\n", s_count, GC_ARAM_SoundBase(), GC_SNDA_SLOTS, GC_SNDA_SLOT_BYTES);
	return qtrue;
}

void GC_SndAram_Shutdown( void )
{
	int i;
	for (i = 0; i < GC_SNDA_PREFETCH; i++) while (s_pfPending[i]) { }
#ifdef GC_DEBUG
	Cmd_RemoveCommand("gc_sndaram");
#endif
	free(s_slotData); free(s_stage);
	s_slotData = NULL; s_stage = NULL;
}

void GC_SndAram_Forget( sndBuffer *chunk )
{
	int i, key = chunk - s_pool;
	for (i = 0; i < GC_SNDA_SLOTS; i++) if (s_slotKey[i] == key) s_slotKey[i] = -1;
	for (i = 0; i < GC_SNDA_PREFETCH; i++) if (s_pfKey[i] == key) s_pfKey[i] = -1;
}

void GC_SndAram_Store( sndBuffer *chunk, const byte *adpcm, int len )
{
	byte *buf = s_stage + (GC_SNDA_PREFETCH + 1) * GC_SNDA_CHUNK_BYTES;
	memset(buf, 0, GC_SNDA_CHUNK_BYTES);
	memcpy(buf, adpcm, len);
	GC_ARAM_Write(GC_ARAM_SoundBase() + (chunk - s_pool) * GC_SNDA_CHUNK_BYTES, buf, GC_SNDA_CHUNK_BYTES);
	GC_SndAram_Forget(chunk);
}

/* The returned slot remains valid until the next samples call. */
const short *GC_SndAram_Samples( sndBuffer *chunk )
{
	int i, slot = 0, pf = -1, key = chunk - s_pool;
	byte *bytes;
	adpcm_state_t state;
#ifdef GC_DEBUG
	u64 start, elapsed;
#endif
	for (i = 0; i < GC_SNDA_SLOTS; i++) {
		if (s_slotKey[i] == key) {
			s_slotUsed[i] = ++s_stamp;
#ifdef GC_DEBUG
			s_hits++;
#endif
			return s_slotData + i * GC_SND_CHUNK_SAMPLES;
		}
	}
#ifdef GC_DEBUG
	s_misses++; start = gettime();
#endif
	for (i = 0; i < GC_SNDA_SLOTS; i++) {
		if (s_slotKey[i] == -1) { slot = i; break; }
		if (s_slotUsed[i] < s_slotUsed[slot]) slot = i;
	}
	for (i = 0; i < GC_SNDA_PREFETCH; i++) if (s_pfKey[i] == key) { pf = i; break; }
	if (pf >= 0) {
#ifdef GC_DEBUG
		s_pfHits++;
		if (s_pfPending[pf]) s_pfWaits++;
#endif
		while (s_pfPending[pf]) { }
		bytes = s_stage + pf * GC_SNDA_CHUNK_BYTES;
	} else {
		bytes = s_stage + GC_SNDA_PREFETCH * GC_SNDA_CHUNK_BYTES;
		DCInvalidateRange(bytes, GC_SNDA_CHUNK_BYTES);
		GC_ARAM_Read(bytes, GC_ARAM_SoundBase() + key * GC_SNDA_CHUNK_BYTES, GC_SNDA_CHUNK_BYTES);
#ifdef GC_DEBUG
		s_syncReads++;
#endif
	}
	/* DMA writes must be visible before the CPU decodes them. */
	DCInvalidateRange(bytes, GC_SNDA_CHUNK_BYTES);
	state = chunk->adpcm;
	S_AdpcmDecode((const char *)bytes, s_slotData + slot * GC_SND_CHUNK_SAMPLES, GC_SND_CHUNK_SAMPLES, &state);
	s_slotKey[slot] = key; s_slotUsed[slot] = ++s_stamp;
	if (pf >= 0) s_pfKey[pf] = -1;
#ifdef GC_DEBUG
	elapsed = diff_ticks(start, gettime());
	s_missTicks += elapsed;
	if (elapsed > s_maxMissTicks) s_maxMissTicks = elapsed;
#endif
	if (chunk->next) {
		int next = chunk->next - s_pool, found = 0;
		for (i = 0; i < GC_SNDA_SLOTS; i++) if (s_slotKey[i] == next) found = 1;
		for (i = 0; i < GC_SNDA_PREFETCH; i++) if (s_pfKey[i] == next) found = 1;
		if (!found) for (i = 0; i < GC_SNDA_PREFETCH; i++) if (!s_pfPending[i]) {
			byte *buf = s_stage + i * GC_SNDA_CHUNK_BYTES;
			DCInvalidateRange(buf, GC_SNDA_CHUNK_BYTES);
			s_pfKey[i] = next; s_pfPending[i] = 1;
			ARQ_PostRequestAsync(&s_pfReq[i], GC_SNDA_OWNER, ARQ_ARAMTOMRAM, ARQ_PRIO_LO,
				GC_ARAM_SoundBase() + next * GC_SNDA_CHUNK_BYTES, (u32)buf, GC_SNDA_CHUNK_BYTES, GC_SndAram_PrefetchDone);
			break;
		}
	}
	return s_slotData + slot * GC_SND_CHUNK_SAMPLES;
}
