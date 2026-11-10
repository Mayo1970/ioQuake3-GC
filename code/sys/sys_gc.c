#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <fat.h>
#include <sdcard/gcsd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "sys_gc.h"
#include "gc_dvd.h"

#ifndef GC_DISC_ID
#define GC_DISC_ID "GQ3E"
#endif

#define GC_FAT_CACHE_PAGES 4
#define GC_FAT_SECTORS_PER_PAGE 64

#define GC_NUM_DEVICES 3

typedef struct {
	const char *name;
	const DISC_INTERFACE *iface;
	qboolean mounted;
} gc_device_t;

static gc_device_t gc_devices[GC_NUM_DEVICES] = {
	{ "sd2sp2", &__io_gcsd2, qfalse },
	{ "carda",  &__io_gcsda, qfalse },
	{ "cardb",  &__io_gcsdb, qfalse }
};

static char gc_basepath[MAX_OSPATH];

static qboolean gc_dvd_tried;
static qboolean gc_dvd_mounted;

static GXRModeObj gc_mode;
static void *gc_console_xfb;
static qboolean gc_console_hidden;

#ifdef GC_DEBUG
static FILE *gc_log;
#endif

void GC_InitConsole(void)
{
	static qboolean initialized = qfalse;
	GXRModeObj *rmode;

	if (initialized)
		return;
	initialized = qtrue;

	VIDEO_Init();
	PAD_Init();

	/* VIDEO_GetPreferredMode crashes on an unknown TV mode when given a pointer. */
	rmode = VIDEO_GetPreferredMode(NULL);
	if (!rmode)
		rmode = &TVNtsc480IntDf;
	gc_mode = *rmode;

	gc_console_xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(&gc_mode));

	console_init(gc_console_xfb, 20, 20, gc_mode.fbWidth, gc_mode.xfbHeight,
	             gc_mode.fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(&gc_mode);
	VIDEO_SetNextFramebuffer(gc_console_xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (gc_mode.viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

GXRModeObj *GC_GetVideoMode(void)
{
	GC_InitConsole();
	return &gc_mode;
}

void *GC_GetConsoleXFB(void)
{
	GC_InitConsole();
	return gc_console_xfb;
}

void GC_SetConsoleHidden(qboolean hidden)
{
	gc_console_hidden = hidden;
}

qboolean GC_InitStorage(int argc, char **argv)
{
	int order[GC_NUM_DEVICES];
	int n = 0, i, sel = -1;
	char path[MAX_OSPATH];
	struct stat st;

	if (argc > 0 && argv && argv[0]) {
		for (i = 0; i < GC_NUM_DEVICES; i++) {
			size_t len = strlen(gc_devices[i].name);
			if (!strncmp(argv[0], gc_devices[i].name, len) &&
			    argv[0][len] == ':') {
				sel = i;
				break;
			}
		}
	}
	if (sel >= 0)
		order[n++] = sel;
	for (i = 0; i < GC_NUM_DEVICES; i++) {
		if (i != sel)
			order[n++] = i;
	}

	for (i = 0; i < n; i++) {
		gc_device_t *dev = &gc_devices[order[i]];

		Com_sprintf(path, sizeof(path), "%s:/quake3", dev->name);

		if (!dev->mounted) {
			if (!fatMount(dev->name, dev->iface, 0,
			              GC_FAT_CACHE_PAGES, GC_FAT_SECTORS_PER_PAGE))
				continue;
			dev->mounted = qtrue;
		}

		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
			strncpy(gc_basepath, path, sizeof(gc_basepath) - 1);
			gc_basepath[sizeof(gc_basepath) - 1] = '\0';
			chdir(gc_basepath);
#ifdef GC_DEBUG
			if (!gc_log) {
				char logpath[MAX_OSPATH];
				Com_sprintf(logpath, sizeof(logpath), "%s/gc_debug.log",
				            gc_basepath);
				gc_log = fopen(logpath, "w");
			}
#endif
			return qtrue;
		}

		fatUnmount(dev->name);
		dev->mounted = qfalse;
	}

	if (!gc_dvd_tried) {
		gc_dvd_tried = qtrue;
		if (memcmp((const void *)0x80000000, GC_DISC_ID, 4) == 0)
			gc_dvd_mounted = GC_DVD_Mount();
	}

	if (gc_dvd_mounted) {
		Com_sprintf(path, sizeof(path), "dvd:/quake3");
		if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
			strncpy(gc_basepath, path, sizeof(gc_basepath) - 1);
			gc_basepath[sizeof(gc_basepath) - 1] = '\0';
			/* dvd: is read-only; a failed chdir is harmless. */
			chdir(gc_basepath);
#ifdef GC_DEBUG
			if (!gc_log) {
				char logpath[MAX_OSPATH];
				Com_sprintf(logpath, sizeof(logpath), "%s/gc_debug.log",
				            gc_basepath);
				gc_log = fopen(logpath, "w");
			}
#endif
			return qtrue;
		}
	}

	return qfalse;
}

void GC_WaitForStorage(int argc, char **argv)
{
	GC_InitConsole();

	if (GC_InitStorage(argc, argv))
		return;

	printf("No quake3 folder found on sd2sp2:, carda: or cardb:.\n");
	printf("Insert the card, then press A to retry. Press START to exit.\n");

	for (;;) {
		u32 pressed;

		PAD_ScanPads();
		pressed = PAD_ButtonsDown(0);

		if (pressed & PAD_BUTTON_A) {
			printf("Retrying...\n");
			if (GC_InitStorage(argc, argv))
				return;
			printf("No quake3 folder found on sd2sp2:, carda: or cardb:.\n");
			printf("Insert the card, then press A to retry. Press START to exit.\n");
		}
		if (pressed & PAD_BUTTON_START)
			Sys_Quit();

		VIDEO_WaitVSync();
	}
}

int Sys_Milliseconds(void)
{
	static u64 timeBase;
	static qboolean initialized = qfalse;

	if (!initialized) {
		timeBase = gettime();
		initialized = qtrue;
	}
	return (int)ticks_to_millisecs(gettime() - timeBase);
}

void Sys_Sleep(int msec)
{
	if (msec > 0)
		usleep(msec * 1000);
}

void Sys_Init(void)
{
	Cvar_Set("arch", OS_STRING " " ARCH_STRING);
	Com_Printf("CPU: Gekko (PowerPC 750CXe), 24 MB main memory\n");
}

void Sys_GLimpInit(void)
{
}

void Sys_GLimpSafeInit(void)
{
}

void Sys_PlatformInit(void)
{
}

void Sys_PlatformExit(void)
{
}

cpuFeatures_t Sys_GetProcessorFeatures(void)
{
	return (cpuFeatures_t)0;
}

char *Sys_DefaultInstallPath(void)
{
	return gc_basepath;
}

char *Sys_DefaultHomePath(void)
{
	return gc_basepath;
}

char *Sys_DefaultAppPath(void)
{
	return gc_basepath;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==================
Sys_ListFilteredFiles
==================
*/
static void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles )
{
	char          search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char          filename[MAX_OSPATH];
	DIR           *fdir;
	struct dirent *d;
	struct stat   st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s/%s", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR) {
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );
		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;
		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

/*
==================
Sys_ListFiles
==================
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs )
{
	struct dirent *d;
	DIR           *fdir;
	qboolean      dironly = wantsubs;
	char          search[MAX_OSPATH];
	int           nfiles;
	char          **listCopy;
	char          *list[MAX_FOUND_FILES];
	int           i;
	struct stat   st;

	int           extLen;

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

	if ((fdir = opendir(directory)) == NULL) {
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < extLen ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - extLen,
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = NULL;

	closedir(fdir);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

/*
==================
Sys_FreeFileList
==================
*/
void Sys_FreeFileList( char **list )
{
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

qboolean Sys_Mkdir(const char *path)
{
	return mkdir(path, 0777) == 0 || errno == EEXIST;
}

mode_t umask(mode_t mask)
{
	return 0;
}

FILE *Sys_Mkfifo(const char *ospath)
{
	return NULL;
}

char *Sys_ConsoleInput(void)
{
	return NULL;
}

char *Sys_GetClipboardData(void)
{
	return NULL;
}

qboolean Sys_LowPhysicalMemory(void)
{
	return qtrue;
}

qboolean Sys_RandomBytes(byte *string, int len)
{
	return qfalse;
}

void Sys_SetEnv(const char *name, const char *value)
{
	if (value && value[0])
		setenv(name, value, 1);
	else
		unsetenv(name);
}

qboolean Sys_WritePIDFile(void)
{
	return qfalse;
}

dialogResult_t Sys_Dialog(dialogType_t type, const char *message, const char *title)
{
	static char buf[MAXPRINTMSG];

	Com_sprintf(buf, sizeof(buf), "%s: %s\n", title, message);
	Sys_Print(buf);
	return DR_OK;
}

void *QDECL Sys_LoadDll(const char *name, qboolean useSystemLib)
{
	return NULL;
}

void Sys_Print(const char *msg)
{
	if (!gc_console_hidden)
		fputs(msg, stdout);
#ifdef GC_DEBUG
	SYS_Report("%s", msg);
	if (gc_log) {
		fputs(msg, gc_log);
		fflush(gc_log);
	}
#endif
}

void QDECL Sys_Error(const char *error, ...)
{
	static char buf[MAXPRINTMSG];
	va_list ap;

	va_start(ap, error);
	Q_vsnprintf(buf, sizeof(buf), error, ap);
	va_end(ap);

	GC_InitConsole();

	if (gc_console_hidden) {
		/* GX_AbortFrame, not GX_DrawDone: a fatal error may leave the GP hung. */
		GX_AbortFrame();
		gc_console_hidden = qfalse;
		printf("\x1b[2J");
	}
	VIDEO_SetNextFramebuffer(gc_console_xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	printf("\n\n=== FATAL ERROR ===\n");
	printf("%s\n", buf);
	printf("===================\nPress START to exit.\n");

#ifdef GC_DEBUG
	SYS_Report("%s\n", buf);
	if (gc_log) {
		fprintf(gc_log, "Sys_Error: %s\n", buf);
		fflush(gc_log);
		fclose(gc_log);
		gc_log = NULL;
	}
#endif

	for (;;) {
		PAD_ScanPads();
		if (PAD_ButtonsDown(0) & PAD_BUTTON_START)
			exit(1);
		VIDEO_WaitVSync();
	}
}

void Sys_Quit(void)
{
#ifdef GC_DEBUG
	if (gc_log) {
		fclose(gc_log);
		gc_log = NULL;
	}
#endif
	exit(0);
}
