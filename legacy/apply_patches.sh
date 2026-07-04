#!/bin/bash
# apply_patches.sh
#
# DEPRECATED — kept only for historical reference and to (re-)generate the
# patched upstream tree if needed. The Wii port now ships with the patched
# ioQ3 source vendored in this repo under code/{qcommon,client,server,...}/.
# Building no longer requires running this script.
#
# To regenerate the vendored tree from a fresh ioq3 clone at ../ioq3:
#   bash apply_patches.sh && cp -r ../ioq3/code/{qcommon,client,server,botlib,renderergl1,renderercommon} code/

IOQ3="$( cd .. && pwd )/ioq3"
QPLAT="$IOQ3/code/qcommon/q_platform.h"

if [ ! -f "$QPLAT" ]; then
    echo "ERROR: Cannot find $QPLAT"
    echo "Make sure ioq3 is cloned alongside ioquake3-wii/"
    exit 1
fi

# ---- q_platform.h: GEKKO/Wii block ----
if grep -q "GEKKO" "$QPLAT"; then
    echo "q_platform.h GEKKO block already applied — skipping."
else
    echo "Patching $QPLAT ..."
    python3 - "$QPLAT" <<'PYEOF'
import sys
path = sys.argv[1]
with open(path, 'r') as f:
    src = f.read()

WII_BLOCK = r"""
//===========================================================================
// Nintendo Wii (Gekko/Broadway PowerPC) — ioquake3-wii port
//===========================================================================
#elif defined(GEKKO)

#define OS_STRING "wii"
#define OS_LITTLE_ENDIAN_UNDEF  // Wii is big-endian; do not set OS_LITTLE_ENDIAN
#undef  Q3_LITTLE_ENDIAN
#define Q3_BIG_ENDIAN

#define ARCH_STRING "ppc"
#define PATH_SEP    '/'
#define DLL_EXT     ".so"   // unused — no dlopen on Wii

#ifndef ID_INLINE
#  define ID_INLINE __inline__
#endif

// No pthread, no dlopen, no mmap on Wii
#define IOAPI_NO_64BIT

"""

marker = '#else\n\n\t#error "Operating system not supported"'
if marker not in src:
    marker = '#else\n\n#error "Operating system not supported"'
if marker not in src:
    lines = src.split('\n')
    for i, line in enumerate(lines):
        if '"Operating system not supported"' in line:
            for j in range(i, max(i-5, 0), -1):
                if lines[j].strip() == '#else':
                    lines.insert(j, WII_BLOCK)
                    break
            break
    src = '\n'.join(lines)
else:
    src = src.replace(marker, WII_BLOCK + marker)

with open(path, 'w') as f:
    f.write(src)
print("q_platform.h patched.")
PYEOF
fi

# ---- qcommon.h: #ifndef guards for Wii memory overrides ----
QCOMMON_H="$IOQ3/code/qcommon/qcommon.h"
if grep -q "ifndef PACKET_BACKUP" "$QCOMMON_H"; then
    echo "qcommon.h #ifndef guards already applied — skipping."
else
    sed -i 's/^#define\tPACKET_BACKUP\t32/#ifndef PACKET_BACKUP\n#define\tPACKET_BACKUP\t32\n#endif/' "$QCOMMON_H"
    sed -i 's/^#define\tPACKET_MASK\t\t(PACKET_BACKUP-1)/#ifndef PACKET_MASK\n#define\tPACKET_MASK\t\t(PACKET_BACKUP-1)\n#endif/' "$QCOMMON_H"
    sed -i 's/^#define\tMAX_RELIABLE_COMMANDS\t64/#ifndef MAX_RELIABLE_COMMANDS\n#define\tMAX_RELIABLE_COMMANDS\t64\n#endif/' "$QCOMMON_H"
    sed -i 's/^#define MAX_DOWNLOAD_WINDOW\t\t48/#ifndef MAX_DOWNLOAD_WINDOW\n#define MAX_DOWNLOAD_WINDOW\t\t48\n#endif/' "$QCOMMON_H"
    echo "qcommon.h patched."
fi

# ---- sv_init.c: move large systemInfo array to BSS ----
SV_INIT="$IOQ3/code/server/sv_init.c"
if grep -q "static char.*systemInfo\[16384\]" "$SV_INIT"; then
    echo "sv_init.c systemInfo already static — skipping."
else
    sed -i 's/char\s*systemInfo\[16384\]/static char systemInfo[16384]/' "$SV_INIT"
    echo "sv_init.c patched (systemInfo -> static)."
fi

# ---- tr_main.c: rename ri to avoid symbol collision ----
TR_MAIN="$IOQ3/code/renderergl1/tr_main.c"
if grep -q "tr_main_ri_unused" "$TR_MAIN"; then
    echo "tr_main.c ri rename already applied — skipping."
else
    sed -i 's/^refimport_t\s*ri\s*;/refimport_t tr_main_ri_unused;/' "$TR_MAIN"
    echo "tr_main.c patched."
fi

