# Renderer

## 1. Overview

The renderer is our 2013 ioquake3 `code/renderer` (baseline `2f187c0`).
The native GX backend comes from ioQuake3-wii `9b9ad24`.
There is no OpenGX and no OpenGL library: the game talks to Flipper through GX.

| File | Role |
|---|---|
| `code/renderer/qgl_gc.h` | GL 1.1 types, enums and `extern` `qgl*` function pointers for the native path. |
| `code/renderer/qgl_gc.c` | Defines the pointers; `QGL_Init` assigns no-ops and the `ngx_route_*` wrappers that forward to `GXBE_*`. |
| `code/renderer/tr_gx.h` | Backend state (`gx_state_t`), texture table and the `GXBE_*` API. |
| `code/renderer/tr_gx.c` | State, matrices, viewport/scissor mapping, TEV pipeline, clear quad, immediate mode, portal clip, vertex staging ring, `GXBE_DrawTess`, depth peek. |
| `code/renderer/tr_gx_texture.c` | Texture table, swizzled upload, mip chain, CMPR encoder (off), sub-image upload, EFB readback. |
| `code/sys/gc_glimp.c` | Video mode, GX FIFO, XFBs, `GLimp_*` and `GC_GX_*`. |
| `code/sys/gc_glimp.h` | `GC_GX_LOGICAL_HEIGHT` (480) and the `GC_GX_*` declarations. |

The `tr_*.c` files keep the upstream renderer logic. GX calls sit where the Wii renderer has them, under `#ifdef HW_DOL`.

## 2. Differences between the two renderer trees

| Area | Wii tree (renderergl1) | This tree | Resolution |
|---|---|---|---|
| Directory layout | `renderercommon` + `renderergl1` | one `code/renderer` | All files stay in `code/renderer`. |
| `qgl*` calls | Function pointers filled by `qgl_wii.c` | Upstream `qgl.h` maps `qgl*` to GL symbols with macros | On `HW_DOL`, `qgl.h` includes `qgl_gc.h`; `qgl_gc.c` fills the pointers. Upstream macros remain for other builds. |
| GL headers | OpenGX headers | `qgl_gc.h` | No OpenGX. Everything the renderer uses comes from `qgl_gc.h`. |
| Image creation flags | `IMGFLAG_MIPMAP`, `IMGFLAG_PICMIP`, `IMGFLAG_CLAMPTOEDGE`, `IMGFLAG_NO_COMPRESSION` | `qboolean mipmap`, `qboolean allowPicmip`, `int glWrapClampMode` | `GXBE_SetUploadImage` takes `( name, allowPicmip )`; the other hints come from the `R_CreateImage` arguments. |
| Glimp signature | `GLimp_Init( qboolean )` | `GLimp_Init( void )` | `InitOpenGL` calls it with no argument. |
| Pre-boot renderer stub | `wii_renderer.c` supplies a stub `GetRefAPI` before the real renderer loads | Our engine calls `GetRefAPI` once from `CL_InitRef` | Not ported; there is no pre-boot stub. |
| GL stubs | `wii_gl_stubs.c` for the pre-boot path | Not needed | Not ported. |
| Platform header | Force-included `wii_platform.h` with `COLOR_*` undefs | `tr_local.h` includes `<gccore.h>` and `#undef`s `COLOR_*` under `HW_DOL` | Same effect, no force-include. |
| `HW_RVL` / MEM2 | Used by the Wii build | Not used | GameCube is `HW_DOL` only; no MEM2. |
| Video modes | 240p/264p, 704 VI width, console XFB reuse | Native mode only; two XFBs, `GC_GX_LOGICAL_HEIGHT` 480 | Dropped. `GC_GX_Init` allocates two XFBs once. |
| GX profiler | Present | Not ported | Dropped. |
| Cvars | `wii_xoffset`, `wii_yoffset`, `wii_xstretch`, `wii_ystretch` | `gc_xoffset`, `gc_yoffset`, `gc_xstretch`, `gc_ystretch` | Renamed, same defaults and flags. |
| Diagnostics | `wii_diag( ... )` | `ri.Printf( PRINT_DEVELOPER, ... )` | Renamed; `WII_DEBUG` blocks are not ported. |
| CMPR | Encoder present | Encoder present, `gx_want_cmpr` returns false | Off until QGC-17. |
| `GEKKO` arms | Cull override in `tr_mesh.c`, LOD skip in `tr_model.c` | Not part of the GX backend | Not ported. |

