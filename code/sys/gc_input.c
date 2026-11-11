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
#include <ogc/pad.h>
#include <string.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../client/keys.h"

extern int Key_GetCatcher( void );

#define GC_STICK_DEADZONE		20
#define GC_STICK_MAX			100
#define GC_CSTICK_DEADZONE		50
#define GC_CSTICK_MAX			85
#define GC_TRIGGER_THRESHOLD	100
#define GC_MENU_CURSOR_SPEED	6.0f
#define GC_UNPLUG_SCANS			10

#define GC_AXIS_SIDE			0
#define GC_AXIS_FORWARD			1
#define GC_AXIS_PITCH			3
#define GC_AXIS_YAW				4
#define GC_AXIS_COUNT			5

#define GC_SYNTH_LTRIG			0x40000000u
#define GC_SYNTH_RTRIG			0x80000000u

typedef enum {
	GC_MODE_GAME,
	GC_MODE_MENU,
	GC_MODE_CONSOLE,
	GC_MODE_MESSAGE
} gc_mode_t;

static qboolean	s_key_held[MAX_KEYS];
static int		s_active_chan = -1;
static int		s_unplug_scans;
static gc_mode_t s_mode;
static qboolean	s_mode_valid;
static u32		s_suppress;
static short	s_old_axis[GC_AXIS_COUNT];
static float	s_accum_x, s_accum_y, s_accum_cx, s_accum_cy;
static qboolean	s_raw;
static qboolean	s_raw_used;
static qboolean	s_combo_hold;
static qboolean	s_latch_a, s_latch_y, s_latch_start;
static qboolean	s_prev_a, s_prev_y, s_prev_start;

static void InjectKey( int q3key, qboolean down )
{
	if ( q3key < 0 || q3key >= MAX_KEYS ) {
		return;
	}
	if ( s_key_held[q3key] == down ) {
		return;
	}
	s_key_held[q3key] = down;
	Com_QueueEvent( 0, SE_KEY, q3key, down, 0, NULL );
}

static void ReleaseAll( void )
{
	int i;

	for ( i = 0; i < MAX_KEYS; i++ ) {
		if ( s_key_held[i] ) {
			s_key_held[i] = qfalse;
			Com_QueueEvent( 0, SE_KEY, i, qfalse, 0, NULL );
		}
	}

	Com_QueueEvent( 0, SE_JOYSTICK_AXIS, GC_AXIS_SIDE, 0, 0, NULL );
	Com_QueueEvent( 0, SE_JOYSTICK_AXIS, GC_AXIS_FORWARD, 0, 0, NULL );
	Com_QueueEvent( 0, SE_JOYSTICK_AXIS, GC_AXIS_PITCH, 0, 0, NULL );
	Com_QueueEvent( 0, SE_JOYSTICK_AXIS, GC_AXIS_YAW, 0, 0, NULL );
	memset( s_old_axis, 0, sizeof( s_old_axis ) );

	s_accum_x = s_accum_y = s_accum_cx = s_accum_cy = 0.0f;
	s_suppress = 0;
	s_raw = qfalse;
	s_raw_used = qtrue;
	s_combo_hold = qfalse;
	s_latch_a = s_latch_y = s_latch_start = qfalse;
	s_prev_a = s_prev_y = s_prev_start = qfalse;
}

static u32 GC_ReadMask( int chan )
{
	u32 mask = PAD_ButtonsHeld( chan );

	if ( PAD_TriggerL( chan ) > GC_TRIGGER_THRESHOLD ) {
		mask |= GC_SYNTH_LTRIG;
	}
	if ( PAD_TriggerR( chan ) > GC_TRIGGER_THRESHOLD ) {
		mask |= GC_SYNTH_RTRIG;
	}
	return mask;
}

static short GC_FilterAxis( s8 raw, int deadzone, int max )
{
	int v = raw;
	int sign = 1;

	if ( v < 0 ) {
		sign = -1;
		v = -v;
	}
	if ( v < deadzone ) {
		return 0;
	}
	if ( v > max ) {
		v = max;
	}
	/* Rescale past the dead zone so the axis does not jump at the edge. */
	return (short)( sign * ( ( v - deadzone ) * 32767 / ( max - deadzone ) ) );
}

