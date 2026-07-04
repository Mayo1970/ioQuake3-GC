# MISTAKES — ioQuake3 GameCube port

Append-only log of approaches that broke or would have broken. **Read before
lighting up an idea that smells "easy", "it's just a recompile", or "the Wii
code already does this".** Mirrors the old-Mac port's `MISTAKES.md` discipline.

---

## "It's just a recompile of the Wii port" — caught at planning (2026-06-06)

**The smell:** the GameCube and Wii share the CPU family (PPC750), the GPU API
(GX), the toolchain (devkitPPC), and libogc. So a GameCube DOL should be a
build-target away.

**Why it breaks:** the Wii port's working set is **~52 MB** (hunk 32 + zone 8 +
sound ~12.6), and *all of it lives in MEM2*. **The GameCube has no MEM2** — only
24 MB Arena1 (shared with the binary, framebuffers, FIFO, stack) and 16 MB of
DMA-only ARAM. `Wii_MEM2_Init`, the `__wrap_calloc`→MEM2 routing, and every
`SYS_GetArena2*` call have no GameCube equivalent. A recompile would fail the
moment it tries to reserve a MEM2 bump arena that isn't there.

**The fix (the project):** a MEM1-only allocator, ARAM paging for audio, and a
rebuilt low-res asset set. See `docs/MEMORY_MAP.md`. It is a memory-reduction
project, not a recompile.

**Lesson:** shared CPU/GPU/toolchain does **not** mean shared memory
architecture. The Wii's 88 MB hid every sizing assumption; the GameCube's 24 MB
exposes all of them at once.

---

## "Retail pak0.pk3 will load, the Wii uses it" — caught at planning (2026-06-06)

**The smell:** the Wii runs unmodified retail paks. Reuse them on GameCube.

**Why it breaks:** the Wii can afford full-res textures because hunk + texture
memory live in 64 MB of MEM2. On GameCube, GX reads textures from the 24 MB main
RAM (the 1 MB Flipper texture cache is a cache, not storage), and the three QVMs
alone eat ~10 MB of hunk before a map loads. A single retail map's full-res
textures + BSP would blow the ~16 MB heap instantly.

**The fix:** rebuilt low-resolution paks with picmip baked into the texels
(Dreamcast approach), a curated small-map roster. This is mandatory asset
pipeline work, not an engine knob — `r_picmip` at runtime still allocates the
full-res image before downsampling on some paths, and doesn't shrink BSP cost.

**Lesson:** the Dreamcast shipped Q3 in 26 MB total by rebuilding the assets, not
by setting a cvar. Same applies here.

---

## "GEKKO means Wii" — pre-emptive note (not yet hit)

**The trap:** devkitPPC defines `GEKKO` for **both** GameCube and Wii. Every
`#if defined(GEKKO)` guard in the Wii tree (hundreds of them) will fire on the
GameCube build too. Wii-only code (MEM2 sizing, WPAD/Wiimote/Classic/DRC input)
guarded only by `GEKKO` will wrongly compile into the GameCube binary and either
fail to link (no `wiiuse`/`bte`) or call MEM2 functions that don't exist.

**The discriminator:** libogc defines **`HW_RVL`** for Wii and **`HW_DOL`** for
GameCube — exactly one per target. Re-guard anything Wii-specific as
`#if defined(HW_RVL)` and add the GameCube arm under `#if defined(HW_DOL)`.
Leave platform-neutral `GEKKO` guards (endianness, `Sys_Milliseconds`) alone.

**Lesson:** decide the GC-vs-Wii discriminator *before* copying the tree, or
you'll chase phantom MEM2/WPAD link errors through every subsystem.

---

## (Open risk, not yet a mistake) Flipper may not run OpenGX unchanged

> **SUPERSEDED 2026-07-02** — the GC port no longer uses OpenGX at all; see
> the entry at the bottom of this file ("risk SUPERSEDED by native GX").