# ---- tr_init.c: rename GetRefAPI to avoid symbol collision ----
TR_INIT="$IOQ3/code/renderergl1/tr_init.c"
if grep -q "tr_init_GetRefAPI_unused" "$TR_INIT"; then
    echo "tr_init.c GetRefAPI rename already applied — skipping."
else
    sed -i 's/Q_EXPORT refexport_t\* QDECL GetRefAPI/Q_EXPORT refexport_t* QDECL tr_init_GetRefAPI_unused/g' "$TR_INIT"
    sed -i 's/refexport_t \*GetRefAPI/refexport_t *tr_init_GetRefAPI_unused/g' "$TR_INIT"
    echo "tr_init.c patched."
fi

# ---- snd_local.h: #ifndef guards for MAX_RAW_STREAMS / MAX_RAW_SAMPLES ----
SND_LOCAL="$IOQ3/code/client/snd_local.h"
if grep -q "ifndef MAX_RAW_STREAMS" "$SND_LOCAL"; then
    echo "snd_local.h guards already applied — skipping."
else
    sed -i 's/^#define\tMAX_RAW_SAMPLES\t16384/#ifndef MAX_RAW_SAMPLES\n#define\tMAX_RAW_SAMPLES\t16384\n#endif/' "$SND_LOCAL"
    sed -i 's/^#define MAX_RAW_STREAMS.*$/#ifndef MAX_RAW_STREAMS\n#define MAX_RAW_STREAMS (MAX_CLIENTS * 2 + 1)\n#endif/' "$SND_LOCAL"
    echo "snd_local.h patched."
fi

# ---- snd_codec.c: #ifdef -> #if so =0 disables codecs correctly ----
SND_CODEC="$IOQ3/code/client/snd_codec.c"
if grep -q "#if USE_CODEC_VORBIS" "$SND_CODEC"; then
    echo "snd_codec.c already patched — skipping."
else
    sed -i 's/#ifdef USE_CODEC_VORBIS/#if USE_CODEC_VORBIS/' "$SND_CODEC"
    sed -i 's/#ifdef USE_CODEC_OPUS/#if USE_CODEC_OPUS/' "$SND_CODEC"
    echo "snd_codec.c patched."
fi

# ---- common.c: #ifndef guards on hunk/zone megs (so wii_platform.h wins) ----
COMMON_C="$IOQ3/code/qcommon/common.c"
if grep -q "ifndef DEF_COMZONEMEGS" "$COMMON_C"; then
    echo "common.c hunk/zone guards already applied — skipping."
else
    sed -i 's|^#define MIN_DEDICATED_COMHUNKMEGS 1$|#ifndef MIN_DEDICATED_COMHUNKMEGS\n#define MIN_DEDICATED_COMHUNKMEGS 1\n#endif|' "$COMMON_C"
    sed -i 's|^#define DEF_COMHUNKMEGS \t128$|#ifndef DEF_COMHUNKMEGS\n#define DEF_COMHUNKMEGS 128\n#endif|' "$COMMON_C"
    sed -i 's|^#define MIN_COMHUNKMEGS\t\tDEF_COMHUNKMEGS$|#ifndef MIN_COMHUNKMEGS\n#define MIN_COMHUNKMEGS\t\tDEF_COMHUNKMEGS\n#endif|' "$COMMON_C"
    sed -i 's|^#define DEF_COMZONEMEGS\t\t48$|#ifndef DEF_COMZONEMEGS\n#define DEF_COMZONEMEGS 48\n#endif|' "$COMMON_C"
    echo "common.c hunk/zone megs guards applied."
fi

# ---- net_ip.c: Wii guard so NET_Sleep doesn't select() with empty fd_set ----
NET_IP_C="$IOQ3/code/qcommon/net_ip.c"
if grep -q "WII_NET_SLEEP_GUARD" "$NET_IP_C"; then
    echo "net_ip.c Wii NET_Sleep guard already applied — skipping."
else
    python3 - "$NET_IP_C" <<'PYEOF'
import sys
path = sys.argv[1]
with open(path) as f: src = f.read()
OLD = '''#ifdef _WIN32
\tif(highestfd == INVALID_SOCKET)
\t{
\t\t// windows ain't happy when select is called without valid FDs
\t\tSleepEx(msec, 0);
\t\treturn;
\t}
#endif'''
NEW = OLD + '''
\t/* WII_NET_SLEEP_GUARD: Wii libogc select() hangs with no FDs in set.
\t * Match the Windows behavior — bail out cleanly. */
#ifdef GEKKO
\tif(highestfd == INVALID_SOCKET) return;
#endif'''
if OLD in src:
    src = src.replace(OLD, NEW, 1)
    with open(path,'w') as f: f.write(src)
    print("net_ip.c NET_Sleep Wii guard applied.")
else:
    print("WARNING: couldn't find NET_Sleep _WIN32 block in net_ip.c.")
PYEOF
fi

echo ""
echo "Done. To vendor the patched tree into this repo (one-time):"
echo "  cp -r ../ioq3/code/{qcommon,client,server,botlib,renderergl1,renderercommon} code/"
echo "  then update Makefile IOQ3_DIR to 'code' and stop using this script."
