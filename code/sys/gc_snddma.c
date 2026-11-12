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
#include <ogc/audio.h>
#include <ogc/cache.h>
#include <ogc/machine/processor.h>
#include <malloc.h>
#include <string.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../client/snd_local.h"

#define GC_SND_RATE			32000
#define GC_SND_BLOCK_FRAMES	512
#define GC_SND_BLOCKS		32
#define GC_SND_FRAMES		( GC_SND_BLOCK_FRAMES * GC_SND_BLOCKS )
#define GC_SND_BLOCK_BYTES	( GC_SND_BLOCK_FRAMES * 4 )
#define GC_SND_BYTES		( GC_SND_FRAMES * 4 )

static u8			*s_ring;
static volatile int	s_playBlock;
static volatile int	s_nextBlock;
static qboolean		s_aiInit;
static qboolean		s_started;

/*
The AI interrupt just swaps blocks; mixing and flushes stay in the main loop.
*/
static void GC_SndDMACallback( void )
{
	s_playBlock = s_nextBlock;
	s_nextBlock = ( s_nextBlock + 1 ) % GC_SND_BLOCKS;
	AUDIO_InitDMA( (u32)( s_ring + s_nextBlock * GC_SND_BLOCK_BYTES ),
		GC_SND_BLOCK_BYTES );
}

qboolean SNDDMA_Init( void )
{
	if ( s_started ) {
		return qtrue;
	}

	if ( !s_aiInit ) {
		AUDIO_Init( NULL );
		AUDIO_SetDSPSampleRate( AI_SAMPLERATE_32KHZ );
		s_aiInit = qtrue;
	}

	s_ring = memalign( 32, GC_SND_BYTES );
	if ( !s_ring ) {
		Com_Printf( "GC snd: no memory for %d bytes\n", GC_SND_BYTES );
		return qfalse;
	}
	memset( s_ring, 0, GC_SND_BYTES );
	DCFlushRange( s_ring, GC_SND_BYTES );

	dma.samplebits = 16;
	dma.speed = GC_SND_RATE;
	dma.channels = 2;
	dma.samples = GC_SND_FRAMES * 2;
	dma.submission_chunk = 1;
	dma.buffer = s_ring;

	s_playBlock = GC_SND_BLOCKS - 1;
	s_nextBlock = 0;
	AUDIO_RegisterDMACallback( GC_SndDMACallback );
	AUDIO_InitDMA( (u32)s_ring, GC_SND_BLOCK_BYTES );
	AUDIO_StartDMA();

	s_started = qtrue;
	Com_Printf( "GC snd: AI %d Hz, ring %d bytes, %d blocks\n",
		GC_SND_RATE, GC_SND_BYTES, GC_SND_BLOCKS );
	return qtrue;
}

int SNDDMA_GetDMAPos( void )
{
	int		block;
	int		frame;
	u32		left;
	u32		level;

	if ( !s_started ) {
		return 0;
	}

	/* Block and byte count must come from the same DMA state. */
	_CPU_ISR_Disable( level );
	block = s_playBlock;
	left = AUDIO_GetDMABytesLeft();
	_CPU_ISR_Restore( level );

	if ( left > GC_SND_BLOCK_BYTES ) {
		left = GC_SND_BLOCK_BYTES;
	}

	frame = block * GC_SND_BLOCK_FRAMES + ( GC_SND_BLOCK_BYTES - left ) / 4;
	return ( frame % GC_SND_FRAMES ) * 2;
}

void SNDDMA_Shutdown( void )
{
	if ( !s_started ) {
		return;
	}

	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback( NULL );
	free( s_ring );
	s_ring = NULL;
	s_started = qfalse;
}

void SNDDMA_BeginPainting( void )
{
}

void SNDDMA_Submit( void )
{
	if ( s_started ) {
		/* The AI reads main memory, not the data cache. */
		DCFlushRange( s_ring, GC_SND_BYTES );
	}
}
