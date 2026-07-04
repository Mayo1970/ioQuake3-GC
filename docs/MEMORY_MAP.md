# GameCube memory map & the RAM diet

**Everything in this file is a projection until measured on hardware (or
Dolphin, which enforces the exact GC memory map).** The Wii figures it derives
from are real (from the parent port's `CLAUDE.md` / `wii_sys.c` / measured
DOLs); the GameCube figures are estimates. Replace each `est` with a measured
value as soon as a build runs.

> **Scope note (2026-07-02 revision):** this file models the **full-port
> stretch goal** (three VMs, singleplayer + bots, retail-ish assets). The
> **CLASSIC build is the plan of record** and has a friendlier budget (no
> qagame, stripped QVMs, small maps) — see `../../CLASSIC_GC.md` for its
> tables. The carve-out and pool mechanics below apply to both.

## The hardware budget

| Region | Size | CPU access | Typical use |
|---|---|---|---|
| Main RAM (Arena1 / MEM1) | **24 MB** 1T-SRAM | direct, fast (~2.6 GB/s) | everything |
| ARAM | **16 MB** DRAM | **DMA only** (`AR_*`/`ARQ_*`), not mapped | audio samples; manual paging |
| GPU embedded | 3 MB | GPU only | 2 MB EFB + 1 MB texture cache |

There is **no MEM2**. The Wii put hunk+zone+sound (~46 MB) in MEM2; here all of
it must come out of the 24 MB Arena1, which *also* holds the engine binary, the
framebuffers, the GX FIFO, and the stack. (Real usable ceiling ≈ **23.5 MB** —
the IPL/OS reserve and DOL loader shave a bit off the top.)

## MEM1 carve-out (what's left for the heap)

The engine heap (`sbrk`/Arena1) is 24 MB **minus** these fixed costs:

| Consumer | Size | Notes |
|---|---|---|
| Engine `.dol` (code + data) | **2.4–2.5 MB measured** (Wii CLASSIC ELF text+data, 2026-07-04) | GC binary a bit smaller: drops `wiiuse`/`bte`/`wiidrc`; needn't embed zpack |
| **BSS** | **10.5–11.0 MB MEASURED**, **~7.5 MB effective on GC** after the decided cinematics cut | Measured via `powerpc-eabi-size build_classic/boot.elf` = 11,048,112 bytes (2026-07-04) — **was estimated 1–2 MB, measurement #1 blew that up 5–10×.** Largest single item was the cinematics (`cin`) buffer at 2.43 MB; **cinematics are disabled on GC** (see breakdown below), reclaiming ≈3.0 MB total. Remaining BSS includes the main task stack (`s_mainStack`, 512 KB) already — don't double-count the "Stack" line below against it. |
| Framebuffers (2× XFB) | 0.6 MB @ 240p / 1.2 MB @ 480i | use 240p (frees ~0.6 MB, cleaner CRT) — same trick as the Wii `WII_240P` path. Dynamically allocated, not in BSS. |
| GX FIFO | 0.25 MB | dynamically allocated, not in BSS |
| libogc + lwIP/BBA + SD buffers | ~0.3–0.5 MB `est` (was ~1 MB) | lwIP's static pbuf pool (`memb_mem_pool_pbufs`, 201 KB) is **already counted inside the measured BSS above** — this line is now just the dynamic/SD residual |
| **Fixed subtotal** | **~10.5–11.5 MB measured+est** (with the decided cinematics cut; ~13.5–14.5 MB without it) | **still up from the pre-measurement ~5.5–7 MB estimate, even after the cut** |
| **Heap remaining (Arena1, ~23.5 MB ceiling)** | **~12–13 MB `est`** (with the decided cinematics cut below; ~9–10 MB without it) | hunk + zone + resident sound + textures |

### BSS breakdown (measured 2026-07-04, `build_classic/boot.elf`, `nm --size-sort`)