OpenGX targets the GX API, which exists on both Flipper (GC) and Hollywood (Wii),
and Hollywood added almost nothing to the GX feature set — so OpenGX is
*expected* to work on GameCube. But it has only ever been exercised on Hollywood
in this codebase. Possible divergence: texture-format support, EFB-copy
parameters, the 1 MB (vs Wii) texture cache behaviour. **Smoke-test a single
textured triangle on real Flipper hardware before building anything on top of
it** (`docs/MEMORY_MAP.md`, measurement #4). Don't assume; confirm.

---

## (Open risk) Flipper is ~⅔ the Wii's fill rate, and the Wii is already render-bound

The Wii frame is dominated by `bk` (the OpenGX→GX backend), ~13–16 ms of a ~22 ms
frame — it is render-bound, not CPU-bound. Flipper runs at 162 MHz vs Hollywood's
243 MHz, so the same scene costs *more* here. Do **not** expect 480p/60. Target
**640×480 (or 240p) / 30 fps**, low `r_picmip`, small maps, few bots. If you
chase fps, target geometry/fill (fewer surfaces, lower-poly models), never the
CPU or the VM — same conclusion as the parent port.

---

## "GameCube has more total RAM than the Dreamcast, so textures will fit" — caught at planning (2026-06-06)

**The smell:** the Dreamcast shipped Q3 in 26 MB total; the GameCube has 40 MB
(24 + 16 ARAM). So textures should be *easier* here than on the platform Q3
actually shipped on.

**Why it's misleading:** the Dreamcast's 26 MB was **16 MB main + a DEDICATED
8 MB VRAM + 2 MB sound** — textures lived in their own 8 MB pool, not competing
with the engine heap. On the GameCube, GX reads textures from **main RAM**
(the 1 MB Flipper cache is a cache, not storage), and OpenGX allocates them with
`memalign` out of the **same Arena1 as hunk and zone** (verified —
`opengx/src/texture.c:499/523`; see `docs/MEMORY_MAP.md` §3). After the binary
(~5 MB), the three VMs (~7–10 MB hunk), zone, and sound, the *effective* texture
budget on GameCube can be **smaller than the Dreamcast's dedicated 8 MB**, even
though total RAM is larger. And ARAM can't hold them — OpenGX needs them
resident and CPU/GPU-addressable.

**The fix:** treat the texture heap as a first-class pool in the budget (it now
is), pre-shrink texels in the reduced paks (not runtime `r_picmip`, which has a
transient 2× spike), and keep the map roster small.

**Lesson:** "more total RAM" ≠ "more texture room" when the architecture is
*unified*. Compare the pool that actually holds textures, not the headline total.

---

## (Open risk) ARAM won't take Q3's audio samples as-is — the mixer is software

The tempting "free 10 MB of MEM1 by parking the sound pool in ARAM" plan fights
Q3's design: `snd_mix.c` `S_PaintChannels` is a **software mixer** doing random
CPU reads of sample data every frame, which DMA-only ARAM cannot serve. Using
ARAM for audio means either paging samples MEM1↔ARAM around mixing, or replacing
the mixer with GC hardware AX/DSP voices (a different sound backend). Neither is
a tweak. First move is to **shrink the sample set in the reduced pak** and keep a
small resident MEM1 pool. See `docs/MEMORY_MAP.md` "ARAM" section.

---

## "Use Quake3e as the base, it's the better engine" — caught at planning (2026-06-06)

**The smell:** Quake3e is the actively-maintained, cleaner, faster fork — surely
a better starting point than ioq3 for a new port.

**Why it's the wrong base *for GameCube*** (verified against
`E:\Users\Matteo\Desktop\quake3\Quake3e`):
1. **It raises every memory floor; the GC needs them lowered.** The *client*
   build enforces `MIN_COMHUNKMEGS 64` (hard-clamped via `Cvar_CheckRange`),
   `DEF_COMHUNKMEGS 128`, zone 12–25 MB (`common.c:44-56`). The GameCube has
   **24 MB total** — the enforced hunk floor alone is 2.6× the whole machine.
   The fork's entire posture assumes plenty of RAM.
2. **It deleted the fixed-function renderer — the one thing OpenGX bridges.**
   Quake3e has **no `renderergl1`**. Its renderers are `renderer` (modernized:
   **VBOs** via `tr_vbo.c`, **ARB programs** via `tr_arb.c`), `renderer2`
   (**GLSL**), `renderervk` (**Vulkan**). OpenGX is fixed-function (no VBO/ARB/
   GLSL) and Flipper has no programmable shaders. The working chain is
   `ioq3 renderergl1 ↔ OpenGX ↔ GX`; using Quake3e means re-introducing
   renderergl1 (= back to ioq3) or building VBO+ARB into OpenGX (huge).
