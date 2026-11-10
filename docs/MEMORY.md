# Memory baseline

## 1. How to measure

- Docker image: `devkitpro/devkitppc:20261102`.
- Base: `main` at `3c459d2d9367a98d9fd389b4743ffe4320721132` (Merge pull request #5 from Mayo1970/task/QGC-8-engine-core).
- Release build, from the repository root:

```
bash tools/docker-make.sh clean
bash tools/docker-make.sh
MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/src" -w /src devkitpro/devkitppc:20261102 bash -c 'export PATH=/opt/devkitpro/devkitPPC/bin:$PATH; powerpc-eabi-size build/ioquake3_gc.elf'
MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/src" -w /src devkitpro/devkitppc:20261102 bash -c 'export PATH=/opt/devkitpro/devkitPPC/bin:$PATH; powerpc-eabi-size -A build/ioquake3_gc.elf'
MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/src" -w /src devkitpro/devkitppc:20261102 bash -c 'export PATH=/opt/devkitpro/devkitPPC/bin:$PATH; powerpc-eabi-nm --size-sort -S -r build/ioquake3_gc.elf'
```

- Debug build used only to resolve source lines:

```
bash tools/docker-make.sh clean
bash tools/docker-make.sh DEBUG=1
MSYS_NO_PATHCONV=1 docker run --rm -v "$(pwd -W):/src" -w /src devkitpro/devkitppc:20261102 bash -c 'export PATH=/opt/devkitpro/devkitPPC/bin:$PATH; powerpc-eabi-nm -l --size-sort -S -r build/ioquake3_gc.elf'
```

## 2. Binary

Berkeley format, release ELF:

```
   text	   data	    bss	    dec	    hex	filename
 914326	 254652	25711264	26880242	19a28f2	build/ioquake3_gc.elf
```

`powerpc-eabi-size -A`, without the `.debug_*` and `.comment` lines (1 MB = 1048576 bytes):

| section | bytes | MB |
|---|---:|---:|
| `.init` | 148 | 0.00 |
| `.text` | 896224 | 0.85 |
| `.rodata` | 100032 | 0.10 |
| `.sdata2` | 6 | 0.00 |
| `.eh_frame_hdr` | 17948 | 0.02 |
| `.eh_frame` | 133264 | 0.13 |
| `.ctors` | 12 | 0.00 |
| `.dtors` | 8 | 0.00 |
| `.data` | 21228 | 0.02 |
| `.sdata` | 108 | 0.00 |
| `.sbss` | 452 | 0.00 |
| `.bss` | 25710812 | 24.52 |
| `.gnu.attributes` | 18 | 0.00 |

## 3. Top 40 BSS symbols

Sizes from the release ELF. Source file from the `DEBUG=1` ELF, path relative to `code/`.

| Rank | Symbol | Bytes | KB | Source file |
|---:|---|---:|---:|---|
| 1 | `s_rawsamples` | 16908288 | 16512.0 | client/snd_dma.c:88 |
| 2 | `cin` | 2426388 | 2369.5 | client/cl_cin.c:130 |
| 3 | `cls` | 893132 | 872.2 | client/cl_main.c:127 |
| 4 | `cl` | 679024 | 663.1 | client/cl_main.c:125 |
| 5 | `facets` | 655360 | 640.0 | qcommon/cm_patch.c:421 |
| 6 | `buckets` | 655360 | 640.0 | server/sv_main.c:378 |
| 7 | `vq8` | 524288 | 512.0 | client/cl_cin.c:74 |
| 8 | `s_gcMainStack` | 524288 | 512.0 | sys/gc_main.c:13 |
| 9 | `s_knownSfx` | 393216 | 384.0 | client/snd_dma.c:73 |
| 10 | `sv` | 312388 | 305.1 | server/sv_main.c:30 |
| 11 | `clc` | 173628 | 169.6 | client/cl_main.c:126 |
| 12 | `cvar_indexes` | 147456 | 144.0 | qcommon/cvar.c:32 |
| 13 | `cl_serverStatusList` | 131904 | 128.8 | client/cl_main.c:152 |
| 14 | `vq4` | 131072 | 128.0 | client/cl_cin.c:73 |
| 15 | `cmd_text_buf` | 131072 | 128.0 | qcommon/cmd.c:38 |
| 16 | `svs` | 122976 | 120.1 | server/sv_main.c:29 |
| 17 | `con` | 65608 | 64.1 | client/cl_console.c:58 |
| 18 | `string.2` | 64000 | 62.5 | qcommon/q_shared.c:990 |
| 19 | `msgHuff` | 57400 | 56.1 | qcommon/msg.c:25 |
| 20 | `loopSounds` | 57344 | 56.0 | client/snd_dma.c:84 |
| 21 | `loopbacks` | 44944 | 43.9 | qcommon/net_chan.c:436 |
| 22 | `pcmCaptureBuffer.0` | 44100 | 43.1 | client/cl_avi.c:520 |
| 23 | `serverBans` | 40960 | 40.0 | server/sv_main.c:65 |
| 24 | `planes` | 40960 | 40.0 | qcommon/cm_patch.c:418 |
| 25 | `cl_pinglist` | 34048 | 33.2 | client/cl_main.c:140 |
| 26 | `vq2` | 32768 | 32.0 | client/cl_cin.c:72 |
| 27 | `paintbuffer` | 32768 | 32.0 | client/snd_mix.c:30 |
| 28 | `com_pushedEvents` | 24576 | 24.0 | qcommon/common.c:1899 |
| 29 | `cinTable` | 19520 | 19.1 | client/cl_cin.c:131 |
| 30 | `fsh` | 18432 | 18.0 | qcommon/files.c:286 |
| 31 | `value.0` | 16384 | 16.0 | qcommon/q_shared.c:1044 |
| 32 | `s_excptStack` | 16384 | 16.0 | libogc |
| 33 | `fs_serverReferencedPaks` | 16384 | 16.0 | qcommon/files.c:300 |
| 34 | `fs_serverReferencedPakNames` | 16384 | 16.0 | qcommon/files.c:301 |
| 35 | `fs_serverPaks` | 16384 | 16.0 | qcommon/files.c:294 |
| 36 | `fs_serverPakNames` | 16384 | 16.0 | qcommon/files.c:295 |
| 37 | `cmd_tokenized` | 9216 | 9.0 | qcommon/cmd.c:351 |
| 38 | `historyEditLines` | 8576 | 8.4 | client/cl_keys.c:30 |
| 39 | `bspworld` | 8208 | 8.0 | botlib/be_aas_bspq3.c:87 |
| 40 | `string.2` | 8192 | 8.0 | qcommon/msg.c:472 |

Sum of the 40 sizes: 25519764 bytes (24.34 MB).
Total `.bss`: 25710812 bytes (24.52 MB).

## 4. Runtime measurements

| Line | Value |
|---|---|
| boot Arena1 | 39493632 (37.66 MB) |
| Arena1 after `Com_Init` | 876544 (0.84 MB) |
| Arena1 after map load | 765952 (0.73 MB) |
| hunk total | 25165824 (24.00 MB) |
| hunk used (low + high permanent) | 15779616 (15.05 MB): low 10630624, high 5148992 |
| zone total | 12582912 (12.00 MB) |
| zone used | 1661572 (1.58 MB) in 290 blocks: botlib 436572, other 1225000 |
| small zone used | 58520 (0.06 MB) |

Measured by the supervisor: `DEBUG=1 GC_MEM1_MB=64`, headless Dolphin 5.0-17995 with the MEM1 override at 64 MB (the stock 24 MB cannot hold the 24.52 MB BSS).
Map q3dm1, QVM interpreter for qagame (4431296 bytes of hunk), ui (2348544) and cgame (5773312), null renderer, null sound.
Arena1 values depend on the 64 MB override; the hunk and zone values do not.

## 5. Budget for 24 MB

1 MB = 1048576 bytes.

| Line | Measured now (MB) | Target (MB) | Notes |
|---|---:|---|---|
| binary (text + data + rodata) | 1.11 | 3.00 | Berkeley text + data: 914326 + 254652 = 1168978 bytes (includes `.rodata` and `.eh_frame`) |
| BSS (incl. the 512 KB main stack) | 24.52 | 3.00 | Berkeley bss 25711264 bytes (`.bss` + `.sbss`); `s_gcMainStack` is 524288 bytes |
| XFB | 0.59 | 1.20 | 1 framebuffer x 640 x 480 x 2 bytes (NTSC); PAL height 576 gives 0.70 MB |
| GX FIFO | 0.00 | 0.25 | no GX yet |
| hunk | 24.00 | 7.50 | `com_hunkMegs` default 24; 15.05 MB used on q3dm1 with QVMs (section 4) |
| zone | 12.00 | 2.50 | `com_zoneMegs` default 12; 1.58 MB used on q3dm1 (section 4) |
| sound | 0.00 | 1.50 | `com_soundMegs` default is 8 MB; the null sound driver allocates nothing |
| textures | 0.00 | 3.00 | null renderer |
| other (libogc, malloc, OS low memory) | n/a | 1.00 | not measured; libogc BSS is inside the BSS line |
| total | 62.22 | 22.95 | sum of measured column: 65243378 bytes |

24 MB = 25165824 bytes. Targets set by Matt on 2026-11-10: sum 22.95 MB, 1.05 MB slack.
Binary and BSS targets include the native modules (QGC-10) and the GX renderer. XFB target is 2 buffers. Sound target is the MEM1 part; the rest goes to ARAM.

## 6. Notes

- `s_rawsamples`, 16908288 bytes (16.13 MB): `portable_samplepair_t s_rawsamples[MAX_RAW_STREAMS][MAX_RAW_SAMPLES]` (`snd_dma.c:88`), the raw stream rings (stream 0: music and cinematic audio; the others: VoIP and entity streams); `MAX_RAW_STREAMS` is `MAX_CLIENTS * 2 + 1` = 129 and `MAX_RAW_SAMPLES` is 16384.
- `cin`, 2426388 bytes (2.31 MB): `static cinematics_t cin` (`cl_cin.c:130`), the RoQ cinematic decoder state; its `linbuf[512 * 512 * 4 * 2]` alone is 2097152 bytes.

## 7. Size changes

| Task | Change | text | data | bss | dec |
|---|---|---:|---:|---:|---:|
| QGC-11 | before (main 1d86ee3) | 1490694 | 409164 | 29105728 | 31005586 |
| QGC-11 | after: interpreter and QVM loader out | 1485686 | 407836 | 29105728 | 30999250 |
| QGC-11 | delta | -5008 | -1328 | 0 | -6336 |
| QGC-12 | before (main 6c81c79) | 1485686 | 407836 | 29105728 | 30999250 |
| QGC-12 | after: BSS diet | 1486238 | 407924 | 6832096 | 8726258 |
| QGC-12 | delta | +552 | +88 | -22273632 | -22272992 |
| QGC-13 | before (main 590cea7) | 1486238 | 407924 | 6832096 | 8726258 |
| QGC-13 | after: hunk clamp, sound megs, CM lump load | 1488350 | 408400 | 6832096 | 8728846 |
| QGC-13 | delta | +2112 | +476 | 0 | +2588 |

`.text` 1455680 (-4960) and `.rodata` 152928 (-936) from `size -A`.

## 8. BSS cuts (QGC-12)

Bytes from release `powerpc-eabi-nm -S`; `cin` disappears as a symbol once it moves to `malloc`.

| Symbol | File | Change | Before | After | Saved |
|---|---|---|---:|---:|---:|
| `s_rawsamples` | client/snd_dma.c | `MAX_RAW_STREAMS` 129 -> 1 | 16908288 | 131072 | 16777216 |
| `s_rawend` | client/snd_dma.c | `MAX_RAW_STREAMS` 129 -> 1 | 516 | 4 | 512 |
| `cls` | client/cl_main.c | `MAX_GLOBAL_SERVERS` 4096 -> 256 | 893132 | 109772 | 783360 |
| `cl_serverStatusList` | client/cl_main.c | `MAX_SERVERSTATUSREQUESTS` 16 -> 4 | 131904 | 32976 | 98928 |
| `buckets` | server/sv_main.c | `MAX_BUCKETS` 16384 -> 1024 | 655360 | 40960 | 614400 |
| `cvar_indexes` | qcommon/cvar.c | `MAX_CVARS` 2048 -> 1024 | 147456 | 73728 | 73728 |
| `cmd_text_buf` | qcommon/cmd.c | `MAX_CMD_BUFFER` 131072 -> 16384 | 131072 | 16384 | 114688 |
| `planes` | qcommon/cm_patch.c | static array -> hunk temp pointer | 40960 | 4 | 40956 |
| `facets` | qcommon/cm_patch.c | static array -> hunk temp pointer | 655360 | 4 | 655356 |
| `cin` | client/cl_cin.c | static struct -> `malloc` block | 2426388 | 0 | 2426388 |
| `vq2` | client/cl_cin.c | static array -> `malloc` block | 32768 | 4 | 32764 |
| `vq4` | client/cl_cin.c | static array -> `malloc` block | 131072 | 4 | 131068 |
| `vq8` | client/cl_cin.c | static array -> `malloc` block | 524288 | 4 | 524284 |
| **Total** | | | **22678564** | **404916** | **22273648** |

`cin` and the VQ tables are now one `malloc` block of 3114516 bytes while a cinematic plays.
`planes` and `facets` now use 696320 bytes of hunk temp memory during each patch build.

### Limits changed

| Limit | File | Upstream | GameCube | Why it is safe |
|---|---|---:|---:|---|
| `MAX_RAW_STREAMS` | client/snd_local.h | 129 | 1 | Extra streams are VoIP only; `USE_VOIP` is not built. Music and cinematic audio use stream 0. |
| `MAX_GLOBAL_SERVERS` | qcommon/q_shared.h | 4096 | 256 | Internet list only; q3_ui shows at most 128 global servers. |
| `MAX_SERVERSTATUSREQUESTS` | qcommon/q_shared.h | 16 | 4 | Server browser status queries in flight; not used in a match. |
| `MAX_BUCKETS` | server/sv_main.c | 16384 | 1024 | Rate-limit records for connectionless packets per address; a LAN host sees few addresses. |
| `MAX_CVARS` | qcommon/cvar.c | 2048 | 1024 | id Quake III 1.32 value. |
| `MAX_CMD_BUFFER` | qcommon/cmd.c | 131072 | 16384 | id Quake III 1.32 value. |

### Top 20 BSS symbols after QGC-12

| Rank | Symbol | Bytes | KB |
|---:|---|---:|---:|
| 1 | `g_entities` | 823296 | 804.0 |
| 2 | `cg_entities` | 733184 | 716.0 |
| 3 | `cl` | 679024 | 663.1 |
| 4 | `s_gcMainStack` | 524288 | 512.0 |
| 5 | `s_knownSfx` | 393216 | 384.0 |
| 6 | `sv` | 312388 | 305.1 |
| 7 | `memoryPool` | 262144 | 256.0 |
| 8 | `clc` | 173628 | 169.6 |
| 9 | `cgs` | 153812 | 150.2 |
| 10 | `cg_localEntities` | 149504 | 146.0 |
| 11 | `s_rawsamples` | 131072 | 128.0 |
| 12 | `memoryPool` | 131072 | 128.0 |
| 13 | `particles` | 126976 | 124.0 |
| 14 | `svs` | 122976 | 120.1 |
| 15 | `cg` | 117004 | 114.3 |
| 16 | `cls` | 109772 | 107.2 |
| 17 | `g_globalserverlist` | 97280 | 95.0 |
| 18 | `cvar_indexes` | 73728 | 72.0 |
| 19 | `cg_markPolys` | 73728 | 72.0 |
| 20 | `con` | 65608 | 64.1 |


## 9. Hunk and zone sizing (QGC-13)

### Defaults

| Setting | Upstream | GameCube | Why |
|---|---:|---:|---|
| `com_hunkMegs` default | 128 | 8 | Hunk target 7.5 MB (section 5), rounded up. The clamp below cuts it to what fits. |
| `com_hunkMegs` minimum | 56 | 6 | Highest measured peak is 4.66 MB (q3dm6). |
| `com_zoneMegs` default (also the minimum) | 24 | 3 | Highest measured zone peak is 2.53 MB (q3dm6). |
| `com_soundMegs` default | 8 | 1 | On `HW_DOL` the cvar counts megabytes (509 chunks of 2060 bytes). Upstream counts units of 1536 chunks, about 3 MB each. |

### Hunk clamp

`Com_InitHunkMemory` on `HW_DOL` keeps `GC_HUNK_RESERVE` (4980736 bytes: textures 3.00 + sound 1.50 + GX FIFO 0.25 MB) and `GC_HUNK_SLACK` (65536 bytes) of the free Arena1 out of the hunk.
If `com_hunkMegs` asks for more, the hunk gets the largest whole MB that fits and the console prints `com_hunkMegs <n> does not fit in free memory, allocating <m> megs.`
If even `MIN_COMHUNKMEGS` does not fit, `Com_Error( ERR_FATAL )`. The cvar value does not change.

Stock 24 MB check (`DEBUG=1`, no memory override, `com_hunkMegs 24`, q3dm1, 4 bots): `com_hunkMegs 24 does not fit in free memory, allocating 6 megs.`
Boot Arena1 15704064, after `Com_Init` 5398528, after the map load 5251072 bytes. The map loads and the bots play. The default 8 also falls back to 6 in stock memory.

### Hunk waste cut: collision map loads lump by lump

Upstream `CM_LoadMap` holds the whole BSP in hunk temp memory while collision loads. That was the hunk peak on every map.
On `HW_DOL` it reads one collision lump at a time (surfaces and drawverts together, for patches) and frees it after use.
The map checksum comes from a streamed MD4 pass (`Com_FileChecksum`), bit-equal to `Com_BlockChecksum` on the whole file: botlib accepts all four `.aas` files.
The lightmap, lightgrid, fog and index lumps are not read. Load time of q3dm6 and q3dm1 (server init to "entered the game") is within 1 s of the old path in Dolphin.

### Peaks

`DEBUG=1 GC_MEM1_MB=64`, Dolphin MEM1 override 64 MB, null renderer, null sound, `g_gametype 0`, 4 bots (`addbot sarge/anarki/visor/doom 2`), timelimit 3.
Peaks come from `meminfo` (`GC peak:` lines, debug builds only). The hunk peak resets on each `Hunk_Clear`; zone peaks never reset.

| Map | BSP bytes | Hunk peak before | Hunk peak after | Hunk in use after load | Zone peak | Small zone peak |
|---|---:|---:|---:|---:|---:|---:|
| q3dm1 | 1906708 | 3823504 (3.65 MB) | 3226464 (3.08 MB) | 3226464 | 2401748 (2.29 MB) | 58912 |
| q3dm6 | 4408212 | 7653456 (7.30 MB) | 4882576 (4.66 MB) | 4847744 | 2649268 (2.53 MB) | 60952 |
| q3dm17 | 2286376 | 3698500 (3.53 MB) | 2816640 (2.69 MB) | 2816640 | 2543952 (2.43 MB) | 58712 |
| q3tourney6 | 2040060 | 3505112 (3.34 MB) | 2781184 (2.65 MB) | 2781184 | 2457600 (2.34 MB) | 59104 |

"Before" is part 1 of QGC-13 (whole-BSP load); "after" is the lump load. q3dm1, q3dm6 and q3dm17 reach "Timelimit hit." with no error.
q3tourney6: the log stops about 30 s after the map load and the match does not reach the time limit. `main` (590cea7) does the same, so this is not a QGC-13 change; it is open for a later task.

### Largest hunk allocations on q3dm6 (after the cut)

| Bytes | Owner |
|---:|---|
| 1188264 + 2088 | hunk temp: drawverts + surfaces lumps during `CMod_LoadPatches` (freed) |
| 856768 | `CMod_LoadVisibility` (PVS) |
| 696336 | hunk temp per patch build: `planes` + `facets` (QGC-12, freed) |
| 426016 | `svs.snapshotEntities` (8 clients x 4 x 64 entities) |
| 282656 | botlib console message heap (`max_messages` 1024) |
| about 1.5 MB | botlib AAS lumps |