static void GC_SetAxis( int axis, short value )
{
	if ( s_old_axis[axis] != value ) {
		Com_QueueEvent( 0, SE_JOYSTICK_AXIS, axis, value, 0, NULL );
		s_old_axis[axis] = value;
	}
}

static void GC_InjectCursorStick( s8 x, s8 y, int max, int deadzone,
	float *ax, float *ay )
{
	int ix = x;
	int iy = y;
	int out_x, out_y;

	if ( ix > -deadzone && ix < deadzone ) {
		ix = 0;
	}
	if ( iy > -deadzone && iy < deadzone ) {
		iy = 0;
	}
	if ( ix == 0 && iy == 0 ) {
		*ax = *ay = 0.0f;
		return;
	}

	if ( ix > max ) {
		ix = max;
	}
	if ( ix < -max ) {
		ix = -max;
	}
	if ( iy > max ) {
		iy = max;
	}
	if ( iy < -max ) {
		iy = -max;
	}

	*ax += ( (float)ix / (float)max ) * GC_MENU_CURSOR_SPEED;
	*ay += ( (float)iy / (float)max ) * GC_MENU_CURSOR_SPEED;

	out_x = (int)*ax;
	out_y = (int)*ay;
	*ax -= (float)out_x;
	*ay -= (float)out_y;

	if ( out_x != 0 || out_y != 0 ) {
		Com_QueueEvent( 0, SE_MOUSE, out_x, -out_y, 0, NULL );
	}
}

static void GC_GameFrame( int chan, u32 eff )
{
	qboolean z = ( eff & PAD_TRIGGER_Z ) ? qtrue : qfalse;
	qboolean a = ( eff & PAD_BUTTON_A ) ? qtrue : qfalse;
	qboolean y = ( eff & PAD_BUTTON_Y ) ? qtrue : qfalse;
	qboolean start = ( eff & PAD_BUTTON_START ) ? qtrue : qfalse;
	qboolean a_press = ( a && !s_prev_a ) ? qtrue : qfalse;
	qboolean y_press = ( y && !s_prev_y ) ? qtrue : qfalse;
	qboolean start_press = ( start && !s_prev_start ) ? qtrue : qfalse;
	char *start_bind;
	int start_key;

	/* Combos need a press that starts while Z is already held. */
	if ( !z ) {
		s_combo_hold = qfalse;
	} else if ( a_press || y_press || start_press ) {
		if ( a_press ) {
			Cbuf_AddText( "messagemode\n" );
			s_latch_a = qtrue;
		} else if ( y_press ) {
			Cbuf_AddText( "toggleconsole\n" );
			s_latch_y = qtrue;
		} else {
			Cbuf_AddText( "quit\n" );
			s_latch_start = qtrue;
		}
		s_combo_hold = qtrue;
	}

	if ( !a ) {
		s_latch_a = qfalse;
	}
	if ( !y ) {
		s_latch_y = qfalse;
	}
	if ( !start ) {
		s_latch_start = qfalse;
	}

	InjectKey( K_JOY1, ( a && !s_latch_a ) ? qtrue : qfalse );
	InjectKey( K_JOY2, ( eff & PAD_BUTTON_B ) ? qtrue : qfalse );
	InjectKey( K_JOY3, ( eff & PAD_BUTTON_X ) ? qtrue : qfalse );
	InjectKey( K_JOY4, ( y && !s_latch_y ) ? qtrue : qfalse );
	InjectKey( K_JOY5, ( z && !s_combo_hold ) ? qtrue : qfalse );

	/* Unbound Start falls back to Escape so it can still leave menus. */
	start_bind = Key_GetBinding( K_JOY6 );
	start_key = ( start_bind && start_bind[0] ) ? K_JOY6 : K_ESCAPE;
	InjectKey( start_key, ( start && !s_latch_start ) ? qtrue : qfalse );

	InjectKey( K_JOY7, ( eff & ( PAD_TRIGGER_L | GC_SYNTH_LTRIG ) ) ? qtrue : qfalse );
	InjectKey( K_JOY8, ( eff & ( PAD_TRIGGER_R | GC_SYNTH_RTRIG ) ) ? qtrue : qfalse );

	InjectKey( K_UPARROW, ( eff & PAD_BUTTON_UP ) ? qtrue : qfalse );
	InjectKey( K_DOWNARROW, ( eff & PAD_BUTTON_DOWN ) ? qtrue : qfalse );
	InjectKey( K_LEFTARROW, ( eff & PAD_BUTTON_LEFT ) ? qtrue : qfalse );
	InjectKey( K_RIGHTARROW, ( eff & PAD_BUTTON_RIGHT ) ? qtrue : qfalse );

	GC_SetAxis( GC_AXIS_SIDE,
		GC_FilterAxis( PAD_StickX( chan ), GC_STICK_DEADZONE, GC_STICK_MAX ) );
	GC_SetAxis( GC_AXIS_FORWARD,
		(short)-GC_FilterAxis( PAD_StickY( chan ), GC_STICK_DEADZONE, GC_STICK_MAX ) );
	GC_SetAxis( GC_AXIS_YAW,
		GC_FilterAxis( PAD_SubStickX( chan ), GC_CSTICK_DEADZONE, GC_CSTICK_MAX ) );
	GC_SetAxis( GC_AXIS_PITCH,
		(short)-GC_FilterAxis( PAD_SubStickY( chan ), GC_CSTICK_DEADZONE, GC_CSTICK_MAX ) );

	s_prev_a = a;
	s_prev_y = y;
	s_prev_start = start;
}

