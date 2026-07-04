# Kickoff prompt — ioQuake3 GameCube port (CLASSIC-first)

Start a new Claude Code session **from this repo**
(`E:\Users\Matteo\Desktop\quake3\ioQuake3-GC\ioQuake3-GC`) and paste the block
below. Read `../CLASSIC_GC.md` (the plan of record), `CLAUDE.md`,
`docs/MEMORY_MAP.md`, and `MISTAKES.md` first.

---

You are picking up a GameCube port of ioQuake3, derived from my working
**Wii port** at `E:\Users\Matteo\Desktop\quake3\ioquake3-wii\ioQuake3-wii`
(the upstream-of-record for all shared engine code). The scaffold here is
**docs-only — nothing has compiled yet.** Read `../CLASSIC_GC.md`, `CLAUDE.md`,
`docs/MEMORY_MAP.md`, and `MISTAKES.md` before doing anything, then
**re-verify their claims against the Wii tree's `CLAUDE.md`** — these docs
went stale once already (see the 2026-07-02 MISTAKES.md entry).

**The decided constraints (do not relitigate):**

- **The target is the CLASSIC build** (Dreamcast-crossplay protocol-43
  multiplayer client — HW-confirmed working on Wii 2026-07-02). No qagame on
  the client, stripped QVMs in `zpack-classic.pk3`, small DC-roster maps. The
  full singleplayer port is a stretch goal, not this project.
- This is a **memory-reduction project, not a recompile.** The Wii port's
  ~46 MB working set lives in MEM2; the GameCube has **no MEM2** — only 24 MB
  Arena1 (real ceiling ~23.5) + 16 MB DMA-only ARAM. See `MISTAKES.md`
  ("it's just a recompile").
- **The renderer is the native GX backend** (`tr_gx.c`/`tr_gx_texture.c`/
  `qgl_wii.c` + `WII_NATIVE_GX` arms). **OpenGX is not used — don't port
  `libs/opengx/`.** 16-bit GC-native texture formats; runtime `r_picmip` is a
  genuine memory lever.
- **CPU is proven adequate** (Gekko = PPC750, same family as the old-Mac G3
  that runs Q3 at 27–45 fps). Don't chase CPU speed. Don't enable the JIT
  (interpreter only — the sizing pre-pass upstream already halved its hunk
  cost; on GC the JIT would only relocate Arena1 pressure).
- **Discriminator is `HW_DOL` (GameCube) vs `HW_RVL` (Wii)**, not `GEKKO`
  (defined on both). Re-guard Wii-only code under `HW_RVL`; it is
  concentrated: `code/input/wii_input.c` (WPAD/DRC, 71 refs),
  `code/sys/wii_sys.c` (MEM2 arena, 6 refs), Makefile libs
  (`wiiuse`/`bte`/`wiidrc`).
- **Networking is required, not optional** (CLASSIC is multiplayer-only) and
  is supported: libogc `lib/cube/libbba.a` + the cross-platform `net_*` API
  that `wii_net.h` already uses. **Dolphin's BBA emulation is the primary
  test bench; real GC + Broadband Adapter comes later.**
- **Retail pak0–2 + zpack-classic.pk3 at `r_picmip 3` is the asset plan.**
  Rebuilt low-res paks are the last resort, not a prerequisite (the 16-bit
  native-GX path changed the math). zpack can ship on SD instead of being
  embedded in the DOL.

**Validation sequence — do these in order, committing as you go. Measure
before you build on top of anything (every GC number in the docs is a
projection until then):**

0. **Take the deciding measurements on the existing Wii build first**
   (`../CLASSIC_GC.md` critical path): instrument `tr_gx_texture.c`'s
   `memalign` total + the hunk high-water mark; run a q3dm6-class map at
   `r_picmip 3` + `cg_forceModel 1`; `powerpc-eabi-size` the ELF for BSS;
   re-measure per-VM hunk (the docs' 10 MB figure predates the non-pow2 +
   pre-pass optimizations). **The texture number decides whether ARAM sound
   paging is needed at all.**
