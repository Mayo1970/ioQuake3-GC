# ioQuake3-GC

ioQuake3-GC is a port of [ioQuake3](https://github.com/ioquake/ioq3) to the
Nintendo GameCube. It runs Quake III Arena from a `.dol` file on a stock
console with 24 MB of main memory.

- Engine baseline: [old-mac-quake3](https://github.com/matthewdeaves/old-mac-quake3)
  `2f187c0` (ioquake3, 2013-01), vendored under `code/`.
- Renderer: the ioquake3 renderer with a native GX backend from
  [ioQuake3-wii](https://github.com/Mayo1970/ioQuake3-wii). No OpenGL layer.
- Game modules: `qagame`, `cgame` and `ui` are native PowerPC code, linked
  into the `.dol`. There is no QVM.
- Toolchain: devkitPPC and libogc 3.2.1. No Nintendo SDK material.
- Output: one `.dol`. Open Arena is planned after Quake III Arena.

## Status

**Pre-alpha.** No release exists yet.

| Area | State |
|---|---|
| Build in Docker | Works |
| Boot, menus, console, cinematics | Works on hardware |
| Maps with bots, stock 24 MB | 22 of 30 maps play on hardware (see [Map list](#map-list)) |
| GX renderer, video modes (480i, 576i, 480p) | Works on hardware |
| Sound effects (ADPCM in ARAM) | Works on hardware |
| Controller input | Works on hardware |
| Config save, quit, Reset button | Works on hardware |
| Music | Not done |
| Rumble | Not done |
| Network (Broadband Adapter) | Not done; loopback only |
| On-screen text input | Not done |
| Open Arena build | Not done |

## Building

All builds run in the official devkitPro Docker image. Do not install a
toolchain on the host.

1. Install Docker.
2. Pull the image:

   ```
   docker pull devkitpro/devkitppc:20261102
   ```

3. Build from the repository root:

   ```
   bash tools/docker-make.sh                        # Git Bash
   powershell -File tools/docker-make.ps1           # PowerShell
   ```

The output is `build/ioquake3_gc.dol`. The wrapper passes its arguments to
`make`.

| Target | Result |
|---|---|
| (none) | `build/ioquake3_gc.elf` and `build/ioquake3_gc.dol` |
| `dist` | Release layout in `dist/ioQuake3-GC/` |
| `iso` | A `.gcm` disc image for Dolphin smoke tests only. Delete it after the test. |
| `clean` | Removes `build/`, `dist/` and all disc images |

| Flag | Effect |
|---|---|
| `DEBUG=1` | Adds `-DGC_DEBUG -g`. The engine log and the `GC ...` memory reports work only in this build. |
| `GC_MEM1_MB=64` | Links for a 64 MB MEM1. Use it only with the Dolphin MEM1 override. |
| `DISC_ID=<id>` | Game ID of the disc image (default `GQ3E`) |
| `ISO_ROOT=<dir>` | Folder packed into the disc image by `make iso` |

`make` does not track `DEBUG`. Run `make clean` before you change a flag.

To install the `.dol` and the game data on an SD card, read
[INSTALLATION.md](INSTALLATION.md). You need your own Quake III Arena
`.pk3` files. This repository holds no game data.

## Main limits

The GameCube has 24 MB of main memory (MEM1) and 16 MB of audio memory
(ARAM). The CPU reads ARAM only through DMA. Stock ioquake3 asks for more
than 60 MB. All numbers below are bytes. 1 MB = 1048576 bytes.

### MEM1 (24 MB, 25165824 bytes)

#### Static: in the `.dol`

Release build, `main` at `15c3a42`, `powerpc-eabi-size`:

| Part | Bytes | Notes |
|---|---:|---|
| `text` | 1924072 | Code, `.rodata`, `.eh_frame` |
| `data` | 489252 | Initialized data |
| `bss` | 6299136 | All static tables, listed below |
| **Total** | **8712460** | 8.31 MB |

Largest BSS symbols (64 KB or more):

| Symbol | Bytes | Owner |
|---|---:|---|
| `g_entities` | 823296 | qagame entities |
| `cg_entities` | 733184 | cgame entities |
| `cl` | 679024 | Client state (snapshots, parse entities) |
| `s_gcMainStack` | 458752 | Main thread stack (448 KB, peak 369840) |
| `sv` | 312388 | Server state |
| `clc` | 173628 | Client connection |
| `tr` | 170540 | Renderer globals |
| `cgs` | 153812 | cgame static state |
| `s_rawsamples` | 131072 | Raw sound stream (cinematic audio) |
| `scratch` | 131072 | Draw-surface radix sort |
| `memoryPool` (qagame) | 131072 | qagame pool |
| `svs` | 122976 | Server static state |
| `cg` | 117004 | cgame frame state |
| `cls` | 109772 | Client static state |
| `tess` | 104048 | Renderer tesselator |
| `s_knownSfx` | 98304 | 1024 sound slots |
| `cvar_indexes` | 73728 | 1024 cvars |
| `s_gx_texobjs` | 65536 | GX texture objects |
| `memoryPool` (ui) | 65536 | q3_ui pool |

#### Dynamic: the heap (Arena1)

Allocated at boot or at renderer start. They stay for the whole session.

| Part | Bytes | Notes |
|---|---:|---|
| Hunk | 8388608 | `com_hunkMegs` 8 (minimum 6). The clamp gives a smaller hunk if 8 MB does not fit. |
| Zone | 2883584 | 2.75 MB, Quake3e size buckets |
| Small zone | 131072 | Peak about 38 KB |
| XFB 0 (console, reused by GX) | 614400 | 737280 in 576-line modes |
| XFB 1 | 614400 | 737280 in 576-line modes |
| GX FIFO | 262144 | |
| GX vertex ring | 262144 | 2 x 128 KB |
| Sound DMA ring | 65536 | 32 blocks of 2048, 32 kHz stereo |
| Sound decode cache | 131072 | 16 slots of 4096 samples |
| Sound staging | 12288 | ARAM read and prefetch |
| Sound chunk metadata | 36768 | 3064 chunks |
| MD3 frame buffers | 16000 | 2 x 8000, current and old frame |
| FAT cache | 131072 | 4 pages x 64 sectors, mounted SD device only |
| Disc window and FST | 32768 + FST | Disc image boot only |

Kept free by the hunk clamp: `GC_HUNK_RESERVE` 2621440 (textures 2.25 MB,
GX FIFO 0.25 MB) and `GC_HUNK_SLACK` 65536.

#### Dynamic: per map

| Part | Size | Notes |
|---|---:|---|
| Textures | 1.6 to 2.1 MB per map | One `memalign` buffer per texture. `RGB565`, `RGB5A3` or `CMPR`. `r_picmip 2`, cap 256x256. |
| Hunk use | 3.4 to 7.9 MB peak | World, collision, botlib AAS, models, shaders, entities |
| Zone use | 1.7 to 2.3 MB peak | |

#### Dynamic: short-lived

| Part | Bytes | When |
|---|---:|---|
| Image decode buffer | up to 262144 | One image load, at the upload size |
| MD3 staging | 32768 | One MD3 load |
| Shader text staging | about 423000 | Renderer start, until the copy to ARAM |
| zlib inflate state | about 40000 | Each read from a `.pk3` |
| Cinematic block | 2065940 | Hunk temp memory, while a RoQ plays |
| Patch build (`planes` + `facets`) | 696320 | Hunk temp memory, per curved surface |
| BSP read windows | about 105000 | Hunk temp memory, during the world load |

The lowest free main memory during a map load is 288640 bytes (q3ctf1,
4 bots). Most failures in the map list come from a full or fragmented heap.

### ARAM (16 MB, 16777216 bytes)

| Region | Address | Bytes | Use |
|---|---|---:|---|
| OS reserve | `0x000000` | 16384 | libogc `AR_Init` |
| Models | `0x004000` | 9437184 | MD3 vertex frames from the bottom. The BSP light grid from the top (q3dm19: 3633376). |
| Shader text | `0x904000` | 1048576 | All `.shader` script text, compressed. 423073 used. |
| Sound | `0xA04000` | 6275072 | 4-bit ADPCM, 3064 chunks, about 392 s at 32 kHz |
| **Total** | | **16777216** | |

A model that does not fit in the model region fails as a whole.
The renderer reads MD3 frames and the light grid back through DMA.
The light grid uses an 8 KB cache in BSS.

## Map list

Results from a real GameCube, stock 24 MB, `DEBUG=1`, `devmap <map>`, bots
at skill 3. Bot count comes from `scripts/arenas.txt`. q3tourney maps run
as Tournament. CTF maps run as Capture the Flag with 4 bots.

**22 of 30 maps load and play.**

| Map | Bots | Result | Lowest free MEM1 | Cause of failure |
|---|---:|---|---:|---|
| q3dm0 | 1 | Plays | 594056 | |
| q3dm1 | 1 | Plays | 735712 | |
| q3dm2 | 1 | Plays | 542144 | |
| q3dm3 | 2 | Plays | 899728 | |
| q3dm4 | 3 | Plays | 743040 | |
| q3dm5 | 2 | Plays | 586808 | |
| q3dm6 | 5 | Plays | 476152 | |
| q3dm7 | 4 | Plays | 342056 | |
| q3dm8 | 4 | Plays | 592880 | |
| q3dm9 | 4 | **Fails** | | Heap full: zlib inflate state (`FS_Read: inflate error -4`) |
| q3dm10 | 3 | Plays | 635744 | |
| q3dm11 | 4 | **Fails** | | Heap: 262144 for `textures/sfx/fan.tga` (no picmip); largest block 217088 |
| q3dm12 | 6 | **Fails** | | Heap: 262144 for `fan.tga`; largest block 114688 |
| q3dm13 | 3 | Plays | 508288 | |
| q3dm14 | 4 | **Fails** | | Heap full: zlib inflate state |
| q3dm15 | 3 | **Fails** | | Heap fragmentation: 131072 needed, 207472 free, largest block 126976 |
| q3dm16 | 3 | Plays | 819152 | |
| q3dm17 | 3 | Plays | 857240 | |
| q3dm18 | 5 | **Fails** | | Heap full: zlib inflate state on the BSP |
| q3dm19 | 3 | Plays | 487088 | |
| q3tourney1 | 1 | Plays | 889392 | |
| q3tourney2 | 1 | Plays | 675632 | |
| q3tourney3 | 1 | Plays | 749288 | |
| q3tourney4 | 1 | Plays | 525960 | |
| q3tourney5 | 1 | Plays | 1030896 | |
| q3tourney6 | 1 | Plays | 619312 | |
| q3ctf1 | 4 | Plays | 288640 | |
| q3ctf2 | 4 | **Fails** | | Zone full: botlib needs 126868 more. It needs about 7 MB more hunk and 0.7 MB more zone. |
| q3ctf3 | 4 | **Fails** | | Hunk full: world surfaces |
| q3ctf4 | 4 | Plays | 711104 | |

Failing maps: q3dm9, q3dm11, q3dm12, q3dm14, q3dm15, q3dm18, q3ctf2,
q3ctf3. With fewer bots, a failing map can still fail: the peak is in the
map load.

## Known limits

- **Memory.** 22 of 30 maps load and play. 8 maps fail to load: q3dm9,
  q3dm11, q3dm12, q3dm14, q3dm15, q3dm18, q3ctf2 and q3ctf3 (see
  [Map list](#map-list)). The margin on maps that play is small (288640
  bytes on q3ctf1).
- **Lower detail to fit 24 MB:**
  - `r_picmip 2` and a 256x256 texture cap. Textures are 16-bit or `CMPR`.
  - Player models use only their lowest LOD.
  - When less than 4 MB of hunk is free, cgame loads one player model
    (sarge) for all bots that have no model loaded.
  - No decals, blood, gibs, brass, smoke trails or impact effects. The
    player blob shadow stays.
  - Items show as sprites. No 3D models in the HUD.
  - No stencil shadows (GX has no stencil).
  - 256 particles, 128 local entities, 1024 sounds, 1 font.
  - No bot reply chats.
- **Cinematics** larger than 512x256 do not play.
- **No music.** Only sound effects play.
- **Sound paging.** The 16-slot decode cache can thrash with many voices.
- **No network.** The Broadband Adapter is not supported. Only local
  play with bots works.
- **No rumble.**
- **No on-screen keyboard.** You cannot type a name or a chat message.
- **One controller.** Only the first connected pad works.
- **No log in release builds.** Use `DEBUG=1` for the engine log.
- **No save from a disc image.** Config saves need an SD card.
- **First boot.** On one boot (Action Replay SD loader, Datel SD adapter in
  slot B, FAT16) the game showed "No quake3 folder found". The next boot
  worked. The cause is not known.
- **Open Arena** is not available yet.

## AI disclosure

This port is developed with the assistance of AI coding tools.

- **Claude Code** (Anthropic) plans the tasks, writes the briefs, reviews
  every change, runs the Docker builds and the Dolphin tests, and writes
  the documents.
- **OpenCode** with a DeepSeek model and **Codex** (OpenAI) write most of
  the code. Their commits use the name `Stroggo`.
- The maintainer makes every design decision, chooses the memory cuts and
  runs all hardware tests.

AI-written code is reviewed before a merge. A release is tested on real
hardware before it ships.

## Credits

- [ioQuake3](https://github.com/ioquake/ioq3): the upstream engine.
- [old-mac-quake3](https://github.com/matthewdeaves/old-mac-quake3)
  (Matthew Deaves): the PowerPC ioquake3 baseline.
- [ioQuake3-wii](https://github.com/Mayo1970/ioQuake3-wii): GX backend,
  pad input and audio reference.
- [Quake3e](https://github.com/ec-/Quake3e): zone allocator.
- [q3ce](https://code.google.com/archive/p/q3ce/): small-memory ideas.
- [devkitPro](https://devkitpro.org/): devkitPPC, libogc, libfat.
- [Dolphin](https://dolphin-emu.org/): emulator for smoke tests.

## License

GPLv2. See [LICENSE](LICENSE) and
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
