#include <gccore.h>
#include <ogc/dvd.h>
#include <sys/iosupport.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../qcommon/q_shared.h"

#include "gc_dvd.h"

#define GC_DVD_WINDOW		32768
#define GC_DVD_ENTRY_SIZE	12
#define GC_DVD_ROOT			0

typedef struct {
	u32 entry;
	u32 offset;
	u32 size;
} gc_dvd_file_t;

typedef struct {
	u32 dir;
	u32 next;
} gc_dvd_dir_t;

static u8 *dvd_fst;
static u32 dvd_fst_entries;
static u8 *dvd_window;
static u64 dvd_window_base;
static u32 dvd_window_len;
static dvdcmdblk dvd_block;

static const u8 *fst_entry(u32 entry)
{
	return dvd_fst + entry * GC_DVD_ENTRY_SIZE;
}

static qboolean fst_is_dir(u32 entry)
{
	return fst_entry(entry)[0] == 1;
}

static u32 fst_word(u32 entry, int word)
{
	const u8 *p = fst_entry(entry) + 4 + word * 4;

	return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}

static const char *fst_name(u32 entry)
{
	const u8 *p = fst_entry(entry);
	u32 off = ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];

	return (const char *)(dvd_fst + dvd_fst_entries * GC_DVD_ENTRY_SIZE + off);
}

static int fst_child(u32 dir, const char *name)
{
	u32 end = fst_word(dir, 1);
	u32 i = dir + 1;

	while (i < end) {
		if (Q_stricmp(fst_name(i), name) == 0)
			return (int)i;
		i = fst_is_dir(i) ? fst_word(i, 1) : i + 1;
	}

	return -1;
}

static qboolean fst_resolve(const char *path, u32 *entry)
{
	const char *p = path;
	char name[256];
	u32 cur = GC_DVD_ROOT;

	if (!p)
		return qfalse;

	if (!Q_stricmpn(p, "dvd:", 4))
		p += 4;

	while (*p == '/')
		p++;

	if (!*p) {
		*entry = GC_DVD_ROOT;
		return qtrue;
	}

	for (;;) {
		u32 len = 0;
		int child;

		while (*p && *p != '/') {
			if (len + 1 >= sizeof(name))
				return qfalse;
			name[len++] = *p++;
		}
		name[len] = '\0';

		while (*p == '/')
			p++;

		if (len == 0 || !strcmp(name, "."))
			continue;

		child = fst_child(cur, name);
		if (child < 0)
			return qfalse;

		if (*p) {
			if (!fst_is_dir(child))
				return qfalse;
			cur = (u32)child;
			continue;
		}

		*entry = (u32)child;
		return qtrue;
	}
}

static qboolean dvd_read(void *dst, u64 offset, u32 len)
{
	u8 *out = dst;

	while (len) {
		u64 base = offset & ~(u64)(GC_DVD_WINDOW - 1);
		u32 in = (u32)(offset - base);
		u32 chunk = GC_DVD_WINDOW - in;

		if (chunk > len)
			chunk = len;

		if (dvd_window_base != base || dvd_window_len < in + chunk) {
			u32 readlen = (in + chunk + 31) & ~31u;

			if (readlen > GC_DVD_WINDOW)
				readlen = GC_DVD_WINDOW;
			DCInvalidateRange(dvd_window, readlen);
			if (DVD_ReadPrio(&dvd_block, dvd_window, readlen, (s64)base, 2) != (s32)readlen)
				return qfalse;
			dvd_window_base = base;
			dvd_window_len = readlen;
		}

		memcpy(out, dvd_window + in, chunk);
		out += chunk;
		offset += chunk;
		len -= chunk;
	}

	return qtrue;
}

static void dvd_set_stat(u32 entry, struct stat *st)
{
	memset(st, 0, sizeof(*st));
	st->st_ino = entry;
	if (fst_is_dir(entry)) {
		st->st_mode = S_IFDIR | 0555;
	} else {
		st->st_mode = S_IFREG | 0444;
		st->st_size = fst_word(entry, 1);
		st->st_blksize = GC_DVD_WINDOW;
	}
}

static int dvd_open_r(struct _reent *r, void *fileStruct, const char *path,
                      int flags, int mode)
{
	gc_dvd_file_t *file = fileStruct;
	u32 entry;

	if (flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_APPEND)) {
		r->_errno = EROFS;
		return -1;
	}

	if (!fst_resolve(path, &entry) || fst_is_dir(entry)) {
		r->_errno = ENOENT;
		return -1;
	}

	file->entry = entry;
	file->offset = 0;
	file->size = fst_word(entry, 1);
	return 0;
}

static int dvd_close_r(struct _reent *r, void *fd)
{
	return 0;
}

