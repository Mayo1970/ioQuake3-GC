# ioQuake3-GC

A port of [ioQuake3](https://github.com/ioquake/ioq3) to the Nintendo GameCube,
using [devkitPPC](https://devkitpro.org/) and [libogc](https://github.com/devkitPro/libogc),
with a native GX renderer (no OpenGL layer) and native game modules (no QVM).

The engine baseline is [old-mac-quake3](https://github.com/matthewdeaves/old-mac-quake3),
an ioquake3 tree from the last SDL 1.2 commit, already proven on PowerPC 750 hardware.
The GX renderer, the pad input and the audio backend come from
[ioQuake3-wii](https://github.com/Mayo1970/ioQuake3-wii).

Two builds are planned from the same source tree:

| Variant | File | Game dir on SD | Order |
|---|---|---|---|
| ioQuake3   | `ioquake3_gc.dol`    | `baseq3` | First |
| Open Arena | `ioquake3_oa_gc.dol` | `baseoa` | After Quake III Arena is done |

## Status

**Pre-alpha. Nothing boots yet.** The repository holds the project setup:
licences, documents and the build contract. The list below is the target, and
each line moves to "works" only after a test on a real GameCube.

- [ ] Builds a `.dol` in Docker
- [ ] Engine core runs (file system, paks, local server)
- [ ] Native game modules (`qagame`, `cgame`, `ui`) linked into the `.dol`
- [ ] Fits the 24 MB of main memory
- [ ] Native GX renderer
- [ ] GameCube controller input + rumble
- [ ] Sound and music
- [ ] Network through the Broadband Adapter
- [ ] Boots into gameplay on hardware
- [ ] Open Arena build

## Prerequisites (Windows)

### 1. Docker

All builds run in the official devkitPro image. No toolchain is installed on
the host.

```
docker pull devkitpro/devkitppc:20261102
```

The image holds devkitPPC r51 (GCC 16.2), libogc 3.2.1, libfat and
gamecube-tools (`elf2dol`).

### 2. Game data

You need to bring your own `.pk3` files — none are included in this repo.

### 3. (Optional) Dolphin

[Dolphin](https://dolphin-emu.org/) runs a `.dol` directly. It is used for
smoke tests only. A result in Dolphin is never a hardware result.

---

## Building

All ioq3 sources are vendored under `code/` (already patched for GameCube) —
no separate patch step, no external checkout.

> The build system arrives with the first sprint tasks. The commands below
> are the contract those tasks implement.

From the repository root:

```
./tools/docker-make.ps1 clean all       # PowerShell
./tools/docker-make.sh clean all        # Git Bash
```

Other targets:

```
make              # ELF + ioquake3_gc.dol
make dist         # release layout in dist/ioQuake3-GC/
make iso          # bootable GameCube disc image for Dolphin smoke tests only
make clean        # wipe all build dirs, dist and any disc image
```

### Build flags

| Flag | Effect |
|---|---|
| `DEBUG=1` | Adds `-DGC_DEBUG -g`; writes a log file beside the game data. Release builds emit no log file. |
| `OA=1` | Open Arena build (`-DSTANDALONEOA`). Planned. |

### Outputs

| Variant | DOL path |
|---|---|
| ioQuake3   | `build/ioquake3_gc.dol` |
| Open Arena | `build_oa/ioquake3_oa_gc.dol` |

The output is always a `.dol`. A disc image (`make iso`) exists only for
Dolphin smoke tests, is never committed and is deleted after the test.

Always run `make clean` before rebuilding after a flag change.

---

## SD card layout

The `.dol` can sit anywhere on the card. The game data lives under one root,
`quake3/`, the same layout as the Wii port, so one card serves both consoles.

```
<dev>:/ioquake3_gc.dol            (any folder; /ipl.dol for PicoBoot)
<dev>:/quake3/baseq3/pak0.pk3 … pak8.pk3
```

`<dev>` is the SD adapter the game finds first:

| Adapter | Device |
|---|---|
| SD2SP2 (Serial Port 2) | `sd2sp2:` |
| SD Gecko, memory card slot A | `carda:` |
| SD Gecko, memory card slot B | `cardb:` |

Format the card as FAT32.

---

## Installing on GameCube

See **[INSTALLATION.md](INSTALLATION.md)** for the full step-by-step,
including where to get the required game files and exactly where to place
them.

---

## Controls (GameCube controller)

Planned layout. It follows the dual-stick layout of the PS3 port, mapped to
the GameCube pad. Buttons will be rebindable from the in-game menu.

### In-game

| Input | Action |
|---|---|
| Control stick | Move (forward/back + strafe) |
| C-stick | Look (yaw + pitch) |
| **R** | Fire |
| **L** | Zoom |
| **A** | Jump |
| **B** | Crouch |
| **Y** | Previous weapon |
| **X** | Next weapon |
| **Z** | Scoreboard |
| **D-pad** | Arrow keys |
| **Start** | Menu (Escape) |
| **Z + A** | Open chat |
| **Z + Y** | Toggle console |
| **Z + Start** | Quit to the loader |

### Menus

| Input | Action |
|---|---|
| Control stick / C-stick | Move cursor |
| D-pad | Arrow keys |
| **A** | Confirm (Enter) |
| **B** | Back (Escape) |
| **Start** | Escape |

A WaveBird has no rumble motor. Rumble is never required to play.

---

## Memory budget

The GameCube has 24 MB of main memory and 16 MB of audio memory (ARAM, DMA
access only). Stock ioquake3 asks for more than 60 MB. The port closes that
gap with:

- **Native game modules.** `qagame`, `cgame` and `ui` are compiled to PowerPC
  and linked into the `.dol`. No QVM bytecode and no interpreter tables on
  the hunk.
- **16-bit GX textures** (`RGB565` / `RGB5A3`) with a `r_picmip` default
  chosen for the texture budget.
- **Sound data in ARAM**, paged to main memory through DMA.
- **Smaller static tables** (BSS), sized for a console, with ideas taken
  from [q3ce](https://code.google.com/archive/p/q3ce/).

The measured table goes here when the engine runs.

---

## Important notes

- The CPU is big-endian. File and network data is little-endian and is
  swapped explicitly.
- Every buffer that GX, the audio DMA or the SD driver reads is 32-byte
  aligned and flushed (`DCFlushRange`) before use. Dolphin does not emulate
  the data cache, so only real hardware shows a missing flush.
- All `.sh` scripts must have LF line endings.
- No Nintendo SDK material is used: only devkitPPC and libogc.

## Credits

- **[ioQuake3](https://github.com/ioquake/ioq3)** — the upstream engine this port is based on.
- **[old-mac-quake3](https://github.com/matthewdeaves/old-mac-quake3)** (Matthew Deaves) — the PowerPC-proven ioquake3 baseline.
- **[ioQuake3-wii](https://github.com/Mayo1970/ioQuake3-wii)** — native GX renderer, GameCube pad input and audio backend.
- **[q3ce](https://code.google.com/archive/p/q3ce/)** — Quake 3 Arena CE, reference for a small memory footprint.
- **[devkitPro](https://devkitpro.org/)** — devkitPPC, libogc, libfat and gamecube-tools.
- **[Dolphin](https://dolphin-emu.org/)** — emulator used for smoke tests.

---

## AI disclosure

Parts of this port are developed with the assistance of AI coding tools. AI is
used for code generation, debugging, porting guidance, and documentation. All
AI-generated code is reviewed, and tested on hardware before a release.

---

## License

ioQuake3 is GPLv2. This port layer is also GPLv2. See `LICENSE` and
`THIRD_PARTY_NOTICES.md`.
