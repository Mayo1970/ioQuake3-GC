# CLAUDE.md — ioQuake3 GameCube port

Guidance for Claude Code when working in this repo.

> **STATUS: docs-first scaffold (created 2026-06-06, revised 2026-07-02).
> Nothing compiled yet.** This repo holds the design only. **The plan of
> record is CLASSIC-first** — see `../CLASSIC_GC.md` for the primary analysis
> and budget; this file covers the platform picture and the stretch goal (a
> full singleplayer port). The next session's job is the validation sequence
> in `KICKOFF_PROMPT.md`.

## What this is

A GameCube port of ioQuake3, derived from the working **Wii port** at
`E:\Users\Matteo\Desktop\quake3\ioquake3-wii\ioQuake3-wii`. The Wii port is the
parent: it already has the **native GX renderer**, the GameCube-controller
input path, the libogc audio backend, the CLASSIC (Dreamcast-crossplay)
protocol stack — HW-confirmed working 2026-07-02 — and all the Q3 engine
Wii-fixes. This project re-targets that work at the GameCube's much tighter
hardware.

**Primary target: the CLASSIC build** (protocol-43 multiplayer client vs
Dreamcast servers). It is the most GC-favorable configuration: no qagame on
the client, stripped QVMs, small-map roster. Full analysis: `../CLASSIC_GC.md`.

The companion reference is the **old-Mac ioquake3 port** at
`E:\Users\Matteo\Desktop\quake3\ioQuake3-GC\old-mac-quake3`. It is not GameCube
code, but it is the load-bearing piece of evidence that the **CPU is adequate**
(see below).

## Verdict in one line

**Viable — a memory-reduction project, not a recompile.** The CPU and GPU are
proven adequate; the single hard blocker is RAM (24 MB, no MEM2). For CLASSIC
the budget closes at the low-to-mid range of the estimates (`../CLASSIC_GC.md`);
for the full-port stretch goal the *Dreamcast diet* still applies: a MEM1-only
allocator, ARAM paging for audio, and possibly a rebuilt low-resolution asset
set. See `docs/MEMORY_MAP.md`.

## The CPU is a non-issue — proven by the old-Mac port

The GameCube's **Gekko** is a 485 MHz PowerPC 750CXe (paired-single FPU, 256 KB
L2, **no AltiVec**). That is the *same core family* as the **G3 449 MHz
(PPC750)** machine `yosemite` in the old-Mac port, which runs ioquake3 at:

- **45 fps @ 640×480**, **27 fps @ 1024×768** — stock settings, no tuning, on a
  *weaker-than-Flipper* Rage 128 16 MB.

Gekko is slightly faster than that G3. So "can a PPC750 run the Q3 engine at a
playable rate?" is answered **yes, by direct hardware evidence**. The bytecode
interpreter is fine — and it got cheaper since this doc was first written: the
Wii tree's `VM_PrepareInterpreter` now runs a sizing pre-pass that allocates
exactly the emitted ints (~half the old expanded-code hunk). The PPC JIT is not
needed here: with no qagame (CLASSIC) the compile targets are small, and on GC
the JIT would only *relocate* Arena1 pressure (code buffer instead of hunk)
while adding a multi-MB compile transient. Do **not** spend effort chasing CPU
speed — it is not the bottleneck.

## Hardware: GameCube vs Wii (the parent platform)

| | Wii (parent port) | GameCube (this port) | Discriminator |
|---|---|---|---|
| CPU | Broadway 729 MHz PPC750CL | **Gekko 485 MHz PPC750CXe** | both `GEKKO` |
| GPU | Hollywood 243 MHz | **Flipper 162 MHz** (~⅔ fill) | same GX |
| Fast main RAM | 24 MB MEM1 (1T-SRAM) | **24 MB** (1T-SRAM) | `SYS_GetArena1*` |
| Large work RAM | **64 MB MEM2 (GDDR3)** | **NONE** | `SYS_GetArena2*` = Wii-only |
| Aux RAM | — | 16 MB ARAM (DMA-only, not CPU-addressable) | `AR_*` / `ARQ_*` |
| GPU embedded | 3 MB (2 MB EFB + 1 MB tex) | 3 MB (2 MB EFB + 1 MB tex) | same |
| Network | built-in WiFi/LAN via IOS | **Broadband Adapter** (SP1); libogc `libbba.a` + lwIP | same `net_*` API |
| **Total usable** | **88 MB** | **24 MB** (+16 MB paged) | — |