1. **Init the git repo here** and bring in the Wii tree. Copy-in is simplest
   for divergence.
2. **`Makefile.gc`** — machine `ogc` (`gamecube_rules`), links
   `bba fat asnd ogc` (no `wiiuse`/`bte`/`wiidrc`, no opengx), interpreter
   only, native GX only. Get it to *link* first and record the size
   (`docs/MEMORY_MAP.md` measurement #1).
3. **`code/sys/gc_sys.c`** — fork of `wii_sys.c` with the MEM2 routing
   removed: MEM1-only allocator out of Arena1, no `SYS_GetArena2*`, no
   `__wrap_calloc` MEM2 path. Set `com_hunkMegs`/`com_zoneMegs`/
   `com_soundMegs` from measured Arena1 free (measurement #2).
4. **Re-guard the Wii-only code** under `HW_RVL` (the three concentrations
   above); the GameCube-pad path in `code/input/wii_input.c` is already the
   primary and should remain.
5. **Boot to menu in Dolphin (GC mode)** with retail pak0–2 +
   `zpack-classic.pk3` — this is also the native-GX-on-Flipper smoke test
   (measurement #4). If GX behavior diverges from Hollywood, fix and log it
   in `MISTAKES.md`.
6. **Load the smallest DC-roster map** and read the hunk high-water mark +
   texture heap (measurement #3). **Make-or-break:** does it fit the
   `../CLASSIC_GC.md` budget?
7. **Connect to a Dreamcast server via Dolphin's BBA** (`net_init`/
   `if_config` over `libbba`). The protocol side is done and HW-confirmed on
   Wii; this validates the GC platform side.
8. **ARAM sound paging** (`AR_*`/`ARQ_*`, ADPCM chunk staging) — **only if
   step 6's numbers demand it.** Effort is medium-high (struct restructure;
   see `../CLASSIC_GC.md` "ARAM as a sound tier"). Try the cheap trims first:
   zone 8→5–6, `com_soundMegs 1`.
9. **Real hardware last:** Swiss + SD Gecko/SD2SP2/GC Loader; online needs a
   physical Broadband Adapter (or clone, e.g. ETH2GC).

**Honesty checks:** every GameCube memory/fps figure in the docs is a
**projection** — replace with measured values as soon as a build runs
(Dolphin counts: it enforces the exact 24 MB + 16 MB ARAM map), and log what
surprised you in `MISTAKES.md`. The realistic target is **640×480 (or 240p) /
~30 fps on DC-roster maps, joining Dreamcast servers**. Ask me before any
plan-level pivot (dropping the CLASSIC-first scope, abandoning the native-GX
carry-over, switching the baseline away from the Wii port). Decide-and-ship
on implementation details.

---

## Quick status snapshot (revised 2026-07-02)

- **Plan of record:** CLASSIC-first — `../CLASSIC_GC.md` (Revision 2).
- **Source-of-record:** the Wii port at `...\ioquake3-wii\ioQuake3-wii`
  (native GX renderer, GC input, audio backend, CLASSIC protocol stack
  HW-confirmed vs real DC servers, engine fixes).
- **CPU evidence:** old-Mac PPC750 port at `...\ioQuake3-GC\old-mac-quake3`.
- **Verified this revision:** GC networking possible (`libbba.a` +
  cross-platform `net_*`); renderer free of Wii-only refs; Wii DOL measured
  2.34–2.53 MB; non-pow2 VM alloc + interpreter pre-pass inherited.
- **Done:** `../CLASSIC_GC.md`, `CLAUDE.md`, `docs/MEMORY_MAP.md`,
  `MISTAKES.md`, this prompt.
- **Not started:** step 0 measurements, git init, tree import, `Makefile.gc`,
  `gc_sys.c`, any compile, Dolphin smoke test.