3. **Its PPC JIT is ppc64/POWER8** (`vm_powerpc.c` guards on `_CALL_ELF` and
   `_ARCH_PWR8`) — useless on 32-bit Gekko; falls to the interpreter (fine, but
   no benefit). Confirms the fork doesn't target this CPU.

**What it would give that's relevant here: almost nothing.** The one cherry-
pickable security fix (pk3dir) is already in the Wii tree; DoS rate-limiting is
already in ioq3; the interpreter macro-op idea was tested → no FPS/memory gain
(render-bound). None touch the binding GC constraints (24 MB RAM, fixed-function
GPU).

**The fix:** stay on **ioq3 (the Wii tree)** — already the leaner, fixed-
function-renderer base with the renderer running on GX. For GameCube you go
*down* from ioq3 (the Dreamcast diet), not sideways-and-up to Quake3e. Same
conclusion the Wii port reached; the GC's tighter constraints make it stronger.

**Lesson:** "best engine" is platform-relative. Quake3e is best for a modern PC
or a 64-bit box. For a 24 MB fixed-function console, its every improvement is a
liability you'd have to revert.

---

## "Use Q3Lite as the base, it's the low-end fork" — caught at planning (2026-06-06)

**The smell:** Q3Lite is literally named for being lightweight and targets the
Raspberry Pi — surely the right base for a constrained console.