| Symbol(s) | Size | What it is | GC status |
|---|---|---|---|
| `cin` | 2.43 MB | cinematic (RoQ) decode buffer(s) | **Decided: cut.** `cl_cin.c`-only static |
| `vq2` + `vq4` + `vq8` | 0.67 MB | RoQ VQ codebooks (0.03+0.13+0.51 MB), used only by cinematic decode | **Decided: cut** — same file, confirmed cinematics-only |
| `cinTable` | 0.02 MB | cinematic handle table | **Decided: cut** — same file |
| `cls` + `cl` | 2.19 MB | client state structs (`clientStatic_t`/`clientActive_t`) | No — protocol/snapshot-sized, fixed cost of being a Q3 client |
| `buckets` | 0.64 MB | lwIP mem pool | No — needed for BBA networking (CLASSIC is multiplayer-only) |
| `scratch.0` + `s_mainStack` | 1.02 MB | renderer scratch buffer / main task stack | No — not cinematics-specific; risky to cut |
| `s_knownSfx` | 0.40 MB | sound registration table | No — fixed by `MAX_SFX`-style constant |
| `sv` + `svs` + `facets` + `serverBans` + `cl_pinglist` | ~0.9 MB | server-hosting state (dedicated-server-capable code, linked even in a join-only client) | **Proposed, not yet decided** — compile out `com_sv_running`/hosting path if GC never hosts |
| everything else (~1150 smaller symbols) | ~2.5 MB | misc fixed tables (fonts, cvars, sound pool bookkeeping, VM tables, …) | Mostly no — death by a thousand cuts, not a single lever |

**Decided GC trim: cinematics disabled entirely.** All of `cl_cin.c`'s
statics (`cin`+`vq2`+`vq4`+`vq8`+`cinTable`) are cinematics-only, confirmed
by reading the file — nothing else references them. Reclaims **≈3.0 MB**
(gate the `CIN_*` entry points to a no-op under `HW_DOL`, or exclude the
file from the GC build). **Proposed, not yet decided:** compiling out
server-hosting support (~0.9 MB) — same pattern as the existing
`com_sv_running` qagame gate, if GC is confirmed join-only with no hosting.

~12–13 MB of working heap (post-fixed-floor, post-cinematics-cut) is close
to **Dreamcast territory** (Dreamcast had 16 MB main *after* its fixed
costs, dedicated to just the game). It is enough *only* with the diet below.

## The two killers inside that ~17 MB

### 1. VM hunk — the pre-diet figure was ~10 MB

Measured on Wii **before** the memory-optimization work (parent `CLAUDE.md`,
Q3 column):

| VM | Wii hunk (old) | Current tree (est) | CLASSIC build |
|---|---|---|---|
| qagame | 4.29 MB | ~3.0 `est` | **0 — not loaded** |
| cgame | 3.58 MB | ~2.4 `est` | ~2.5 `est` (stripped QVM) |
| ui | 2.35 MB | ~1.6 `est` | ~1.5 `est` (stripped QVM) |
| **Total** | **10.22 MB** | **~7 `est`** | **~4 `est`** |

Two upstream changes shrink these and are **inherited for free** (both verified
in the Wii tree 2026-07-02 — older revisions of this file said the first was
absent):

- **Non-power-of-2 VM `dataAlloc`** (`vm.c`, GEKKO arm): allocation covers real
  data + 64 KB stack instead of the power-of-2 rounding — saves 1–3 MB per VM.
- **Interpreter sizing pre-pass** (`VM_PrepareInterpreter`): the expanded-code
  buffer is allocated at exactly the emitted int count, not worst-case
  `codeLength*4` — roughly half the old expanded-code hunk. The GC build runs
  the interpreter, so this lands squarely here.

**Re-measure per-VM hunk on the current Wii tree** before trusting any column
above — the "old" numbers predate both changes. Release (`NDEBUG`) builds also
drop the 24-byte-per-alloc debug headers, same as the Wii.

### 2. Per-map texture + BSP memory