### Part 1 adaptations

| Change | Why |
|---|---|
| Whole-file `WII_NATIVE_GX` wrapper removed from `tr_gx.c`, `tr_gx_texture.c`, `qgl_gc.c` | These files build only for the GameCube. |
| `wii_diag` becomes `ri.Printf( PRINT_DEVELOPER, ... )` | No Wii diagnostic channel. |
| `wii_glimp.h` becomes `../sys/gc_glimp.h`; `Wii_GX_GetEFBHeight` becomes `GC_GX_GetEFBHeight` | Shared glimp source. |
| `wii_*` cvars become `gc_*` | GameCube naming. |
| `GXBE_SetUploadImage( name, flags )` becomes `( name, qboolean picmip )`; `IMGFLAG_PICMIP` tests become `picmip`; `IMGFLAG_NO_COMPRESSION` test dropped | Our `image_t` has no `IMGFLAG_*`. |
| `gx_is_text_art` and the `STANDALONETA` block removed | Team Arena only. |
| `WII_GX_*` constants become `GC_GX_*` | Renamed in `gc_glimp.h`. |
| No type changes | `glIndex_t`, `color4ub_t`, `vec2_t` and the `tess` fields exist in the 2013 tree. |

### Part 2 adaptations

| Change | Why |
|---|---|
| `GXBE_Upload32`/`GXBE_TexSubImage2D` guarded in both `RE_UploadCinematic` and the inline upload in `RE_StretchRaw` | The Wii tree delegates `RE_StretchRaw` to `RE_UploadCinematic`; our 2013 tree still has the upload inline there. |
| `GL_CLAMP_TO_EDGE` passed to those uploads instead of `GL_CLAMP` | `qgl_gc.h` has no `GL_CLAMP`; `GXBE_Upload32` only distinguishes `GL_REPEAT`. |
| `s_flipMatrix` and `R_BlendOverTexture` with `mipBlendColors` guarded with `#ifndef HW_DOL` | Their only users are the GL portal path and the CPU mip loop, both compiled out under GX; avoids new unused-symbol warnings. |
| `RB_SetGL2D` keeps `qglDisable( GL_CULL_FACE )` | The brief's arm covers the viewport/matrix block only; the line is a routed no-op in the native path. |
| `InitOpenGL` keeps `R_InitCommandBuffers` and guards `renderer_buffer` | Our older function has both; only the GL constant query is replaced. |
| `RB_DrawSun` keeps its `void` signature | Our tree's sun path is behind `#if 0` in `RB_RenderDrawSurfList`; only the modelview arm was ported. |
| `RB_ReadPixels` already exists at one place | Screenshot, JPEG and levelshot all go through it; the `packAlign = 4` and `GXBE_ReadPixelsRGB` arm is there. |

## 3. Cache and alignment (AC3)

Every buffer the GP reads or writes is 32-byte aligned. `DCFlushRange` runs after the CPU writes data the GP will read. The readback destination is CPU-only.

| Buffer | File | Alignment | `DCFlushRange` | `GX_InvalidateTexAll` |
|---|---|---|---|---|
| GX FIFO (256 KB) | `sys/gc_glimp.c` | `memalign( 32, GX_FIFO_SIZE )` | not needed; handed to `GX_Init` | no |
| Console XFB + XFB 1 (2) | `sys/sys_gc.c`, `sys/gc_glimp.c` | XFB 0 is the console XFB from `GC_InitConsole`; XFB 1 from `SYS_AllocateFramebuffer`, libogc 32-byte aligned | not needed; written by `GX_CopyDisp` | no |
| Texture buffer | `renderer/tr_gx_texture.c` | `memalign( 32, size )` | after the CPU encodes the mip chain | after every upload |
| Sub-image upload | `renderer/tr_gx_texture.c` | same texture buffer | after re-encoding | after the re-upload |
| Vertex staging ring (2 x 128 KB) | `renderer/tr_gx.c` | `memalign( 32, 2 * GXBE_RING_HALF )` | staged block after copy; client arrays in the fallback path | no; `GX_InvVtxCache` after the address reuse |
| `tess` | `renderer/tr_shade.c` | `__attribute__((aligned(32)))` | copied into the ring, flushed there; fallback flushes the client arrays | no |
| Cinematic / RoQ frames | `renderer/tr_backend.c` | caller data; encoded into the texture buffer | inside `GXBE_Upload32` / `GXBE_TexSubImage2D` | inside both |
| EFB readback buffer | `renderer/tr_gx_texture.c` | `memalign( 32, size )` | `DCInvalidateRange` before the GP copy, not a flush | no |
| Readback destination | `renderer/tr_init.c` | temp memory, `packAlign = 4` | CPU reads after `GX_DrawDone` | no |