static void GC_ConsoleFrame( u32 eff )
{
	qboolean z = ( eff & PAD_TRIGGER_Z ) ? qtrue : qfalse;
	qboolean y = ( eff & PAD_BUTTON_Y ) ? qtrue : qfalse;
	qboolean start = ( eff & PAD_BUTTON_START ) ? qtrue : qfalse;
	qboolean y_press = ( y && !s_prev_y ) ? qtrue : qfalse;
	qboolean start_press = ( start && !s_prev_start ) ? qtrue : qfalse;

	if ( z && y_press ) {
		Cbuf_AddText( "toggleconsole\n" );
		s_latch_y = qtrue;
	}
	if ( z && start_press ) {
		Cbuf_AddText( "quit\n" );
		s_latch_start = qtrue;
	}

	if ( !y ) {
		s_latch_y = qfalse;
	}
	if ( !start ) {
		s_latch_start = qfalse;
	}

	InjectKey( K_ENTER, ( eff & PAD_BUTTON_A ) ? qtrue : qfalse );
	InjectKey( K_ESCAPE, ( ( eff & PAD_BUTTON_B ) ||
		( start && !s_latch_start ) ) ? qtrue : qfalse );

	InjectKey( K_UPARROW, ( eff & PAD_BUTTON_UP ) ? qtrue : qfalse );
	InjectKey( K_DOWNARROW, ( eff & PAD_BUTTON_DOWN ) ? qtrue : qfalse );
	InjectKey( K_LEFTARROW, ( eff & PAD_BUTTON_LEFT ) ? qtrue : qfalse );
	InjectKey( K_RIGHTARROW, ( eff & PAD_BUTTON_RIGHT ) ? qtrue : qfalse );

	s_prev_y = y;
	s_prev_start = start;
}

static void GC_MessageFrame( u32 eff )
{
	InjectKey( K_ENTER, ( eff & PAD_BUTTON_A ) ? qtrue : qfalse );
	InjectKey( K_ESCAPE,
		( eff & ( PAD_BUTTON_B | PAD_BUTTON_START ) ) ? qtrue : qfalse );

	InjectKey( K_UPARROW, ( eff & PAD_BUTTON_UP ) ? qtrue : qfalse );
	InjectKey( K_DOWNARROW, ( eff & PAD_BUTTON_DOWN ) ? qtrue : qfalse );
	InjectKey( K_LEFTARROW, ( eff & PAD_BUTTON_LEFT ) ? qtrue : qfalse );
	InjectKey( K_RIGHTARROW, ( eff & PAD_BUTTON_RIGHT ) ? qtrue : qfalse );
}