Retail Q3 maps reference tens of MB of source textures, plus several MB of
BSP/lightmap/collision into the hunk. GX textures are read by Flipper from
main RAM, so **every resident texture occupies Arena1**. There is no separate
VRAM to hide them in (the 1 MB Flipper texture cache is a cache, not storage).

**Mitigation:** `r_picmip` (each step = −75% of texture bytes) + small maps.
For the full-port stretch goal with its wider asset set, rebuilt **low-res
paks** (Dreamcast approach, picmip baked into the texels) may still be needed —
but see §3: under the native GX backend, runtime picmip is a genuine lever, so
measure before building an asset pipeline.

### 3. Textures are a FOURTH pool — and not in the hunk (updated 2026-07-02)

The **native GX backend** (`code/renderer/tr_gx_texture.c` — the Wii tree's
default renderer; OpenGX is no longer the model) allocates every resident
texture via `memalign(32)`. On the Wii that lands in **sbrk → MEM2**, invisibly
separate from the hunk. On the GameCube there is no MEM2, so it pulls from the
**same Arena1 as hunk and zone.** The real contenders for the ~17 MB heap are
**hunk + zone + sound + the texture heap** — four pools, not three.

What changed vs the old OpenGX-based analysis (all favorable):

- **Textures are 16-bit** (`GX_TF_RGB565` / `GX_TF_RGB5A3` — GameCube-native
  formats), uploaded from `Upload32`'s already scaled+picmipped buffer. Half
  the bytes of the RGBA8 pool the old analysis assumed.
- **Mips are generated in-place** (box filter inside `GXBE_Upload32`,
  `GX_LIN_MIP_LIN`); the resident allocation is ≈1.33× the base level. The old
  OpenGX alloc-new → keep-old → memcpy → free-old **2× transient spike is
  gone**; the remaining transient is Upload32's RGBA working buffer, freed
  after conversion.
- **Consequence: runtime `r_picmip` directly cuts the resident allocation.**
  The old claim "runtime picmip doesn't help, only baked paks do" is obsolete.
  Pre-shrunk paks remain a fallback for the full port, not a prerequisite.

Unchanged and still true: `r_picmip` shrinks bytes-per-texture, **not** the
unique-texture *count* — the per-session texture floor is set by how many
distinct textures the map + HUD + skins reference. That count is why the
texture line stays the deciding measurement.

## Target heap split (full-port stretch goal — tune from measurement)

| Pool | where | GC target `est` | Wii (for contrast) |
|---|---|---|---|
| Hunk (`com_hunkMegs`) | Arena1 | **8–10** | 32 (MEM2) |
| Zone (`com_zoneMegs`) | Arena1 | **2–3** | 8 (MEM2) |
| **Texture heap (memalign, 16-bit)** | **Arena1** | **2–3 `est`** | (MEM2, hidden) |
| Sound resident (`com_soundMegs`) | Arena1 | **1** | 2 (~6.3 MB pool, MEM2) |
| **Total demand** | | **~13–17 `est`** | |

Against a ~16.5–18 MB heap that is **tight but plausible** — better than the
pre-revision arithmetic (which had 32-bit textures and a 7.5 MB fixed floor),
but with little slack: peak heap (including the Upload32 transient) must stay
under measured Arena1 free. **If the ELF measures big, or Arena1 free comes in
low, the map roster shrinks first, textures second, hunk last.** Don't trust
these splits until measurements #1–#3 land. The CLASSIC budget (no qagame) is
the version of this table that actually closes comfortably — see
`../../CLASSIC_GC.md`.

## ARAM (16 MB) — the pressure valve (NOT as easy as it looks)

ARAM is not CPU-addressable; you `ARQ_PostRequest` DMA transfers between Arena1
and ARAM. **The obvious "move the sound pool to ARAM" plan fights Q3's
architecture** for the *raw* sample path: Q3's software mixer (`snd_mix.c`
`S_PaintChannels`) does random CPU reads of sample data every frame. The honest
options:

1. **Audio — three sub-options, none free:**
   - (a) **Shrink the resident set** (`com_soundMegs 1`, ADPCM already 4:1,
     CLASSIC's narrow sound roster). Simplest; probably the first move. Not an
     ARAM use at all.
   - (b) **Page ADPCM chunks MEM1↔ARAM** around mixing. The ADPCM path decodes
     one 2 KB chunk at a time, sequentially — the textbook DMA pattern — so
     this is *architecturally* sound, but the chunk payload is stored inline in
     `sndBuffer` and the raw-stereo path reads chunks directly, so it's a
     struct restructure, not a tweak. **Effort: medium-high.** Full verified
     analysis: `../../CLASSIC_GC.md` ("ARAM as a sound tier").
   - (c) **Replace Q3's software mixer with GC hardware AX/DSP voices** that mix
     natively from ARAM (what real GC games do). Biggest win, biggest rewrite —
     a different sound backend. Only if (a)+(b) aren't enough.
2. **Texture/data staging (last resort).** The native GX backend expects
   resident `memalign`'d texels, so ARAM texture staging means a custom
   residency layer with real latency. Don't.

**Rule:** ARAM is a backing store, never live memory. Anything needing random
CPU access lives in Arena1 unless you replace the subsystem that touches it.

## Open measurements to take first (in order)

Dolphin (GC mode) enforces the exact 24 MB + 16 MB ARAM map, so **#1–#4 can
all be taken in the emulator** before real hardware. #0 needs only the
existing Wii build.

0. ✅ **DONE 2026-07-04 — texture heap measured, hunk partially measured.**
   Instrumented `tr_gx_texture.c` with live resident/peak byte counters
   (surfaced via `imagelist`), ran `build_classic` in Dolphin (Wii mode,
   q3dm6, `r_picmip 3` + `cg_forceModel 1`), read `qconsole.log` back out of
   Dolphin's `sd.raw` (the normal SD folder-sync-back didn't fire on close;
   extracted directly with `pyfatfs` instead). **Result: GX resident texture
   heap = 3,175,040 bytes (3.03 MB), peak == resident** (confirms no
   transient upload spike). This is the **pessimistic end** of the
   previously estimated 0.5–2 MB band — see `../../CLASSIC_GC.md` Revision 4
   for what this means for the low-res-pak decision. **Follow-up done same
   day:** rebuilt with `WII_VM_NATIVE=0` (interpreter, GC's actual VM mode)
   on the same q3dm6 run — total hunk in use rose from 18.59 MB (JIT) to
   20.57 MB (interpreter); minus qagame (2.75 MB), **~17.0 MB for
   cgame+ui+world on q3dm6 alone**, more than double the ~6–8 MB estimate.
   See `../../CLASSIC_GC.md` Revision 5 — this is now the dominant budget
   problem, bigger than the texture heap, and points at the map roster
   (BSP/lightmap size, not just textures) as the next lever.
1. ✅ **DONE 2026-07-04 — Linked ELF size** (`powerpc-eabi-size
   build_classic/boot.elf`): text 1,875,736 + data 668,600 + **BSS
   11,048,112** = 13,592,448 bytes. **BSS was estimated at 1–2 MB; the real
   number is 5–10× that** — see the breakdown above. This alone roughly
   doubles the fixed floor and is the biggest correction so far to the whole
   budget.
2. **Actual Arena1 free** at runtime: `SYS_GetArena1Hi() - SYS_GetArena1Lo()`
   after video + FIFO + stack init. This is ground truth for the heap.
3. **Smallest-map hunk high-water mark** — does a q3dm1-class map fit the
   hunk target?
4. **Native-GX-on-Flipper smoke test** — does a textured scene render
   correctly? (Validates the renderer carry-over; replaces the old
   "OpenGX-on-Flipper" test — OpenGX is no longer used.)

Until #0–#4 exist, every number above is a guess. Record them in this file as
they land.
