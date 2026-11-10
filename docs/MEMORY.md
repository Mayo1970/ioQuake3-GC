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