static void GC_MenuFrame( int chan, u32 raw )
{
	u32 eff = raw & ~s_suppress;
	qboolean z = ( eff & PAD_TRIGGER_Z ) ? qtrue : qfalse;

	/* Raw bind layer: Z held emits game JOY codes for the Controls screen. */
	if ( !s_raw && z ) {
		ReleaseAll();
		s_raw = qtrue;
		s_raw_used = qfalse;
		eff = raw;
	} else if ( s_raw && !z ) {
		qboolean tap = s_raw_used ? qfalse : qtrue;

		ReleaseAll();
		s_raw = qfalse;
		s_suppress = raw & ~(u32)PAD_TRIGGER_Z;
		eff = raw & ~s_suppress;
		if ( tap ) {
			/* Bind grabbers act on key-down; the immediate up keeps key_held honest. */
			InjectKey( K_JOY5, qtrue );
			InjectKey( K_JOY5, qfalse );
		}
	}

	if ( s_raw ) {
		InjectKey( K_JOY1, ( eff & PAD_BUTTON_A ) ? qtrue : qfalse );
		InjectKey( K_JOY2, ( eff & PAD_BUTTON_B ) ? qtrue : qfalse );
		InjectKey( K_JOY3, ( eff & PAD_BUTTON_X ) ? qtrue : qfalse );
		InjectKey( K_JOY4, ( eff & PAD_BUTTON_Y ) ? qtrue : qfalse );
		InjectKey( K_JOY6, ( eff & PAD_BUTTON_START ) ? qtrue : qfalse );
		InjectKey( K_JOY7, ( eff & ( PAD_TRIGGER_L | GC_SYNTH_LTRIG ) ) ? qtrue : qfalse );
		InjectKey( K_JOY8, ( eff & ( PAD_TRIGGER_R | GC_SYNTH_RTRIG ) ) ? qtrue : qfalse );

		if ( eff & ( PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y |
			PAD_BUTTON_START | PAD_TRIGGER_L | PAD_TRIGGER_R |
			GC_SYNTH_LTRIG | GC_SYNTH_RTRIG | PAD_BUTTON_UP |
			PAD_BUTTON_DOWN | PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT ) ) {
			s_raw_used = qtrue;
		}
	} else {
		InjectKey( K_ENTER, ( eff & PAD_BUTTON_A ) ? qtrue : qfalse );
		InjectKey( K_ESCAPE,
			( eff & ( PAD_BUTTON_B | PAD_BUTTON_START ) ) ? qtrue : qfalse );
	}

	InjectKey( K_UPARROW, ( eff & PAD_BUTTON_UP ) ? qtrue : qfalse );
	InjectKey( K_DOWNARROW, ( eff & PAD_BUTTON_DOWN ) ? qtrue : qfalse );
	InjectKey( K_LEFTARROW, ( eff & PAD_BUTTON_LEFT ) ? qtrue : qfalse );
	InjectKey( K_RIGHTARROW, ( eff & PAD_BUTTON_RIGHT ) ? qtrue : qfalse );

	GC_InjectCursorStick( PAD_StickX( chan ), PAD_StickY( chan ),
		GC_STICK_MAX, GC_STICK_DEADZONE, &s_accum_x, &s_accum_y );
	GC_InjectCursorStick( PAD_SubStickX( chan ), PAD_SubStickY( chan ),
		GC_CSTICK_MAX, GC_CSTICK_DEADZONE, &s_accum_cx, &s_accum_cy );
}

static gc_mode_t GC_CurrentMode( void )
{
	int catcher = Key_GetCatcher();

	if ( catcher & KEYCATCH_CONSOLE ) {
		return GC_MODE_CONSOLE;
	}
	if ( catcher & KEYCATCH_UI ) {
		return GC_MODE_MENU;
	}
	if ( catcher & KEYCATCH_MESSAGE ) {
		return GC_MODE_MESSAGE;
	}
	return GC_MODE_GAME;
}