**Build-time discriminator: `HW_RVL` (Wii) vs `HW_DOL` (GameCube).** libogc
defines exactly one of these per target. `GEKKO` is defined for *both*, so every
existing `#if defined(GEKKO)` guard in the Wii tree will also fire on GameCube —
convenient, but it means **anything Wii-specific (MEM2, WPAD) must be re-guarded
`#if defined(HW_RVL)`**, with a `#if defined(HW_DOL)` arm for the GameCube path.
The job is contained (grep-measured 2026-07-02): WPAD/DRC lives in
`code/input/wii_input.c` (71 refs), MEM2/Arena2 in `code/sys/wii_sys.c` (6
refs), plus the Makefile's `wiiuse`/`bte`/`wiidrc` libs. `code/renderer/` has
**zero** Wii-only references.

## THE blocker: RAM (no MEM2)

The Wii port's working set, straight from its `code/sys/wii_sys.c` and memory
table:

- Hunk **32 MB** + Zone **8 MB** + Sound pool **~6.3 MB** = **~46 MB**, *all of
  it in MEM2*.

The GameCube has **no MEM2**. There is only the 24 MB Arena1 (shared with the
engine binary, framebuffers, GX FIFO, and stack) plus 16 MB of DMA-only ARAM.
You cannot drop a 46 MB working set into it. `Wii_MEM2_Init`, the
`__wrap_calloc`→MEM2 routing, and every `SYS_GetArena2*` call have **no
GameCube equivalent** — this is precisely why it is not a recompile.

The diet target and the ARAM-paging plan live in `docs/MEMORY_MAP.md`; the
CLASSIC-specific budget (much friendlier — no qagame) lives in
`../CLASSIC_GC.md`. The two sharpest pressures for the **full-port** stretch
goal: the **three QVMs' hunk cost** (formerly ~10 MB measured; lower now that
the non-pow2 `dataAlloc` and the interpreter sizing pre-pass are in the Wii
tree — re-measure), and **per-map texture memory**, which on GC competes for
the same Arena1 as everything else.

