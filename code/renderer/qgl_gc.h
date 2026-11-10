/* GL types and enums for the GX backend; no GL library on the GameCube. */
#ifndef __QGL_GC_H__
#define __QGL_GC_H__

#define APIENTRY
#define APIENTRYP APIENTRY *

typedef unsigned int	GLenum;
typedef unsigned int	GLbitfield;
typedef unsigned int	GLuint;
typedef int		GLint;
typedef int		GLsizei;
typedef unsigned char	GLboolean;
typedef unsigned char	GLubyte;
typedef signed char	GLbyte;
typedef short		GLshort;
typedef unsigned short	GLushort;
typedef float		GLfloat;
typedef float		GLclampf;
typedef double		GLdouble;
typedef double		GLclampd;
#define GLvoid void

#define GL_ADD                               0x0104
#define GL_ALPHA_TEST                        0x0BC0
#define GL_ALWAYS                            0x0207
#define GL_BACK                              0x0405
#define GL_BACK_LEFT                         0x0402
#define GL_BACK_RIGHT                        0x0403
#define GL_BLEND                             0x0BE2
#define GL_CLAMP_TO_EDGE                     0x812F
#define GL_CLIP_PLANE0                       0x3000
#define GL_COLOR_ARRAY                       0x8076
#define GL_COLOR_BUFFER_BIT                  0x00004000
#define GL_COLOR_WRITEMASK                   0x0C23
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT     0x83F1
#define GL_CULL_FACE                         0x0B44
#define GL_DECAL                             0x2101
#define GL_DECR                              0x1E03
#define GL_DEPTH_BUFFER_BIT                  0x00000100
#define GL_DEPTH_COMPONENT                   0x1902
#define GL_DEPTH_TEST                        0x0B71
#define GL_DST_ALPHA                         0x0304
#define GL_DST_COLOR                         0x0306
#define GL_EQUAL                             0x0202
#define GL_EXTENSIONS                        0x1F03
#define GL_FALSE                             0x0
#define GL_FILL                              0x1B02
#define GL_FLAT                              0x1D00
#define GL_FLOAT                             0x1406
#define GL_FRONT                             0x0404
#define GL_FRONT_AND_BACK                    0x0408
#define GL_GEQUAL                            0x0206
#define GL_GREATER                           0x0204
#define GL_INCR                              0x1E02
#define GL_INVALID_ENUM                      0x0500
#define GL_INVALID_OPERATION                 0x0502
#define GL_INVALID_VALUE                     0x0501
#define GL_KEEP                              0x1E00
#define GL_LEQUAL                            0x0203
#define GL_LESS                              0x0201
#define GL_LINE                              0x1B01
#define GL_LINEAR                            0x2601
#define GL_LINEAR_MIPMAP_LINEAR              0x2703
#define GL_LINEAR_MIPMAP_NEAREST             0x2701
#define GL_LINES                             0x0001
#define GL_LUMINANCE                         0x1909
#define GL_LUMINANCE16                       0x8042
#define GL_LUMINANCE16_ALPHA16               0x8048
#define GL_LUMINANCE8                        0x8040
#define GL_LUMINANCE8_ALPHA8                 0x8045
#define GL_LUMINANCE_ALPHA                   0x190A
#define GL_MAX_TEXTURE_SIZE                  0x0D33
#define GL_MAX_TEXTURE_UNITS_ARB             0x84E2
#define GL_MODELVIEW                         0x1700
#define GL_MODULATE                          0x2100
#define GL_NEAREST                           0x2600
#define GL_NEAREST_MIPMAP_LINEAR             0x2702
#define GL_NEAREST_MIPMAP_NEAREST            0x2700
#define GL_NOTEQUAL                          0x0205
#define GL_NO_ERROR                          0x0
#define GL_ONE                               0x1
#define GL_ONE_MINUS_DST_ALPHA               0x0305
#define GL_ONE_MINUS_DST_COLOR               0x0307
#define GL_ONE_MINUS_SRC_ALPHA               0x0303
#define GL_ONE_MINUS_SRC_COLOR               0x0301
#define GL_OUT_OF_MEMORY                     0x0505
#define GL_PACK_ALIGNMENT                    0x0D05
#define GL_POLYGON                           0x0009
#define GL_POLYGON_OFFSET_FILL               0x8037
#define GL_PROJECTION                        0x1701
#define GL_QUADS                             0x0007
#define GL_RENDERER                          0x1F01
#define GL_REPEAT                            0x2901
#define GL_REPLACE                           0x1E01
#define GL_RGB                               0x1907
#define GL_RGB4_S3TC                         0x83A1
#define GL_RGB5                              0x8050
#define GL_RGB8                              0x8051
#define GL_RGBA                              0x1908
#define GL_RGBA4                             0x8056
#define GL_RGBA8                             0x8058
#define GL_SCISSOR_TEST                      0x0C11
#define GL_SMOOTH                            0x1D01
#define GL_SRC_ALPHA                         0x0302
#define GL_SRC_ALPHA_SATURATE                0x0308
#define GL_SRC_COLOR                         0x0300
#define GL_STACK_OVERFLOW                    0x0503
#define GL_STACK_UNDERFLOW                   0x0504
#define GL_STENCIL_BUFFER_BIT                0x00000400
#define GL_STENCIL_INDEX                     0x1901
#define GL_STENCIL_TEST                      0x0B90
#define GL_TEXTURE0_ARB                      0x84C0
#define GL_TEXTURE1_ARB                      0x84C1
#define GL_TEXTURE_2D                        0x0DE1
#define GL_TEXTURE_BORDER_COLOR              0x1004
#define GL_TEXTURE_COORD_ARRAY               0x8078
#define GL_TEXTURE_ENV                       0x2300
#define GL_TEXTURE_ENV_MODE                  0x2200
#define GL_TEXTURE_MAG_FILTER                0x2800
#define GL_TEXTURE_MAX_ANISOTROPY_EXT        0x84FE
#define GL_TEXTURE_MIN_FILTER                0x2801
#define GL_TEXTURE_WRAP_S                    0x2802
#define GL_TEXTURE_WRAP_T                    0x2803
#define GL_TRIANGLES                         0x0004
#define GL_TRIANGLE_FAN                      0x0006
#define GL_TRIANGLE_STRIP                    0x0005
#define GL_TRUE                              0x1
#define GL_UNSIGNED_BYTE                     0x1401
#define GL_UNSIGNED_INT                      0x1405
#define GL_VERSION                           0x1F02
#define GL_VERTEX_ARRAY                      0x8074
#define GL_ZERO                              0x0