void IN_Init( void )
{
	static const struct {
		int key;
		const char *cmd;
	} default_binds[] = {
		{ K_JOY1, "+moveup" },
		{ K_JOY2, "+movedown" },
		{ K_JOY3, "weapnext" },
		{ K_JOY4, "weapprev" },
		{ K_JOY5, "+scores" },
		{ K_JOY6, "togglemenu" },
		{ K_JOY7, "+zoom" },
		{ K_JOY8, "+attack" }
	};
	int i;

	for ( i = 0; i < MAX_KEYS; i++ ) {
		s_key_held[i] = qfalse;
	}

	s_active_chan = -1;
	s_unplug_scans = 0;
	s_mode = GC_MODE_GAME;
	s_mode_valid = qfalse;
	s_suppress = 0;
	memset( s_old_axis, 0, sizeof( s_old_axis ) );
	s_accum_x = s_accum_y = s_accum_cx = s_accum_cy = 0.0f;
	s_raw = qfalse;
	s_raw_used = qtrue;
	s_combo_hold = qfalse;
	s_latch_a = s_latch_y = s_latch_start = qfalse;
	s_prev_a = s_prev_y = s_prev_start = qfalse;

	for ( i = 0; i < (int)( sizeof( default_binds ) / sizeof( default_binds[0] ) ); i++ ) {
		char *bind = Key_GetBinding( default_binds[i].key );

		if ( !bind || !bind[0] ) {
			Key_SetBinding( default_binds[i].key, default_binds[i].cmd );
		}
	}

	Com_Printf( "GC pad: input ready\n" );
}

void IN_Frame( void )
{
	u32 scan;
	u32 raw;
	int chan;
	gc_mode_t mode;

	scan = PAD_ScanPads();

	if ( s_active_chan >= 0 ) {
		if ( scan & ( 1u << s_active_chan ) ) {
			s_unplug_scans = 0;
		} else {
			/* Debounce the scan bit so a single read error does not drop the pad. */
			if ( ++s_unplug_scans < GC_UNPLUG_SCANS ) {
				return;
			}
			Com_Printf( "GC pad: port %d unplugged\n", s_active_chan + 1 );
			ReleaseAll();
			s_active_chan = -1;
			s_unplug_scans = 0;
		}
	}

	if ( s_active_chan < 0 ) {
		for ( chan = 0; chan < PAD_CHANMAX; chan++ ) {
			if ( scan & ( 1u << chan ) ) {
				break;
			}
		}
		if ( chan == PAD_CHANMAX ) {
			return;
		}
		s_active_chan = chan;
		s_unplug_scans = 0;
		Com_Printf( "GC pad: port %d active\n", chan + 1 );
	}

	raw = GC_ReadMask( s_active_chan );
	mode = GC_CurrentMode();

	if ( !s_mode_valid || mode != s_mode ) {
		ReleaseAll();
		s_mode = mode;
		s_mode_valid = qtrue;
		/* Buttons held across the change stay quiet until released. */
		s_suppress = raw;
	}

	s_suppress &= raw;

	if ( mode == GC_MODE_MENU ) {
		GC_MenuFrame( s_active_chan, raw );
	} else {
		u32 eff = raw & ~s_suppress;

		switch ( mode ) {
		case GC_MODE_GAME:
			GC_GameFrame( s_active_chan, eff );
			break;
		case GC_MODE_CONSOLE:
			GC_ConsoleFrame( eff );
			break;
		case GC_MODE_MESSAGE:
			GC_MessageFrame( eff );
			break;
		default:
			break;
		}
	}
}

void IN_Shutdown( void )
{
	if ( s_active_chan != -1 ) {
		ReleaseAll();
	}
	s_active_chan = -1;
}

void IN_Restart( void )
{
	IN_Shutdown();
	IN_Init();
}

void Sys_SendKeyEvents( void )
{
}
