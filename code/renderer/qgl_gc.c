/* Wire ioQ3's qgl* pointers to the native GX backend. */

#include "tr_local.h"

void (APIENTRYP qglActiveTextureARB) (GLenum texture) = NULL;
void (APIENTRYP qglClientActiveTextureARB) (GLenum texture) = NULL;
void (APIENTRYP qglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t) = NULL;

void (APIENTRYP qglLockArraysEXT) (GLint first, GLsizei count) = NULL;
void (APIENTRYP qglUnlockArraysEXT) (void) = NULL;

void (APIENTRYP qglAlphaFunc) (GLenum func, GLclampf ref) = NULL;
void (APIENTRYP qglArrayElement) (GLint i) = NULL;
void (APIENTRYP qglBegin) (GLenum mode) = NULL;
void (APIENTRYP qglBindTexture) (GLenum target, GLuint texture) = NULL;
void (APIENTRYP qglBlendFunc) (GLenum sfactor, GLenum dfactor) = NULL;
void (APIENTRYP qglCallList) (GLuint list) = NULL;
void (APIENTRYP qglClear) (GLbitfield mask) = NULL;
void (APIENTRYP qglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = NULL;
void (APIENTRYP qglClearDepth) (GLclampd depth) = NULL;
void (APIENTRYP qglClearStencil) (GLint s) = NULL;
void (APIENTRYP qglClipPlane) (GLenum plane, const GLdouble *equation) = NULL;
void (APIENTRYP qglColor3f) (GLfloat red, GLfloat green, GLfloat blue) = NULL;
void (APIENTRYP qglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void (APIENTRYP qglColor4ubv) (const GLubyte *v) = NULL;
void (APIENTRYP qglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) = NULL;
void (APIENTRYP qglColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) = NULL;
void (APIENTRYP qglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void (APIENTRYP qglCullFace) (GLenum mode) = NULL;
void (APIENTRYP qglDeleteTextures) (GLsizei n, const GLuint *textures) = NULL;
void (APIENTRYP qglDepthFunc) (GLenum func) = NULL;
void (APIENTRYP qglDepthMask) (GLboolean flag) = NULL;
void (APIENTRYP qglDepthRange) (GLclampd zNear, GLclampd zFar) = NULL;
void (APIENTRYP qglDisable) (GLenum cap) = NULL;
void (APIENTRYP qglDisableClientState) (GLenum array) = NULL;
void (APIENTRYP qglDrawArrays) (GLenum mode, GLint first, GLsizei count) = NULL;
void (APIENTRYP qglDrawBuffer) (GLenum mode) = NULL;
void (APIENTRYP qglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) = NULL;
void (APIENTRYP qglEnable) (GLenum cap) = NULL;
void (APIENTRYP qglEnableClientState) (GLenum array) = NULL;
void (APIENTRYP qglEnd) (void) = NULL;
void (APIENTRYP qglFinish) (void) = NULL;
void (APIENTRYP qglFlush) (void) = NULL;
void (APIENTRYP qglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) = NULL;
void (APIENTRYP qglGenTextures) (GLsizei n, GLuint *textures) = NULL;
void (APIENTRYP qglGetBooleanv) (GLenum pname, GLboolean *params) = NULL;
GLenum (APIENTRYP qglGetError) (void) = NULL;
void (APIENTRYP qglGetIntegerv) (GLenum pname, GLint *params) = NULL;
const GLubyte *(APIENTRYP qglGetString) (GLenum name) = NULL;
void (APIENTRYP qglLineWidth) (GLfloat width) = NULL;
void (APIENTRYP qglLoadIdentity) (void) = NULL;
void (APIENTRYP qglLoadMatrixf) (const GLfloat *m) = NULL;
void (APIENTRYP qglMatrixMode) (GLenum mode) = NULL;
void (APIENTRYP qglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) = NULL;
void (APIENTRYP qglPolygonMode) (GLenum face, GLenum mode) = NULL;
void (APIENTRYP qglPolygonOffset) (GLfloat factor, GLfloat units) = NULL;
void (APIENTRYP qglPopMatrix) (void) = NULL;
void (APIENTRYP qglPushMatrix) (void) = NULL;
void (APIENTRYP qglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) = NULL;
void (APIENTRYP qglScissor) (GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void (APIENTRYP qglShadeModel) (GLenum mode) = NULL;
void (APIENTRYP qglStencilFunc) (GLenum func, GLint ref, GLuint mask) = NULL;
void (APIENTRYP qglStencilMask) (GLuint mask) = NULL;
void (APIENTRYP qglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass) = NULL;
void (APIENTRYP qglTexCoord2f) (GLfloat s, GLfloat t) = NULL;
void (APIENTRYP qglTexCoord2fv) (const GLfloat *v) = NULL;
void (APIENTRYP qglTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) = NULL;
void (APIENTRYP qglTexEnvf) (GLenum target, GLenum pname, GLfloat param) = NULL;
void (APIENTRYP qglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) = NULL;
void (APIENTRYP qglTexParameterf) (GLenum target, GLenum pname, GLfloat param) = NULL;
void (APIENTRYP qglTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params) = NULL;
void (APIENTRYP qglTexParameteri) (GLenum target, GLenum pname, GLint param) = NULL;
void (APIENTRYP qglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) = NULL;
void (APIENTRYP qglTranslatef) (GLfloat x, GLfloat y, GLfloat z) = NULL;
void (APIENTRYP qglVertex2f) (GLfloat x, GLfloat y) = NULL;
void (APIENTRYP qglVertex3f) (GLfloat x, GLfloat y, GLfloat z) = NULL;
void (APIENTRYP qglVertex3fv) (const GLfloat *v) = NULL;
void (APIENTRYP qglVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) = NULL;
void (APIENTRYP qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height) = NULL;

static void noop_DrawBuffer  (GLenum m)             { (void)m; }
static void noop_PolygonMode (GLenum f, GLenum m)   { (void)f; (void)m; }
static void noop_LockArrays  (GLint f, GLsizei c)   { (void)f; (void)c; }
static void noop_UnlockArrays(void)                 { }

static void noop_void(void) {}
static void noop_u(GLenum a) { (void)a; }
static void noop_uu(GLenum a, GLenum b) { (void)a; (void)b; }
static void noop_i(GLint a) { (void)a; }
static void noop_f(GLfloat a) { (void)a; }
static void noop_d(GLclampd a) { (void)a; }
static void noop_b(GLboolean a) { (void)a; }
static void noop_bbbb(GLboolean a, GLboolean b, GLboolean c, GLboolean d)
    { (void)a; (void)b; (void)c; (void)d; }
static void noop_nu(GLsizei n, const GLuint *p) { (void)n; (void)p; }
static void noop_ni(GLsizei n, GLuint *p) { (void)n; (void)p; }
static GLenum noop_ret_u(void) { return GL_NO_ERROR; }
static const GLubyte *noop_ret_str(GLenum a) { (void)a; return (const GLubyte *)""; }
static void noop_getintegerv(GLenum pname, GLint *params)
    { (void)pname; if (params) *params = 0; }
static void noop_getbooleanv(GLenum pname, GLboolean *params)
    { (void)pname; if (params) *params = 0; }
static void noop_teximage2d(GLenum tgt, GLint lvl, GLint ifmt,
    GLsizei w, GLsizei h, GLint b, GLenum fmt, GLenum tp, const GLvoid *d)
    { (void)tgt; (void)lvl; (void)ifmt; (void)w; (void)h; (void)b; (void)fmt; (void)tp; (void)d; }
static void noop_texsubimage2d(GLenum tgt, GLint lvl,
    GLint xo, GLint yo, GLsizei w, GLsizei h, GLenum fmt, GLenum tp, const GLvoid *d)
    { (void)tgt; (void)lvl; (void)xo; (void)yo; (void)w; (void)h; (void)fmt; (void)tp; (void)d; }
static void noop_readpixels(GLint x, GLint y, GLsizei w, GLsizei h,
    GLenum fmt, GLenum tp, GLvoid *d)
    { (void)x; (void)y; (void)w; (void)h; (void)fmt; (void)tp; (void)d; }
static void noop_scissor(GLint x, GLint y, GLsizei w, GLsizei h)
    { (void)x; (void)y; (void)w; (void)h; }
static void noop_viewport(GLint x, GLint y, GLsizei w, GLsizei h)
    { (void)x; (void)y; (void)w; (void)h; }
static void noop_stencil3(GLenum a, GLint b, GLuint c) { (void)a; (void)b; (void)c; }
static void noop_stencil2(GLenum a, GLenum b, GLenum c) { (void)a; (void)b; (void)c; }
static void noop_stencil_mask(GLuint a) { (void)a; }
static void noop_texparamf(GLenum a, GLenum b, GLfloat c) { (void)a; (void)b; (void)c; }
static void noop_texparamfv(GLenum a, GLenum b, const GLfloat *c) { (void)a; (void)b; (void)c; }
static void noop_texparami(GLenum a, GLenum b, GLint c) { (void)a; (void)b; (void)c; }
static void noop_alpharef(GLenum a, GLclampf b) { (void)a; (void)b; }
static void noop_color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
    { (void)r; (void)g; (void)b; (void)a; }
static void noop_color3f(GLfloat r, GLfloat g, GLfloat b) { (void)r; (void)g; (void)b; }
static void noop_color4ubv(const GLubyte *v) { (void)v; }
static void noop_loadmatrix(const GLfloat *m) { (void)m; }
static void noop_translatef(GLfloat x, GLfloat y, GLfloat z) { (void)x; (void)y; (void)z; }
static void noop_frustum(GLdouble l, GLdouble r, GLdouble b, GLdouble t,
    GLdouble n, GLdouble f)
    { (void)l; (void)r; (void)b; (void)t; (void)n; (void)f; }
static void noop_ortho(GLdouble l, GLdouble r, GLdouble b, GLdouble t,
    GLdouble n, GLdouble f)
    { (void)l; (void)r; (void)b; (void)t; (void)n; (void)f; }
static void noop_clipplane(GLenum p, const GLdouble *eq) { (void)p; (void)eq; }
static void noop_texcoord2f(GLfloat s, GLfloat t) { (void)s; (void)t; }
static void noop_texcoord2fv(const GLfloat *v) { (void)v; }
static void noop_vertex2f(GLfloat x, GLfloat y) { (void)x; (void)y; }
static void noop_vertex3f(GLfloat x, GLfloat y, GLfloat z) { (void)x; (void)y; (void)z; }
static void noop_vertex3fv(const GLfloat *v) { (void)v; }
static void noop_arrayelement(GLint i) { (void)i; }
static void noop_calllist(GLuint l) { (void)l; }
static void noop_begin(GLenum m) { (void)m; }
static void noop_drawelements(GLenum m, GLsizei c, GLenum t, const GLvoid *i)
    { (void)m; (void)c; (void)t; (void)i; }
static void noop_drawarrays(GLenum m, GLint f, GLsizei c) { (void)m; (void)f; (void)c; }
static void noop_texenvf(GLenum t, GLenum p, GLfloat v) { (void)t; (void)p; (void)v; }
static void noop_shademodel(GLenum m) { (void)m; }
static void noop_multitex(GLenum t) { (void)t; }
static void noop_multitexcoord(GLenum t, GLfloat s, GLfloat v)
    { (void)t; (void)s; (void)v; }
static void noop_bind(GLenum t, GLuint n) { (void)t; (void)n; }
static void noop_clear(GLbitfield m) { (void)m; }

/* Routed wrappers: forward to GXBE so every call site works without per-site #ifdefs. */
static void ngx_route_depthrange(GLclampd n, GLclampd f)
{
    GXBE_DepthRange((float)n, (float)f);
}
/* GL stride 0 means tightly packed: size * sizeof(type) */
static int ngx_eff_stride(GLint size, GLenum type, GLsizei stride)
{
    if (stride != 0)
        return (int)stride;
    return (int)size * ((type == GL_UNSIGNED_BYTE) ? 1 : 4);
}
static void ngx_route_vertexptr(GLint size, GLenum type, GLsizei stride, const GLvoid *p)
{
    GXBE_SetVertexPtr(p, ngx_eff_stride(size, type, stride));
}
static void ngx_route_colorptr(GLint size, GLenum type, GLsizei stride, const GLvoid *p)
{
    GXBE_SetColorPtr(p, ngx_eff_stride(size, type, stride));
}
static void ngx_route_texcoordptr(GLint size, GLenum type, GLsizei stride, const GLvoid *p)
{
    GXBE_SetTexCoordPtr(p, ngx_eff_stride(size, type, stride));
}
static void ngx_route_enable(GLenum cap)
{
    if (cap == GL_TEXTURE_2D)
        GXBE_SetTexture2DEnabled(1);
    else if (cap == GL_POLYGON_OFFSET_FILL)
        GXBE_SetPolygonOffsetEnabled(1);
    /* all other caps (CLIP_PLANE0, STENCIL_TEST, ...) are handled
     * elsewhere or unsupported on GX — ignore */
}
static void ngx_route_disable(GLenum cap)
{
    if (cap == GL_TEXTURE_2D)
        GXBE_SetTexture2DEnabled(0);
    else if (cap == GL_POLYGON_OFFSET_FILL)
        GXBE_SetPolygonOffsetEnabled(0);
}
static void ngx_route_polygonoffset(GLfloat factor, GLfloat units)
{
    GXBE_PolygonOffset((float)factor, (float)units);
}

void QGL_Init(void)
{
    qglBindTexture          = noop_bind;
    qglBlendFunc            = noop_uu;
    qglClear                = noop_clear;
    qglClearColor           = noop_color4f;
    qglClearStencil         = noop_i;
    qglColorMask            = noop_bbbb;
    qglCullFace             = noop_u;
    qglDeleteTextures       = noop_nu;
    qglDepthFunc            = noop_u;
    qglDepthMask            = noop_b;
    qglDisable              = ngx_route_disable;
    qglDrawArrays           = noop_drawarrays;
    qglDrawElements         = noop_drawelements;
    qglEnable               = ngx_route_enable;
    qglFinish               = noop_void;
    qglFlush                = noop_void;
    qglGenTextures          = noop_ni;
    qglGetBooleanv          = noop_getbooleanv;
    qglGetError             = noop_ret_u;
    qglGetIntegerv          = noop_getintegerv;
    qglGetString            = noop_ret_str;
    qglLineWidth            = noop_f;
    qglPolygonOffset        = ngx_route_polygonoffset;
    qglReadPixels           = noop_readpixels;
    qglScissor              = noop_scissor;
    qglStencilFunc          = noop_stencil3;
    qglStencilMask          = noop_stencil_mask;
    qglStencilOp            = noop_stencil2;
    qglTexImage2D           = noop_teximage2d;
    qglTexParameterf        = noop_texparamf;
    qglTexParameterfv       = noop_texparamfv;
    qglTexParameteri        = noop_texparami;
    qglTexSubImage2D        = noop_texsubimage2d;
    qglViewport             = noop_viewport;

    qglAlphaFunc            = noop_alpharef;
    qglColor4f              = noop_color4f;
    qglColorPointer         = ngx_route_colorptr;
    qglDisableClientState   = noop_u;
    qglEnableClientState    = noop_u;
    qglLoadIdentity         = noop_void;
    qglLoadMatrixf          = noop_loadmatrix;
    qglMatrixMode           = noop_u;
    qglPopMatrix            = noop_void;
    qglPushMatrix           = noop_void;
    qglShadeModel           = noop_shademodel;
    qglTexCoordPointer      = ngx_route_texcoordptr;
    qglTexEnvf              = noop_texenvf;
    qglTranslatef           = noop_translatef;
    qglVertexPointer        = ngx_route_vertexptr;

    qglClearDepth           = noop_d;
    qglDepthRange           = ngx_route_depthrange;
    qglDrawBuffer           = noop_DrawBuffer;
    qglPolygonMode          = noop_PolygonMode;

    qglArrayElement         = noop_arrayelement;
    qglBegin                = noop_begin;
    qglCallList             = noop_calllist;
    qglClipPlane            = noop_clipplane;
    qglColor3f              = noop_color3f;
    qglColor4ubv            = noop_color4ubv;
    qglEnd                  = noop_void;
    qglFrustum              = noop_frustum;
    qglOrtho                = noop_ortho;
    qglTexCoord2f           = noop_texcoord2f;
    qglTexCoord2fv          = noop_texcoord2fv;
    qglVertex2f             = noop_vertex2f;
    qglVertex3f             = noop_vertex3f;
    qglVertex3fv            = noop_vertex3fv;

    qglActiveTextureARB       = noop_multitex;
    qglClientActiveTextureARB = noop_multitex;
    qglMultiTexCoord2fARB     = noop_multitexcoord;

    qglLockArraysEXT   = noop_LockArrays;
    qglUnlockArraysEXT = noop_UnlockArrays;
}

void QGL_Shutdown(void)
{
}