static ssize_t dvd_read_r(struct _reent *r, void *fd, char *ptr, size_t len)
{
	gc_dvd_file_t *file = fd;
	u32 left;
	u64 offset;

	if (file->offset >= file->size)
		return 0;

	left = file->size - file->offset;
	if (len > left)
		len = left;

	offset = fst_word(file->entry, 0) + file->offset;
	if (!dvd_read(ptr, offset, len)) {
		r->_errno = EIO;
		return -1;
	}

	file->offset += len;
	return len;
}

static off_t dvd_seek_r(struct _reent *r, void *fd, off_t pos, int dir)
{
	gc_dvd_file_t *file = fd;
	off_t offset;

	switch (dir) {
	case SEEK_SET:
		offset = pos;
		break;
	case SEEK_CUR:
		offset = (off_t)file->offset + pos;
		break;
	case SEEK_END:
		offset = (off_t)file->size + pos;
		break;
	default:
		r->_errno = EINVAL;
		return -1;
	}

	if (offset < 0 || offset > (off_t)file->size) {
		r->_errno = EINVAL;
		return -1;
	}

	file->offset = (u32)offset;
	return offset;
}

static int dvd_fstat_r(struct _reent *r, void *fd, struct stat *st)
{
	gc_dvd_file_t *file = fd;

	dvd_set_stat(file->entry, st);
	return 0;
}

static int dvd_stat_r(struct _reent *r, const char *file, struct stat *st)
{
	u32 entry;

	if (!fst_resolve(file, &entry)) {
		r->_errno = ENOENT;
		return -1;
	}

	dvd_set_stat(entry, st);
	return 0;
}

static DIR_ITER *dvd_diropen_r(struct _reent *r, DIR_ITER *dirState,
                               const char *path)
{
	gc_dvd_dir_t *dir = dirState->dirStruct;
	u32 entry;

	if (!fst_resolve(path, &entry) || !fst_is_dir(entry)) {
		r->_errno = ENOENT;
		return NULL;
	}

	dir->dir = entry;
	dir->next = entry + 1;
	return dirState;
}

static int dvd_dirreset_r(struct _reent *r, DIR_ITER *dirState)
{
	gc_dvd_dir_t *dir = dirState->dirStruct;

	dir->next = dir->dir + 1;
	return 0;
}

static int dvd_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename,
                         struct stat *filestat)
{
	gc_dvd_dir_t *dir = dirState->dirStruct;
	u32 end = fst_word(dir->dir, 1);
	u32 i = dir->next;

	if (i >= end) {
		r->_errno = ENOENT;
		return -1;
	}

	strcpy(filename, fst_name(i));
	dvd_set_stat(i, filestat);
	dir->next = fst_is_dir(i) ? fst_word(i, 1) : i + 1;
	return 0;
}

static int dvd_dirclose_r(struct _reent *r, DIR_ITER *dirState)
{
	return 0;
}

static const devoptab_t dvd_devoptab = {
	.name = "dvd",
	.structSize = sizeof(gc_dvd_file_t),
	.open_r = dvd_open_r,
	.close_r = dvd_close_r,
	.read_r = dvd_read_r,
	.seek_r = dvd_seek_r,
	.fstat_r = dvd_fstat_r,
	.stat_r = dvd_stat_r,
	.dirStateSize = sizeof(gc_dvd_dir_t),
	.diropen_r = dvd_diropen_r,
	.dirreset_r = dvd_dirreset_r,
	.dirnext_r = dvd_dirnext_r,
	.dirclose_r = dvd_dirclose_r,
};

qboolean GC_DVD_Mount(void)
{
	u8 header[0x440] __attribute__((aligned(32)));
	u32 fst_off, fst_size;

	dvd_window = memalign(32, GC_DVD_WINDOW);
	if (!dvd_window)
		return qfalse;
	dvd_window_base = ~(u64)0;
	dvd_window_len = 0;

	DVD_Init();
	if (DVD_Mount() < 0)
		return qfalse;

	if (!dvd_read(header, 0, sizeof(header)))
		return qfalse;

	fst_off = ((u32)header[0x424] << 24) | ((u32)header[0x425] << 16) |
	          ((u32)header[0x426] << 8) | header[0x427];
	fst_size = ((u32)header[0x428] << 24) | ((u32)header[0x429] << 16) |
	           ((u32)header[0x42A] << 8) | header[0x42B];

	if (fst_size < GC_DVD_ENTRY_SIZE)
		return qfalse;

	dvd_fst = memalign(32, (fst_size + 31) & ~31u);
	if (!dvd_fst)
		return qfalse;

	if (!dvd_read(dvd_fst, fst_off, fst_size))
		return qfalse;

	dvd_fst_entries = fst_word(GC_DVD_ROOT, 1);
	if (dvd_fst_entries < 1 ||
	    dvd_fst_entries * GC_DVD_ENTRY_SIZE > fst_size)
		return qfalse;

	if (AddDevice(&dvd_devoptab) != 0)
		return qfalse;

	return qtrue;
}
