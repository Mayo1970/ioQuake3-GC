#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

/* libogc COLOR_* clash with the q_shared color escapes. */
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_WHITE

#include "../renderer/tr_local.h"
#include "gc_glimp.h"

#define GX_FIFO_SIZE    (256 * 1024)
#define NUM_FRAMEBUFFERS 2

static GXRModeObj   s_mode;   /* private copy, so libogc's mode tables stay stock */
static GXRModeObj  *s_rmode       = NULL;
static void        *s_framebuf[NUM_FRAMEBUFFERS] = { NULL, NULL };
static void        *s_gp_fifo     = NULL;
static int          s_fb_index    = 0;
static qboolean     s_initialised = qfalse;

qboolean GC_GX_Init(void)
{
    if (s_initialised)
        return qtrue;

    /* Video mode is boot-time only: a vid_restart re-init reuses the XFBs (GC_GX_Shutdown
       keeps them) instead of leaking two new ones. */
    if (!s_framebuf[0]) {
        VIDEO_Init();
        VIDEO_GetPreferredMode(&s_mode);
        s_rmode = &s_mode;

        void *fb0 = SYS_AllocateFramebuffer(s_rmode);
        if (!fb0) {
            printf("[glimp] FATAL: could not allocate framebuffer\n");
            return qfalse;
        }
        s_framebuf[0] = MEM_K0_TO_K1(fb0);

        void *fb1 = SYS_AllocateFramebuffer(s_rmode);
        if (!fb1) {
            printf("[glimp] FATAL: could not allocate framebuffer\n");
            return qfalse;
        }
        s_framebuf[1] = MEM_K0_TO_K1(fb1);
    }

    VIDEO_Configure(s_rmode);
    VIDEO_SetNextFramebuffer(s_framebuf[0]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (s_rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    s_gp_fifo = memalign(32, GX_FIFO_SIZE);
    if (!s_gp_fifo) {
        printf("[glimp] FATAL: could not allocate GX FIFO\n");
        return qfalse;
    }
    memset(s_gp_fifo, 0, GX_FIFO_SIZE);
    GX_Init(s_gp_fifo, GX_FIFO_SIZE);

    GXColor bg = { 0, 0, 0, 255 };
    GX_SetCopyClear(bg, 0x00FFFFFF);

    float yscale = GX_GetYScaleFactor(s_rmode->efbHeight, s_rmode->xfbHeight);
    u32   xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, s_rmode->fbWidth, s_rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, s_rmode->fbWidth, s_rmode->efbHeight);
    GX_SetDispCopyDst(s_rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(s_rmode->aa, s_rmode->sample_pattern, GX_TRUE,
                     s_rmode->vfilter);
    GX_SetFieldMode(s_rmode->field_rendering,
                    ((s_rmode->viHeight == 2 * s_rmode->xfbHeight)
                        ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_CopyDisp(s_framebuf[s_fb_index], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    s_initialised = qtrue;
    return qtrue;
}

void GC_GX_EndFrame(void)
{
    /* No VIDEO_WaitVSync here - hard vsync halves FPS the instant a frame runs long. */
    s_fb_index ^= 1;
    GX_CopyDisp(s_framebuf[s_fb_index], GX_TRUE);
    GX_DrawDone();
    GXBE_FrameEnd();
    VIDEO_SetNextFramebuffer(s_framebuf[s_fb_index]);
    VIDEO_Flush();
}

void GC_GX_Shutdown(void)
{
    if (!s_initialised)
        return;
    GX_AbortFrame();
    GX_Flush();
    VIDEO_SetBlack(TRUE);
    VIDEO_Flush();
    if (s_gp_fifo) { free(s_gp_fifo); s_gp_fifo = NULL; }
    s_initialised = qfalse;
}

GXRModeObj *GC_GX_GetRMode(void)
{
    return s_rmode;
}

int GC_GX_GetEFBHeight(void)
{
    return s_rmode ? (int)s_rmode->efbHeight : 480;
}

int GC_GX_GetXFBHeight(void)
{
    return s_rmode ? (int)s_rmode->xfbHeight : 480;
}

void GLimp_Init( void )
{
    /* GC_GX_Shutdown frees the GX FIFO on every vid_restart; without a re-init the GP
       write-gather pipe points at freed memory and the next draw hangs the console. */
    QGL_Init();

    if (!GC_GX_Init())
        Com_Error(ERR_FATAL, "GLimp_Init: GC_GX_Init failed");

    GXRModeObj *rmode = GC_GX_GetRMode();

    glConfig.vidWidth      = rmode ? (int)rmode->fbWidth   : 640;
    glConfig.vidHeight     = GC_GX_LOGICAL_HEIGHT;   /* tr_gx.c maps it onto the EFB */
    glConfig.windowAspect  = (float)glConfig.vidWidth / (float)glConfig.vidHeight;
    glConfig.colorBits     = 24;
    glConfig.depthBits     = 24;
    glConfig.stencilBits   = 0;
    glConfig.deviceSupportsGamma     = qfalse;
    glConfig.textureCompression      = TC_NONE;
    glConfig.textureEnvAddAvailable  = qfalse;
    glConfig.numTextureUnits         = 2;
    Q_strncpyz(glConfig.renderer_string, "GX/GameCube", sizeof(glConfig.renderer_string));
    Q_strncpyz(glConfig.vendor_string,   "Nintendo",    sizeof(glConfig.vendor_string));
    Q_strncpyz(glConfig.version_string,  "GX 1.0",      sizeof(glConfig.version_string));
    Q_strncpyz(glConfig.extensions_string,
               "GL_ARB_multitexture GL_EXT_compiled_vertex_array",
               sizeof(glConfig.extensions_string));
}

void GLimp_Shutdown(void)
{
    GC_GX_Shutdown();
    QGL_Shutdown();
}

void GLimp_EndFrame(void)
{
    GC_GX_EndFrame();
}

void GLimp_SetGamma(unsigned char red[256],
                    unsigned char green[256],
                    unsigned char blue[256])
{
    (void)red; (void)green; (void)blue;
}

qboolean GLimp_SpawnRenderThread(void (*function)(void))
{
    (void)function;
    return qfalse;
}

void *GLimp_RendererSleep(void)      { return NULL; }
void  GLimp_FrontEndSleep(void)      { }
void  GLimp_WakeRenderer(void *data) { (void)data; }

void GLimp_LogComment(char *comment)
{
    (void)comment;
}

void GLimp_Minimize(void)
{
}