⚠️ **Textures are a separate heap that the Wii hid in MEM2.** The native GX
backend `memalign`s every resident texture (`code/renderer/tr_gx_texture.c`);
on the Wii that's sbrk→MEM2, on the GameCube it's the *same Arena1 as
hunk/zone* — a **fourth contender** for the heap. The saving grace (new since
this doc's first revision): textures are **16-bit** (`GX_TF_RGB565`/`RGB5A3`)
with in-place box-filter mips, so the footprint is half of what the old
OpenGX-based analysis assumed, and the old 2× mip-upload transient is gone.
See `docs/MEMORY_MAP.md` §3 and `MISTAKES.md` ("more total RAM ≠ more texture
room").

## Precedent: this class of problem is solved (Dreamcast)

Quake III Arena **shipped as a retail product on the Sega Dreamcast** (2000):
16 MB main + 8 MB VRAM + 2 MB sound = **26 MB total — less than the GameCube's
40 MB.** It got there with rebuilt assets (lower-res textures, baked picmip,
trimmed sound, smaller map roster). So "Q3A in ~24–40 MB" is *demonstrated
feasible*, just not by recompiling retail paks. The GameCube has *more* RAM than
the platform Q3 actually shipped on — the engineering, not the silicon, is the
question.

## What carries over from the Wii port (low risk)

Copy from `...\ioquake3-wii\ioQuake3-wii\` and keep mostly as-is:

- **Renderer: the native GX backend** (`code/renderer/tr_gx.c`,
  `tr_gx_texture.c`, `qgl_wii.c`, plus the `WII_NATIVE_GX` arms in
  `code/renderergl1/` and `code/sys/wii_glimp.c`). Direct GX calls,
  HW-confirmed on Wii, grep-verified free of MEM2/`HW_RVL` references, and it
  uses GameCube-native 16-bit texture formats. GX on Flipper is the API's
  home turf (Hollywood added almost nothing) — still **smoke-test a textured
  scene early** (Dolphin GC mode, then hardware), but this is a far smaller
  bet than the OpenGX translation layer this section used to prescribe.
  **`libs/opengx/` is NOT needed on GameCube at all** — don't port it, don't
  link it. ⚠️ Flipper is ~⅔ the fill rate and the Wii frame is already
  render-bound (`bk`) — target **640×480 / 30 fps** (240p for CRTs), lower
  `r_picmip`, expect no headroom.
- **Input:** the GameCube-controller path in `code/input/wii_input.c` (PAD/SI)
  already exists and is the *primary* input here. The WPAD/Wiimote/Classic and
  Wii U DRC code is dead weight on GameCube — gate it out under `HW_RVL`.
- **Audio:** the libogc ASND backend (`code/audio/wii_snd.c` +
  `wii_sys.c` sound shims) works on GameCube (same DSP; `lib/cube/libasnd.a`
  exists). Sounds are ADPCM 4:1 and the pool is `com_soundMegs 2` (~6.3 MB);
  the pool wants to live in **ARAM** if the budget demands it (see
  `../CLASSIC_GC.md` for the honest effort rating).
- **Network:** `code/sys/wii_net.h` uses only cross-platform libogc `net_*`
  calls (`net_init`, `if_config`, sockets, `net_gethostbyname` — verified
  against `network.h`; only `net_init_async`/`net_get_status`/wc24 are
  Wii-gated, and the port doesn't use them). On GC this runs over the
  **Broadband Adapter** via `lib/cube/libbba.a`. For CLASSIC this is not
  optional — it's the whole point — and the decided plan is **Dolphin BBA
  emulation first, real BBA hardware later**.
- **CLASSIC protocol stack:** proto-43 scramble, compat wire format, event
  translation, retail persistant ABI, `CL_CompatUserinfo` — all engine-side,
  platform-neutral, HW-confirmed on Wii vs real DC servers (2026-07-02).
- **Engine Wii-fixes:** `Sys_Milliseconds` overflow fix, the FS / pk3dir
  security fix, the netchan sizing, the cinematic ring — all platform-neutral,
  keep them.

## GameCube-specific work needed (the actual project)

1. **MEM1-only allocator** — a `gc_sys.c` replacing `wii_sys.c`'s MEM2 routing.
   Hunk + zone + resident sound all come out of the single 24 MB Arena1. Set
   `com_hunkMegs`/`com_zoneMegs`/`com_soundMegs` from what Arena1 actually has
   after the binary/FB/FIFO/stack carve-out.
2. **ARAM paging** — `AR_*`/`ARQ_*` to stage the ADPCM sound chunks, DMA'd into
   a small MEM1 staging buffer around mixing. This is the new subsystem the Wii
   never needed; effort is medium-high (`../CLASSIC_GC.md` has the verified
   struct-level analysis). Only build it if the texture measurement says the
   budget doesn't close without it.
3. **VM hunk** — already improved upstream: the non-power-of-2 VM `dataAlloc`
   **is in the Wii tree now** (older revisions of this doc said it wasn't),
   as is the interpreter sizing pre-pass. Inherited for free; re-measure
   per-VM hunk on the current tree before trusting the old ~10 MB figure.
4. **Asset set** — for CLASSIC, retail `pak0–pak2` + `zpack-classic.pk3`
   (186 KB) at runtime `r_picmip 3` is the plan; rebuilt low-res paks are the
   **last resort**, no longer a prerequisite (16-bit textures + working
   runtime picmip changed the math). For the full-port stretch goal, rebuilt
   paks likely remain mandatory.
5. **Build target** — `Makefile.gc` (machine `ogc`, `gamecube_rules`), links
   `bba fat asnd ogc` (no `wiiuse`/`bte`/`wiidrc`, no opengx), libfat over
   SD Gecko / SD2SP2.
6. **Cinematics disabled** — `code/client/cl_cin.c`'s file-scope statics
   (`cin`, `vq2`, `vq4`, `vq8`, `cinTable`) are cinematics-only (confirmed
   2026-07-04, nothing else references them) and total **≈3.0 MB of BSS** —
   measured 2026-07-04 at 10.5–11.0 MB total, ~5–10× the docs' old estimate.
   Gate `CIN_PlayCinematic`/`CIN_RunCinematic`/etc. to a no-op under
   `HW_DOL`, or exclude the file from the GC compile if nothing else pulls
   in its externs. Decided, not optional — see `../CLASSIC_GC.md` Revision 3
   and `docs/MEMORY_MAP.md` for the full BSS breakdown.

## Build (planned)

devkitPPC builds GameCube `.dol`s the same way it builds Wii ones — the
toolchain is shared; only the machine rules and linked libs differ.

- Wii: `machine = rvl` (`$(DEVKITPPC)/wii_rules`), links `wiiuse bte fat asnd ogc`.
- GameCube: `machine = ogc` (`$(DEVKITPPC)/gamecube_rules`), `-DGEKKO -mogc
  -mcpu=750`, links `bba fat asnd ogc` (no `wiiuse`/`bte`/`wiidrc`).

Build from the **devkitPro MSYS2 shell** (not Git Bash), same as the Wii port.
Primary test bench: **Dolphin in GameCube mode** (enforces the exact
24 MB + 16 MB ARAM map and emulates the BBA — it's both the memory and the
network test bench). Real hardware via **Swiss** (SD2SP2 / SD Gecko / GC
Loader); online on real HW needs a **Broadband Adapter** (or modern clone,
e.g. ETH2GC).

Reference binary sizes (measured 2026-07-02, Wii builds): `build/boot.dol`
= 2.34 MB (Q3A), `build_classic/boot.dol` = 2.53 MB (includes Wii-only libs
GC drops + the embedded zpack GC needn't embed). DOL size = text+data only;
**BSS is extra and unmeasured** — `powerpc-eabi-size` the ELF (measurement #1
in `docs/MEMORY_MAP.md`).

## Conventions (inherited)

- **Don't create new docs** unless asked; keep these four (plus
  `../CLASSIC_GC.md`) current.
- Build from devkitPro MSYS2.
- The Wii port is the upstream-of-record for shared engine code. Fixes that
  aren't GameCube-specific should ideally land there too. **These docs went
  stale once already** (native GX, VM optimizations, CLASSIC completion all
  landed upstream unnoticed — see `MISTAKES.md`); re-verify against the Wii
  tree's `CLAUDE.md` at kickoff.
- Honesty: every GameCube memory/fps number in these docs is a **projection
  until measured on hardware/Dolphin**. The Wii numbers are real; the GC ones
  are not yet. Mark estimates as estimates.

## References

- **Primary plan (CLASSIC-first): `../CLASSIC_GC.md`**
- Parent Wii port: `E:\Users\Matteo\Desktop\quake3\ioquake3-wii\ioQuake3-wii\CLAUDE.md`
- CPU-viability evidence (old-Mac PPC750): `E:\Users\Matteo\Desktop\quake3\ioQuake3-GC\old-mac-quake3\`
- Memory plan (full-port stretch goal): `docs/MEMORY_MAP.md`
- Known traps: `MISTAKES.md`
- Kickoff: `KICKOFF_PROMPT.md`