extern void (APIENTRYP qglActiveTextureARB) (GLenum texture);
extern void (APIENTRYP qglClientActiveTextureARB) (GLenum texture);
extern void (APIENTRYP qglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);

extern void (APIENTRYP qglLockArraysEXT) (GLint first, GLsizei count);
extern void (APIENTRYP qglUnlockArraysEXT) (void);

extern void (APIENTRYP qglAlphaFunc) (GLenum func, GLclampf ref);
extern void (APIENTRYP qglArrayElement) (GLint i);
extern void (APIENTRYP qglBegin) (GLenum mode);
extern void (APIENTRYP qglBindTexture) (GLenum target, GLuint texture);
extern void (APIENTRYP qglBlendFunc) (GLenum sfactor, GLenum dfactor);
extern void (APIENTRYP qglCallList) (GLuint list);
extern void (APIENTRYP qglClear) (GLbitfield mask);
extern void (APIENTRYP qglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void (APIENTRYP qglClearDepth) (GLclampd depth);
extern void (APIENTRYP qglClearStencil) (GLint s);
extern void (APIENTRYP qglClipPlane) (GLenum plane, const GLdouble *equation);
extern void (APIENTRYP qglColor3f) (GLfloat red, GLfloat green, GLfloat blue);
extern void (APIENTRYP qglColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (APIENTRYP qglColor4ubv) (const GLubyte *v);
extern void (APIENTRYP qglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void (APIENTRYP qglColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void (APIENTRYP qglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRYP qglCullFace) (GLenum mode);
extern void (APIENTRYP qglDeleteTextures) (GLsizei n, const GLuint *textures);
extern void (APIENTRYP qglDepthFunc) (GLenum func);
extern void (APIENTRYP qglDepthMask) (GLboolean flag);
extern void (APIENTRYP qglDepthRange) (GLclampd zNear, GLclampd zFar);
extern void (APIENTRYP qglDisable) (GLenum cap);
extern void (APIENTRYP qglDisableClientState) (GLenum array);
extern void (APIENTRYP qglDrawArrays) (GLenum mode, GLint first, GLsizei count);
extern void (APIENTRYP qglDrawBuffer) (GLenum mode);
extern void (APIENTRYP qglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
extern void (APIENTRYP qglEnable) (GLenum cap);
extern void (APIENTRYP qglEnableClientState) (GLenum array);
extern void (APIENTRYP qglEnd) (void);
extern void (APIENTRYP qglFinish) (void);
extern void (APIENTRYP qglFlush) (void);
extern void (APIENTRYP qglFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void (APIENTRYP qglGenTextures) (GLsizei n, GLuint *textures);
extern void (APIENTRYP qglGetBooleanv) (GLenum pname, GLboolean *params);
extern GLenum (APIENTRYP qglGetError) (void);
extern void (APIENTRYP qglGetIntegerv) (GLenum pname, GLint *params);
extern const GLubyte *(APIENTRYP qglGetString) (GLenum name);
extern void (APIENTRYP qglLineWidth) (GLfloat width);
extern void (APIENTRYP qglLoadIdentity) (void);
extern void (APIENTRYP qglLoadMatrixf) (const GLfloat *m);
extern void (APIENTRYP qglMatrixMode) (GLenum mode);
extern void (APIENTRYP qglOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void (APIENTRYP qglPolygonMode) (GLenum face, GLenum mode);
extern void (APIENTRYP qglPolygonOffset) (GLfloat factor, GLfloat units);
extern void (APIENTRYP qglPopMatrix) (void);
extern void (APIENTRYP qglPushMatrix) (void);
extern void (APIENTRYP qglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern void (APIENTRYP qglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRYP qglShadeModel) (GLenum mode);
extern void (APIENTRYP qglStencilFunc) (GLenum func, GLint ref, GLuint mask);
extern void (APIENTRYP qglStencilMask) (GLuint mask);
extern void (APIENTRYP qglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
extern void (APIENTRYP qglTexCoord2f) (GLfloat s, GLfloat t);
extern void (APIENTRYP qglTexCoord2fv) (const GLfloat *v);
extern void (APIENTRYP qglTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void (APIENTRYP qglTexEnvf) (GLenum target, GLenum pname, GLfloat param);
extern void (APIENTRYP qglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRYP qglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
extern void (APIENTRYP qglTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
extern void (APIENTRYP qglTexParameteri) (GLenum target, GLenum pname, GLint param);
extern void (APIENTRYP qglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRYP qglTranslatef) (GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRYP qglVertex2f) (GLfloat x, GLfloat y);
extern void (APIENTRYP qglVertex3f) (GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRYP qglVertex3fv) (const GLfloat *v);
extern void (APIENTRYP qglVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void (APIENTRYP qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

void QGL_Init(void);
void QGL_Shutdown(void);

#endif