**Why it doesn't help here** (verified against `E:\Users\Matteo\Desktop\quake3\q3lite`):
Q3Lite is the *friendliest* of the three forks — it's an **ioq3 fork** (compatible
lineage, 32-bit PPC) and **keeps `renderergl1` fixed-function** (the OpenGX-
bridgeable renderer; it even adds a fixed-function `renderergles1`). It would NOT
fight you like Quake3e. **But "lite" means embedded GPU/CPU + Linux, not a small
RAM budget** (README verbatim: *"embedded Linux systems... running on OpenGL ES
and SDL2... slim the code down to improve performance on low-end embedded
processors"*). It misses the GameCube on every distinguishing axis:
- Target RAM floor = **Pi Zero / Pi 1, 512 MB** (21× the GC's 24 MB); still
  hard-clamps client hunk to **56 MB** (`common.c:1569`).
- Headline feature is the **OpenGL ES** renderer — wrong API for Flipper; and
  OpenGX is not a GLES implementation. You'd use GX/OpenGX regardless.
- Assumes **embedded Linux + SDL2**; the GC is bare-metal libogc.

So it offers **nothing for the binding constraint (24 MB RAM)** and its new value
(GLES/embedded-Linux) is a platform stack the GC doesn't have.

**The fix:** stay on the **Wii/ioq3 tree** — also ioq3, but already carrying the
GX/OpenGX renderer, libogc audio/video, and GC-controller input you'd reuse
wholesale. Rebasing onto Q3Lite throws all that away to gain an unusable GLES
renderer. *Mine* Q3Lite for low-end feature-strip/config ideas; don't rebase.

**Lesson:** "lite" / "low-end" is axis-specific — check whether a fork is light
on *RAM*, *GPU*, or *CPU*. Q3Lite is light on GPU/CPU with RAM to spare; the GC
needs light on RAM above all.

---

## The GC docs went stale while the parent tree moved — caught at re-verification (2026-07-02)

**The smell:** the scaffold docs were carefully evidence-checked when written
(2026-06-06/24), so they could be trusted at kickoff.

**Why it broke:** the docs are *derived from a moving parent*. In under a
month the Wii tree invalidated four load-bearing claims, all silently:

- **"The renderer carry-over is OpenGX"** — the Wii tree switched its default
  to the **native GX backend** (`WII_NATIVE_GX=1`), which changes the texture
  math (16-bit formats, in-place mips, no 2× transient) and deletes the
  OpenGX-on-Flipper risk entirely.
- **"The non-pow2 VM `dataAlloc` is NOT in the Wii tree"** — it landed
  (plus a new interpreter sizing pre-pass worth ~half the expanded-code hunk).
- **"Sound pool ~12.6 MB"** — the Wii settled on `com_soundMegs 2` (~6.3 MB)
  for all flavors, so the Wii working set is ~46 MB, not 52.
- **"Binary ~5 MB / fixed floor ~7.5 MB"** — the real Wii DOL measures
  2.34–2.53 MB text+data (BSS still unmeasured). The floor estimate was ~2 MB
  too fat.

Every error was in the *pessimistic* direction this time — but the same drift
could just as easily hide a new blocker.

**The fix:** each doc now carries a revision date, and the kickoff sequence
starts by re-verifying these docs against the Wii tree's `CLAUDE.md` (and
re-measuring the per-VM hunk numbers, which predate the optimizations).

**Lesson:** docs derived from a living upstream are snapshots, not truths.
Stamp them, and budget a re-verification pass at the start of any session
that acts on them.

---

## "BSS is ~1-2 MB" — estimate wrong by 5-10x, caught at measurement (2026-07-04)

**The smell:** every doc's fixed-floor table listed BSS as "~1-2 MB `est`"
and treated the DOL file size (2.3-2.5 MB, text+data only) as most of the
binary's footprint, with BSS as an afterthought ("Q3 has some large statics
... `powerpc-eabi-size` the ELF to replace the `est`").

**Why it broke:** ran `powerpc-eabi-size` on the actual Wii `build_classic/
boot.elf` (this was measurement #1 in the validation sequence — step 0 of
`KICKOFF_PROMPT.md`). Real numbers: text 1,875,736 + data 668,600 + **BSS
11,048,112** bytes. BSS is *4.4x the DOL's text+data combined* and roughly
6x the top of the estimated range. `nm --size-sort` breakdown: `cin`
(cinematics RoQ decode buffer) alone is 2.43 MB; `cls`+`cl` (client state
structs) are 2.19 MB; a lwIP mem pool (`buckets`) is 0.64 MB; RoQ VQ
codebooks + renderer scratch + main task stack are 1.54 MB; sound
registration table 0.40 MB; server-hosting state (`sv`/`svs`/`facets`/
`serverBans`, linked in even for a join-only client) is ~0.9 MB.

**Impact:** the fixed floor (binary+BSS+FB+FIFO+stack) roughly doubled, from
an estimated ~5.5-7 MB to a measured ~13.5-14.5 MB. Against the ~23.5 MB
real Arena1 ceiling, the heap left for hunk+zone+sound+textures dropped from
an estimated ~16.5-18 MB to ~9-10 MB. The CLASSIC combined-levers budget
table, which previously appeared to close "at the low-to-mid range" even
with ARAM sound, is now ~2-5 MB over by estimate even with every trim
applied — see `CLASSIC_GC.md` Revision 3.

**The fix / new levers this exposed:** BSS turned out to have its own
"picmip"-equivalent moves, previously absent from the plan entirely: (1)
cinematics (`cin` + its VQ codebooks) can be shrunk or skipped for
~2.4-4 MB; (2) server-hosting state can be compiled out under `HW_DOL` for a
join-only GC client for ~0.9 MB, mirroring the existing `com_sv_running`
qagame gate. Both are now probably load-bearing, not optional, given how
little heap remains.

**Lesson:** a DOL/ELF's file size only tells you code+data. **BSS is
invisible until you run `powerpc-eabi-size` on the linked ELF** — a large
game engine's static buffers (cinematic decode buffers, protocol snapshot
structs, VQ codebooks) can dwarf the code itself, and "the DOL is only
2.5 MB" is not evidence that the binary's RAM footprint is small. Measure
the ELF, not the DOL, before trusting any fixed-floor number on a
RAM-constrained target.

---

## "OpenGX may not run on Flipper" — risk SUPERSEDED by native GX (2026-07-02)

The open-risk entry above ("Flipper may not run OpenGX unchanged") is
obsolete: **the GC port will not use OpenGX at all.** The Wii tree's default
renderer is now the native GX backend (`code/renderer/tr_gx.c`,
`tr_gx_texture.c`, `qgl_wii.c` + `WII_NATIVE_GX` arms in `renderergl1/`) —
direct GX calls at ioq3's choke points, HW-confirmed on Wii, grep-verified to
contain zero `HW_RVL`/MEM2 references, using `GX_TF_RGB565`/`RGB5A3` (both
GameCube-native formats). Don't port or link `libs/opengx/` on GC.

**Residual risk (smaller, still real):** the native backend has only ever run
on Hollywood. Smoke-test a textured scene on Flipper (Dolphin GC mode, then
real hardware) before building on it — same discipline, much smaller bet. Its
perf note also softens the fill-rate entry above a little: the Phase-6 vertex
staging ring reduced backend overhead vs the OpenGX era, but the 30-fps target
for GC stands until measured.