`GX_InvalidateTexAll` is called only in `tr_gx_texture.c`, once at the end of `GXBE_Upload32` and once in `GXBE_TexSubImage2D`.
Texture data is never cached between uploads; the GP sees the flushed buffer through the loaded `GXTexObj`.

## 4. Video modes and frame loop (QGC-16)

One video mode serves the whole program. `GC_InitConsole` calls `VIDEO_GetPreferredMode( NULL )` once and copies the result into `gc_mode`; `GC_GetVideoMode` returns that copy. If the call returns `NULL` (an unknown TV mode such as `VI_DEBUG`), the fallback is `TVNtsc480IntDf`. Every video size in the GameCube code comes from the `GXRModeObj` fields. The only fixed number is `GC_GX_LOGICAL_HEIGHT` (480): the engine's logical 2D height, which `tr_gx.c` maps onto `efbHeight`.

The GX renderer reuses the console XFB as XFB 0 (`GC_GetConsoleXFB`) and allocates only one more. After the renderer starts, console text is hidden: `Sys_Print` stops writing to the XFB, because XFB 0 now holds a GX frame. `Sys_Error` aborts the GX frame, restores the console XFB and shows the message on screen even after the renderer has started.

| Mode | libogc table | fbWidth | efbHeight | xfbHeight | viHeight | xfbMode | Y scale |
|---|---|---:|---:|---:|---:|---|---:|
| NTSC 480i | `TVNtsc480IntDf` | 640 | 480 | 480 | 480 | `VI_XFBMODE_DF` | 1.00 |
| PAL 576i | `TVPal576IntDfScale` | 640 | 480 | 576 | 576 | `VI_XFBMODE_DF` | 1.20 |
| PAL60 (EuRGB60 480i) | `TVEurgb60Hz480IntDf` | 640 | 480 | 480 | 480 | `VI_XFBMODE_DF` | 1.00 |
| 480p NTSC | `TVNtsc480Prog` | 640 | 480 | 480 | 480 | `VI_XFBMODE_SF` | 1.00 |
| 480p PAL component | `TVPal576ProgScale` | 640 | 480 | 576 | 576 | `VI_XFBMODE_SF` | 1.20 |

`GC_GX_EndFrame` runs in this order:

```
GX_DrawDone
GXBE_FrameEnd
swap XFB
GX_CopyDisp
GX_Flush
VIDEO_SetNextFramebuffer
VIDEO_Flush
VIDEO_WaitVSync only when r_swapInterval is not 0
```

`GX_Flush` pushes the copy out of the write-gather pipe before the VI can show the XFB. The default `r_swapInterval` is 0, so no vsync wait runs and a long frame does not drop to half rate.

Dolphin smoke (`DEBUG=1 GC_MEM1_MB=64`, `com_hunkMegs 24`, q3dm1, 300 s, no error):

| Run | `GC video:` line |
|---|---|
| NTSC disc | `tvmode 0 fb 640x480 efb 480 xfb 480 vi 640x480`, xfbmode DF |
| PAL disc (bi2 region 2) | `tvmode 4 fb 640x480 efb 480 xfb 576 vi 640x576`, xfbmode DF |
| NTSC disc, `SYSCONF.IPL.PGS=True` | `tvmode 2 fb 640x480 efb 480 xfb 480 vi 640x480`, xfbmode SF |

PAL60 is checked by code review: Dolphin's GameCube boot does not set EuRGB60.
Frame dumps (Dolphin OGL under Xvfb) show the console over the main menu (NTSC) and q3dm1 gameplay (PAL 576i).
`r_swapInterval 1` with `vid_restart` runs q3dm1 with no error.
