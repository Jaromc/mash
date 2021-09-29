//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_HEADER_H_
#define _C_MASH_OPENGL_HEADER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "windows.h"

#include <gl\gl.h>
#include "wglext.h"
#include "glext.h"

#elif defined (MASH_APPLE)

//define legacy so that old glext.h files don't get included
#define GL_GLEXT_LEGACY 1

#include <OpenGL/gl.h>

#undef GL_GLEXT_LEGACY

#define GL_GLEXT_PROTOTYPES
#include "glext.h"

#elif defined (MASH_LINUX)
#define GL_GLEXT_LEGACY 1

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

#undef GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include "glext.h"

#endif

#if defined (MASH_LINUX) || defined (MASH_WINDOWS)
extern PFNGLGETSTRINGIPROC glGetStringiPtr;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLogPtr;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRangePtr;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRangePtr;
extern PFNGLGENBUFFERSPROC glGenBuffersPtr;
extern PFNGLBINDBUFFERPROC	glBindBufferPtr;
extern PFNGLBUFFERDATAPROC	glBufferDataPtr;
extern PFNGLBUFFERSUBDATAPROC glBufferSubDataPtr;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointerPtr;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatusPtr;

extern PFNGLGENRENDERBUFFERSPROC glGenFramebuffersPtr;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteFramebuffersPtr;
extern PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1DPtr;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3DPtr;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbufferPtr;
extern PFNGLDRAWBUFFERSPROC glDrawBuffersPtr;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPtr;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstancedPtr;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstancedPtr;

extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffersPtr;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffersPtr;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbufferPtr;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStoragePtr;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameterivPtr;

extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArrayPtr;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArrayPtr;
extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparatePtr;
extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparatePtr;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmapPtr;

extern PFNGLUNMAPBUFFERPROC glUnmapBufferPtr;
extern PFNGLMAPBUFFERPROC glMapBufferPtr;
extern PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameterivPtr;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffersPtr;

extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocationPtr;
extern PFNGLCREATEPROGRAMPROC glCreateProgramPtr;
extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniformPtr;
extern PFNGLGETPROGRAMIVPROC glGetProgramivPtr;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocationPtr;
extern PFNGLLINKPROGRAMPROC glLinkProgramPtr;
extern PFNGLUSEPROGRAMPROC glUseProgramPtr;
extern PFNGLATTACHSHADERPROC glAttachShaderPtr;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndexPtr;
extern PFNGLDETACHSHADERPROC glDetachShaderPtr;
extern PFNGLDELETESHADERPROC glDeleteShaderPtr;

extern PFNGLCOMPILESHADERPROC glCompileShaderPtr;
extern PFNGLCREATESHADERPROC glCreateShaderPtr;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLogPtr;
extern PFNGLGETSHADERIVPROC glGetShaderivPtr;
extern PFNGLSHADERSOURCEPROC glShaderSourcePtr;
extern PFNGLDELETEPROGRAMPROC glDeleteProgramPtr;

extern PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocationPtr;
extern PFNGLGETACTIVEATTRIBPROC glGetActiveAttribPtr;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocationPtr;
extern PFNGLACTIVETEXTUREPROC glActiveTexturePtr;

extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockNamePtr;
extern PFNGLBINDBUFFERRANGEPROC glBindBufferRangePtr;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBindingPtr;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBasePtr;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockivPtr;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fvPtr;
extern PFNGLUNIFORM1IPROC glUniform1iPtr;
extern PFNGLUNIFORM1IVPROC glUniform1ivPtr;
extern PFNGLUNIFORM1FPROC glUniform1fPtr;
extern PFNGLUNIFORM1FVPROC glUniform1fvPtr;
extern PFNGLUNIFORM2IVPROC glUniform2ivPtr;
extern PFNGLUNIFORM2IPROC glUniform2iPtr;
extern PFNGLUNIFORM2FVPROC glUniform2fvPtr;
extern PFNGLUNIFORM2FPROC glUniform2fPtr;
extern PFNGLUNIFORM3IVPROC glUniform3ivPtr;
extern PFNGLUNIFORM3IPROC glUniform3iPtr;
extern PFNGLUNIFORM3FVPROC glUniform3fvPtr;
extern PFNGLUNIFORM3FPROC glUniform3fPtr;
extern PFNGLUNIFORM4IVPROC glUniform4ivPtr;
extern PFNGLUNIFORM4IPROC glUniform4iPtr;
extern PFNGLUNIFORM4FVPROC glUniform4fvPtr;
extern PFNGLUNIFORM4FPROC glUniform4fPtr;

extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisorPtr;

#elif defined (MASH_APPLE)

typedef void (* glAccumProcPtr) (GLenum op, GLfloat value);
typedef void (* glAlphaFuncProcPtr) (GLenum func, GLclampf ref);
typedef GLboolean (* glAreTexturesResidentProcPtr) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (* glArrayElementProcPtr) (GLint i);
typedef void (* glBeginProcPtr) (GLenum mode);
typedef void (* glBindTextureProcPtr) (GLenum target, GLuint texture);
typedef void (* glBitmapProcPtr) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (* glBlendColorProcPtr) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (* glBlendEquationProcPtr) (GLenum mode);
typedef void (* glBlendEquationSeparateProcPtr) (GLenum modeRGB, GLenum modeAlpha);
typedef void (* glBlendFuncProcPtr) (GLenum sfactor, GLenum dfactor);
typedef void (* glCallListProcPtr) (GLuint list);
typedef void (* glCallListsProcPtr) (GLsizei n, GLenum type, const GLvoid *lists);
typedef void (* glClearProcPtr) (GLbitfield mask);
typedef void (* glClearAccumProcPtr) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (* glClearColorProcPtr) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (* glClearDepthProcPtr) (GLclampd depth);
typedef void (* glClearIndexProcPtr) (GLfloat c);
typedef void (* glClearStencilProcPtr) (GLint s);
typedef void (* glClipPlaneProcPtr) (GLenum plane, const GLdouble *equation);
typedef void (* glColor3bProcPtr) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (* glColor3bvProcPtr) (const GLbyte *v);
typedef void (* glColor3dProcPtr) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (* glColor3dvProcPtr) (const GLdouble *v);
typedef void (* glColor3fProcPtr) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (* glColor3fvProcPtr) (const GLfloat *v);
typedef void (* glColor3iProcPtr) (GLint red, GLint green, GLint blue);
typedef void (* glColor3ivProcPtr) (const GLint *v);
typedef void (* glColor3sProcPtr) (GLshort red, GLshort green, GLshort blue);
typedef void (* glColor3svProcPtr) (const GLshort *v);
typedef void (* glColor3ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (* glColor3ubvProcPtr) (const GLubyte *v);
typedef void (* glColor3uiProcPtr) (GLuint red, GLuint green, GLuint blue);
typedef void (* glColor3uivProcPtr) (const GLuint *v);
typedef void (* glColor3usProcPtr) (GLushort red, GLushort green, GLushort blue);
typedef void (* glColor3usvProcPtr) (const GLushort *v);
typedef void (* glColor4bProcPtr) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef void (* glColor4bvProcPtr) (const GLbyte *v);
typedef void (* glColor4dProcPtr) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void (* glColor4dvProcPtr) (const GLdouble *v);
typedef void (* glColor4fProcPtr) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (* glColor4fvProcPtr) (const GLfloat *v);
typedef void (* glColor4iProcPtr) (GLint red, GLint green, GLint blue, GLint alpha);
typedef void (* glColor4ivProcPtr) (const GLint *v);
typedef void (* glColor4sProcPtr) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef void (* glColor4svProcPtr) (const GLshort *v);
typedef void (* glColor4ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (* glColor4ubvProcPtr) (const GLubyte *v);
typedef void (* glColor4uiProcPtr) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef void (* glColor4uivProcPtr) (const GLuint *v);
typedef void (* glColor4usProcPtr) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef void (* glColor4usvProcPtr) (const GLushort *v);
typedef void (* glColorMaskProcPtr) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (* glColorMaterialProcPtr) (GLenum face, GLenum mode);
typedef void (* glColorPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glColorSubTableProcPtr) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (* glColorTableProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (* glColorTableParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glColorTableParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glConvolutionFilter1DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (* glConvolutionFilter2DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (* glConvolutionParameterfProcPtr) (GLenum target, GLenum pname, GLfloat params);
typedef void (* glConvolutionParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glConvolutionParameteriProcPtr) (GLenum target, GLenum pname, GLint params);
typedef void (* glConvolutionParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glCopyColorSubTableProcPtr) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (* glCopyColorTableProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (* glCopyConvolutionFilter1DProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (* glCopyConvolutionFilter2DProcPtr) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCopyPixelsProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void (* glCopyTexImage1DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (* glCopyTexImage2DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (* glCopyTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (* glCopyTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCopyTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glCullFaceProcPtr) (GLenum mode);
typedef void (* glDeleteListsProcPtr) (GLuint list, GLsizei range);
typedef void (* glDeleteTexturesProcPtr) (GLsizei n, const GLuint *textures);
typedef void (* glDepthFuncProcPtr) (GLenum func);
typedef void (* glDepthMaskProcPtr) (GLboolean flag);
typedef void (* glDepthRangeProcPtr) (GLclampd zNear, GLclampd zFar);
typedef void (* glDisableProcPtr) (GLenum cap);
typedef void (* glDisableClientStateProcPtr) (GLenum array);
typedef void (* glDrawArraysProcPtr) (GLenum mode, GLint first, GLsizei count);
typedef void (* glDrawBufferProcPtr) (GLenum mode);
typedef void (* glDrawElementsProcPtr) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (* glDrawPixelsProcPtr) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glDrawRangeElementsProcPtr) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (* glEdgeFlagProcPtr) (GLboolean flag);
typedef void (* glEdgeFlagPointerProcPtr) (GLsizei stride, const GLvoid *pointer);
typedef void (* glEdgeFlagvProcPtr) (const GLboolean *flag);
typedef void (* glEnableProcPtr) (GLenum cap);
typedef void (* glEnableClientStateProcPtr) (GLenum array);
typedef void (* glEndProcPtr) (void);
typedef void (* glEndListProcPtr) (void);
typedef void (* glEvalCoord1dProcPtr) (GLdouble u);
typedef void (* glEvalCoord1dvProcPtr) (const GLdouble *u);
typedef void (* glEvalCoord1fProcPtr) (GLfloat u);
typedef void (* glEvalCoord1fvProcPtr) (const GLfloat *u);
typedef void (* glEvalCoord2dProcPtr) (GLdouble u, GLdouble v);
typedef void (* glEvalCoord2dvProcPtr) (const GLdouble *u);
typedef void (* glEvalCoord2fProcPtr) (GLfloat u, GLfloat v);
typedef void (* glEvalCoord2fvProcPtr) (const GLfloat *u);
typedef void (* glEvalMesh1ProcPtr) (GLenum mode, GLint i1, GLint i2);
typedef void (* glEvalMesh2ProcPtr) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef void (* glEvalPoint1ProcPtr) (GLint i);
typedef void (* glEvalPoint2ProcPtr) (GLint i, GLint j);
typedef void (* glFeedbackBufferProcPtr) (GLsizei size, GLenum type, GLfloat *buffer);
typedef void (* glFinishProcPtr) (void);
typedef void (* glFlushProcPtr) (void);
typedef void (* glFogfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glFogfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glFogiProcPtr) (GLenum pname, GLint param);
typedef void (* glFogivProcPtr) (GLenum pname, const GLint *params);
typedef void (* glFrontFaceProcPtr) (GLenum mode);
typedef void (* glFrustumProcPtr) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef GLuint (* glGenListsProcPtr) (GLsizei range);
typedef void (* glGenTexturesProcPtr) (GLsizei n, GLuint *textures);
typedef void (* glGetBooleanvProcPtr) (GLenum pname, GLboolean *params);
typedef void (* glGetClipPlaneProcPtr) (GLenum plane, GLdouble *equation);
typedef void (* glGetColorTableProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (* glGetColorTableParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetColorTableParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetConvolutionFilterProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (* glGetConvolutionParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetConvolutionParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetDoublevProcPtr) (GLenum pname, GLdouble *params);
typedef GLenum (* glGetErrorProcPtr) (void);
typedef void (* glGetFloatvProcPtr) (GLenum pname, GLfloat *params);
typedef void (* glGetHistogramProcPtr) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (* glGetHistogramParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetHistogramParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetIntegervProcPtr) (GLenum pname, GLint *params);
typedef void (* glGetLightfvProcPtr) (GLenum light, GLenum pname, GLfloat *params);
typedef void (* glGetLightivProcPtr) (GLenum light, GLenum pname, GLint *params);
typedef void (* glGetMapdvProcPtr) (GLenum target, GLenum query, GLdouble *v);
typedef void (* glGetMapfvProcPtr) (GLenum target, GLenum query, GLfloat *v);
typedef void (* glGetMapivProcPtr) (GLenum target, GLenum query, GLint *v);
typedef void (* glGetMaterialfvProcPtr) (GLenum face, GLenum pname, GLfloat *params);
typedef void (* glGetMaterialivProcPtr) (GLenum face, GLenum pname, GLint *params);
typedef void (* glGetMinmaxProcPtr) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (* glGetMinmaxParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetMinmaxParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetPixelMapfvProcPtr) (GLenum map, GLfloat *values);
typedef void (* glGetPixelMapuivProcPtr) (GLenum map, GLuint *values);
typedef void (* glGetPixelMapusvProcPtr) (GLenum map, GLushort *values);
typedef void (* glGetPointervProcPtr) (GLenum pname, GLvoid **params);
typedef void (* glGetPolygonStippleProcPtr) (GLubyte *mask);
typedef void (* glGetSeparableFilterProcPtr) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef const GLubyte *(* glGetStringProcPtr) (GLenum name);
typedef void (* glGetTexEnvfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetTexEnvivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetTexGendvProcPtr) (GLenum coord, GLenum pname, GLdouble *params);
typedef void (* glGetTexGenfvProcPtr) (GLenum coord, GLenum pname, GLfloat *params);
typedef void (* glGetTexGenivProcPtr) (GLenum coord, GLenum pname, GLint *params);
typedef void (* glGetTexImageProcPtr) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void (* glGetTexLevelParameterfvProcPtr) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (* glGetTexLevelParameterivProcPtr) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (* glGetTexParameterfvProcPtr) (GLenum target, GLenum pname, GLfloat *params);
typedef void (* glGetTexParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glHintProcPtr) (GLenum target, GLenum mode);
typedef void (* glHistogramProcPtr) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (* glIndexMaskProcPtr) (GLuint mask);
typedef void (* glIndexPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glIndexdProcPtr) (GLdouble c);
typedef void (* glIndexdvProcPtr) (const GLdouble *c);
typedef void (* glIndexfProcPtr) (GLfloat c);
typedef void (* glIndexfvProcPtr) (const GLfloat *c);
typedef void (* glIndexiProcPtr) (GLint c);
typedef void (* glIndexivProcPtr) (const GLint *c);
typedef void (* glIndexsProcPtr) (GLshort c);
typedef void (* glIndexsvProcPtr) (const GLshort *c);
typedef void (* glIndexubProcPtr) (GLubyte c);
typedef void (* glIndexubvProcPtr) (const GLubyte *c);
typedef void (* glInitNamesProcPtr) (void);
typedef void (* glInterleavedArraysProcPtr) (GLenum format, GLsizei stride, const GLvoid *pointer);
typedef GLboolean (* glIsEnabledProcPtr) (GLenum cap);
typedef GLboolean (* glIsListProcPtr) (GLuint list);
typedef GLboolean (* glIsTextureProcPtr) (GLuint texture);
typedef void (* glLightModelfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glLightModelfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glLightModeliProcPtr) (GLenum pname, GLint param);
typedef void (* glLightModelivProcPtr) (GLenum pname, const GLint *params);
typedef void (* glLightfProcPtr) (GLenum light, GLenum pname, GLfloat param);
typedef void (* glLightfvProcPtr) (GLenum light, GLenum pname, const GLfloat *params);
typedef void (* glLightiProcPtr) (GLenum light, GLenum pname, GLint param);
typedef void (* glLightivProcPtr) (GLenum light, GLenum pname, const GLint *params);
typedef void (* glLineStippleProcPtr) (GLint factor, GLushort pattern);
typedef void (* glLineWidthProcPtr) (GLfloat width);
typedef void (* glListBaseProcPtr) (GLuint base);
typedef void (* glLoadIdentityProcPtr) (void);
typedef void (* glLoadMatrixdProcPtr) (const GLdouble *m);
typedef void (* glLoadMatrixfProcPtr) (const GLfloat *m);
typedef void (* glLoadNameProcPtr) (GLuint name);
typedef void (* glLogicOpProcPtr) (GLenum opcode);
typedef void (* glMap1dProcPtr) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void (* glMap1fProcPtr) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void (* glMap2dProcPtr) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void (* glMap2fProcPtr) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
typedef void (* glMapGrid1dProcPtr) (GLint un, GLdouble u1, GLdouble u2);
typedef void (* glMapGrid1fProcPtr) (GLint un, GLfloat u1, GLfloat u2);
typedef void (* glMapGrid2dProcPtr) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef void (* glMapGrid2fProcPtr) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef void (* glMaterialfProcPtr) (GLenum face, GLenum pname, GLfloat param);
typedef void (* glMaterialfvProcPtr) (GLenum face, GLenum pname, const GLfloat *params);
typedef void (* glMaterialiProcPtr) (GLenum face, GLenum pname, GLint param);
typedef void (* glMaterialivProcPtr) (GLenum face, GLenum pname, const GLint *params);
typedef void (* glMatrixModeProcPtr) (GLenum mode);
typedef void (* glMinmaxProcPtr) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (* glMultMatrixdProcPtr) (const GLdouble *m);
typedef void (* glMultMatrixfProcPtr) (const GLfloat *m);
typedef void (* glNewListProcPtr) (GLuint list, GLenum mode);
typedef void (* glNormal3bProcPtr) (GLbyte nx, GLbyte ny, GLbyte nz);
typedef void (* glNormal3bvProcPtr) (const GLbyte *v);
typedef void (* glNormal3dProcPtr) (GLdouble nx, GLdouble ny, GLdouble nz);
typedef void (* glNormal3dvProcPtr) (const GLdouble *v);
typedef void (* glNormal3fProcPtr) (GLfloat nx, GLfloat ny, GLfloat nz);
typedef void (* glNormal3fvProcPtr) (const GLfloat *v);
typedef void (* glNormal3iProcPtr) (GLint nx, GLint ny, GLint nz);
typedef void (* glNormal3ivProcPtr) (const GLint *v);
typedef void (* glNormal3sProcPtr) (GLshort nx, GLshort ny, GLshort nz);
typedef void (* glNormal3svProcPtr) (const GLshort *v);
typedef void (* glNormalPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glOrthoProcPtr) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (* glPassThroughProcPtr) (GLfloat token);
typedef void (* glPixelMapfvProcPtr) (GLenum map, GLint mapsize, const GLfloat *values);
typedef void (* glPixelMapuivProcPtr) (GLenum map, GLint mapsize, const GLuint *values);
typedef void (* glPixelMapusvProcPtr) (GLenum map, GLint mapsize, const GLushort *values);
typedef void (* glPixelStorefProcPtr) (GLenum pname, GLfloat param);
typedef void (* glPixelStoreiProcPtr) (GLenum pname, GLint param);
typedef void (* glPixelTransferfProcPtr) (GLenum pname, GLfloat param);
typedef void (* glPixelTransferiProcPtr) (GLenum pname, GLint param);
typedef void (* glPixelZoomProcPtr) (GLfloat xfactor, GLfloat yfactor);
typedef void (* glPointSizeProcPtr) (GLfloat size);
typedef void (* glPolygonModeProcPtr) (GLenum face, GLenum mode);
typedef void (* glPolygonOffsetProcPtr) (GLfloat factor, GLfloat units);
typedef void (* glPolygonStippleProcPtr) (const GLubyte *mask);
typedef void (* glPopAttribProcPtr) (void);
typedef void (* glPopClientAttribProcPtr) (void);
typedef void (* glPopMatrixProcPtr) (void);
typedef void (* glPopNameProcPtr) (void);
typedef void (* glPrioritizeTexturesProcPtr) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (* glPushAttribProcPtr) (GLbitfield mask);
typedef void (* glPushClientAttribProcPtr) (GLbitfield mask);
typedef void (* glPushMatrixProcPtr) (void);
typedef void (* glPushNameProcPtr) (GLuint name);
typedef void (* glRasterPos2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glRasterPos2dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glRasterPos2fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos2iProcPtr) (GLint x, GLint y);
typedef void (* glRasterPos2ivProcPtr) (const GLint *v);
typedef void (* glRasterPos2sProcPtr) (GLshort x, GLshort y);
typedef void (* glRasterPos2svProcPtr) (const GLshort *v);
typedef void (* glRasterPos3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glRasterPos3dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glRasterPos3fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glRasterPos3ivProcPtr) (const GLint *v);
typedef void (* glRasterPos3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glRasterPos3svProcPtr) (const GLshort *v);
typedef void (* glRasterPos4dProcPtr) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glRasterPos4dvProcPtr) (const GLdouble *v);
typedef void (* glRasterPos4fProcPtr) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glRasterPos4fvProcPtr) (const GLfloat *v);
typedef void (* glRasterPos4iProcPtr) (GLint x, GLint y, GLint z, GLint w);
typedef void (* glRasterPos4ivProcPtr) (const GLint *v);
typedef void (* glRasterPos4sProcPtr) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glRasterPos4svProcPtr) (const GLshort *v);
typedef void (* glReadBufferProcPtr) (GLenum mode);
typedef void (* glReadPixelsProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (* glRectdProcPtr) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void (* glRectdvProcPtr) (const GLdouble *v1, const GLdouble *v2);
typedef void (* glRectfProcPtr) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void (* glRectfvProcPtr) (const GLfloat *v1, const GLfloat *v2);
typedef void (* glRectiProcPtr) (GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (* glRectivProcPtr) (const GLint *v1, const GLint *v2);
typedef void (* glRectsProcPtr) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void (* glRectsvProcPtr) (const GLshort *v1, const GLshort *v2);
typedef GLint (* glRenderModeProcPtr) (GLenum mode);
typedef void (* glResetHistogramProcPtr) (GLenum target);
typedef void (* glResetMinmaxProcPtr) (GLenum target);
typedef void (* glRotatedProcPtr) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void (* glRotatefProcPtr) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void (* glScaledProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glScalefProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glScissorProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (* glSelectBufferProcPtr) (GLsizei size, GLuint *buffer);
typedef void (* glSeparableFilter2DProcPtr) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (* glShadeModelProcPtr) (GLenum mode);
typedef void (* glStencilFuncProcPtr) (GLenum func, GLint ref, GLuint mask);
typedef void (* glStencilMaskProcPtr) (GLuint mask);
typedef void (* glStencilOpProcPtr) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (* glTexCoord1dProcPtr) (GLdouble s);
typedef void (* glTexCoord1dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord1fProcPtr) (GLfloat s);
typedef void (* glTexCoord1fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord1iProcPtr) (GLint s);
typedef void (* glTexCoord1ivProcPtr) (const GLint *v);
typedef void (* glTexCoord1sProcPtr) (GLshort s);
typedef void (* glTexCoord1svProcPtr) (const GLshort *v);
typedef void (* glTexCoord2dProcPtr) (GLdouble s, GLdouble t);
typedef void (* glTexCoord2dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord2fProcPtr) (GLfloat s, GLfloat t);
typedef void (* glTexCoord2fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord2iProcPtr) (GLint s, GLint t);
typedef void (* glTexCoord2ivProcPtr) (const GLint *v);
typedef void (* glTexCoord2sProcPtr) (GLshort s, GLshort t);
typedef void (* glTexCoord2svProcPtr) (const GLshort *v);
typedef void (* glTexCoord3dProcPtr) (GLdouble s, GLdouble t, GLdouble r);
typedef void (* glTexCoord3dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord3fProcPtr) (GLfloat s, GLfloat t, GLfloat r);
typedef void (* glTexCoord3fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord3iProcPtr) (GLint s, GLint t, GLint r);
typedef void (* glTexCoord3ivProcPtr) (const GLint *v);
typedef void (* glTexCoord3sProcPtr) (GLshort s, GLshort t, GLshort r);
typedef void (* glTexCoord3svProcPtr) (const GLshort *v);
typedef void (* glTexCoord4dProcPtr) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (* glTexCoord4dvProcPtr) (const GLdouble *v);
typedef void (* glTexCoord4fProcPtr) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (* glTexCoord4fvProcPtr) (const GLfloat *v);
typedef void (* glTexCoord4iProcPtr) (GLint s, GLint t, GLint r, GLint q);
typedef void (* glTexCoord4ivProcPtr) (const GLint *v);
typedef void (* glTexCoord4sProcPtr) (GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (* glTexCoord4svProcPtr) (const GLshort *v);
typedef void (* glTexCoordPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glTexEnvfProcPtr) (GLenum target, GLenum pname, GLfloat param);
typedef void (* glTexEnvfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glTexEnviProcPtr) (GLenum target, GLenum pname, GLint param);
typedef void (* glTexEnvivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glTexGendProcPtr) (GLenum coord, GLenum pname, GLdouble param);
typedef void (* glTexGendvProcPtr) (GLenum coord, GLenum pname, const GLdouble *params);
typedef void (* glTexGenfProcPtr) (GLenum coord, GLenum pname, GLfloat param);
typedef void (* glTexGenfvProcPtr) (GLenum coord, GLenum pname, const GLfloat *params);
typedef void (* glTexGeniProcPtr) (GLenum coord, GLenum pname, GLint param);
typedef void (* glTexGenivProcPtr) (GLenum coord, GLenum pname, const GLint *params);
typedef void (* glTexImage1DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexImage2DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexImage3DProcPtr) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexParameterfProcPtr) (GLenum target, GLenum pname, GLfloat param);
typedef void (* glTexParameterfvProcPtr) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (* glTexParameteriProcPtr) (GLenum target, GLenum pname, GLint param);
typedef void (* glTexParameterivProcPtr) (GLenum target, GLenum pname, const GLint *params);
typedef void (* glTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (* glTranslatedProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glTranslatefProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertex2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glVertex2dvProcPtr) (const GLdouble *v);
typedef void (* glVertex2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glVertex2fvProcPtr) (const GLfloat *v);
typedef void (* glVertex2iProcPtr) (GLint x, GLint y);
typedef void (* glVertex2ivProcPtr) (const GLint *v);
typedef void (* glVertex2sProcPtr) (GLshort x, GLshort y);
typedef void (* glVertex2svProcPtr) (const GLshort *v);
typedef void (* glVertex3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glVertex3dvProcPtr) (const GLdouble *v);
typedef void (* glVertex3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertex3fvProcPtr) (const GLfloat *v);
typedef void (* glVertex3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glVertex3ivProcPtr) (const GLint *v);
typedef void (* glVertex3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glVertex3svProcPtr) (const GLshort *v);
typedef void (* glVertex4dProcPtr) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glVertex4dvProcPtr) (const GLdouble *v);
typedef void (* glVertex4fProcPtr) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glVertex4fvProcPtr) (const GLfloat *v);
typedef void (* glVertex4iProcPtr) (GLint x, GLint y, GLint z, GLint w);
typedef void (* glVertex4ivProcPtr) (const GLint *v);
typedef void (* glVertex4sProcPtr) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glVertex4svProcPtr) (const GLshort *v);
typedef void (* glVertexPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (* glViewportProcPtr) (GLint x, GLint y, GLsizei width, GLsizei height);

typedef void (* glSampleCoverageProcPtr) (GLclampf value, GLboolean invert);

typedef void (* glLoadTransposeMatrixfProcPtr) (const GLfloat *m);
typedef void (* glLoadTransposeMatrixdProcPtr) (const GLdouble *m);
typedef void (* glMultTransposeMatrixfProcPtr) (const GLfloat *m);
typedef void (* glMultTransposeMatrixdProcPtr) (const GLdouble *m);

typedef void (* glCompressedTexImage3DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexImage2DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexImage1DProcPtr) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage3DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage2DProcPtr) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glCompressedTexSubImage1DProcPtr) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (* glGetCompressedTexImageProcPtr) (GLenum target, GLint lod, GLvoid *img);

typedef void (* glActiveTextureProcPtr) (GLenum texture);
typedef void (* glClientActiveTextureProcPtr) (GLenum texture);
typedef void (* glMultiTexCoord1dProcPtr) (GLenum target, GLdouble s);
typedef void (* glMultiTexCoord1dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord1fProcPtr) (GLenum target, GLfloat s);
typedef void (* glMultiTexCoord1fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord1iProcPtr) (GLenum target, GLint s);
typedef void (* glMultiTexCoord1ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord1sProcPtr) (GLenum target, GLshort s);
typedef void (* glMultiTexCoord1svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord2dProcPtr) (GLenum target, GLdouble s, GLdouble t);
typedef void (* glMultiTexCoord2dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord2fProcPtr) (GLenum target, GLfloat s, GLfloat t);
typedef void (* glMultiTexCoord2fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord2iProcPtr) (GLenum target, GLint s, GLint t);
typedef void (* glMultiTexCoord2ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord2sProcPtr) (GLenum target, GLshort s, GLshort t);
typedef void (* glMultiTexCoord2svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord3dProcPtr) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (* glMultiTexCoord3dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord3fProcPtr) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (* glMultiTexCoord3fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord3iProcPtr) (GLenum target, GLint s, GLint t, GLint r);
typedef void (* glMultiTexCoord3ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord3sProcPtr) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (* glMultiTexCoord3svProcPtr) (GLenum target, const GLshort *v);
typedef void (* glMultiTexCoord4dProcPtr) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (* glMultiTexCoord4dvProcPtr) (GLenum target, const GLdouble *v);
typedef void (* glMultiTexCoord4fProcPtr) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (* glMultiTexCoord4fvProcPtr) (GLenum target, const GLfloat *v);
typedef void (* glMultiTexCoord4iProcPtr) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (* glMultiTexCoord4ivProcPtr) (GLenum target, const GLint *v);
typedef void (* glMultiTexCoord4sProcPtr) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (* glMultiTexCoord4svProcPtr) (GLenum target, const GLshort *v);

typedef void (* glFogCoordfProcPtr) (GLfloat coord);
typedef void (* glFogCoordfvProcPtr) (const GLfloat *coord);  
typedef void (* glFogCoorddProcPtr) (GLdouble coord);
typedef void (* glFogCoorddvProcPtr) (const GLdouble *coord);   
typedef void (* glFogCoordPointerProcPtr) (GLenum type, GLsizei stride, const GLvoid *pointer);

typedef void (* glSecondaryColor3bProcPtr) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (* glSecondaryColor3bvProcPtr) (const GLbyte *v);
typedef void (* glSecondaryColor3dProcPtr) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (* glSecondaryColor3dvProcPtr) (const GLdouble *v);
typedef void (* glSecondaryColor3fProcPtr) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (* glSecondaryColor3fvProcPtr) (const GLfloat *v);
typedef void (* glSecondaryColor3iProcPtr) (GLint red, GLint green, GLint blue);
typedef void (* glSecondaryColor3ivProcPtr) (const GLint *v);
typedef void (* glSecondaryColor3sProcPtr) (GLshort red, GLshort green, GLshort blue);
typedef void (* glSecondaryColor3svProcPtr) (const GLshort *v);
typedef void (* glSecondaryColor3ubProcPtr) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (* glSecondaryColor3ubvProcPtr) (const GLubyte *v);
typedef void (* glSecondaryColor3uiProcPtr) (GLuint red, GLuint green, GLuint blue);
typedef void (* glSecondaryColor3uivProcPtr) (const GLuint *v);
typedef void (* glSecondaryColor3usProcPtr) (GLushort red, GLushort green, GLushort blue);
typedef void (* glSecondaryColor3usvProcPtr) (const GLushort *v);
typedef void (* glSecondaryColorPointerProcPtr) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

typedef void (* glPointParameterfProcPtr) (GLenum pname, GLfloat param); 
typedef void (* glPointParameterfvProcPtr) (GLenum pname, const GLfloat *params);
typedef void (* glPointParameteriProcPtr) (GLenum pname, GLint param); 
typedef void (* glPointParameterivProcPtr) (GLenum pname, const GLint *params);

typedef void (* glBlendFuncSeparateProcPtr) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

typedef void (* glMultiDrawArraysProcPtr) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
typedef void (* glMultiDrawElementsProcPtr) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount);

typedef void (* glWindowPos2dProcPtr) (GLdouble x, GLdouble y);
typedef void (* glWindowPos2dvProcPtr) (const GLdouble *v);
typedef void (* glWindowPos2fProcPtr) (GLfloat x, GLfloat y);
typedef void (* glWindowPos2fvProcPtr) (const GLfloat *v);
typedef void (* glWindowPos2iProcPtr) (GLint x, GLint y); 
typedef void (* glWindowPos2ivProcPtr) (const GLint *v);
typedef void (* glWindowPos2sProcPtr) (GLshort x, GLshort y);
typedef void (* glWindowPos2svProcPtr) (const GLshort *v);
typedef void (* glWindowPos3dProcPtr) (GLdouble x, GLdouble y, GLdouble z);
typedef void (* glWindowPos3dvProcPtr) (const GLdouble *v);
typedef void (* glWindowPos3fProcPtr) (GLfloat x, GLfloat y, GLfloat z);
typedef void (* glWindowPos3fvProcPtr) (const GLfloat *v);
typedef void (* glWindowPos3iProcPtr) (GLint x, GLint y, GLint z);
typedef void (* glWindowPos3ivProcPtr) (const GLint *v);
typedef void (* glWindowPos3sProcPtr) (GLshort x, GLshort y, GLshort z);
typedef void (* glWindowPos3svProcPtr) (const GLshort *v);

typedef void (* glGenQueriesProcPtr) (GLsizei n, GLuint *ids);
typedef void (* glDeleteQueriesProcPtr) (GLsizei n, const GLuint *ids);
typedef GLboolean (* glIsQueryProcPtr) (GLuint id);
typedef void (* glBeginQueryProcPtr) (GLenum target, GLuint id);
typedef void (* glEndQueryProcPtr) (GLenum target);
typedef void (* glGetQueryivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetQueryObjectivProcPtr) (GLuint id, GLenum pname, GLint *params);
typedef void (* glGetQueryObjectuivProcPtr) (GLuint id, GLenum pname, GLuint *params);

typedef void (* glBindBufferProcPtr) (GLenum target, GLuint buffer);
typedef void (* glDeleteBuffersProcPtr) (GLsizei n, const GLuint *buffers);
typedef void (* glGenBuffersProcPtr) (GLsizei n, GLuint *buffers);
typedef GLboolean (* glIsBufferProcPtr) (GLuint buffer);
typedef void (* glBufferDataProcPtr) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (* glBufferSubDataProcPtr) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef void (* glGetBufferSubDataProcPtr) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
typedef GLvoid *(* glMapBufferProcPtr) (GLenum target, GLenum access);
typedef GLboolean (* glUnmapBufferProcPtr) (GLenum target);
typedef void (* glGetBufferParameterivProcPtr) (GLenum target, GLenum pname, GLint *params);
typedef void (* glGetBufferPointervProcPtr) (GLenum target, GLenum pname, GLvoid **params);

typedef void (* glDrawBuffersProcPtr) (GLsizei n, const GLenum *bufs);
typedef void (* glVertexAttrib1dProcPtr) (GLuint index, GLdouble x);
typedef void (* glVertexAttrib1dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib1fProcPtr) (GLuint index, GLfloat x);
typedef void (* glVertexAttrib1fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib1sProcPtr) (GLuint index, GLshort x);
typedef void (* glVertexAttrib1svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib2dProcPtr) (GLuint index, GLdouble x, GLdouble y);
typedef void (* glVertexAttrib2dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib2fProcPtr) (GLuint index, GLfloat x, GLfloat y);
typedef void (* glVertexAttrib2fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib2sProcPtr) (GLuint index, GLshort x, GLshort y);
typedef void (* glVertexAttrib2svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib3dProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (* glVertexAttrib3dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib3fProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (* glVertexAttrib3fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib3sProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (* glVertexAttrib3svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NbvProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4NivProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4NsvProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4NubProcPtr) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (* glVertexAttrib4NubvProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4NuivProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4NusvProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttrib4bvProcPtr) (GLuint index, const GLbyte *v);
typedef void (* glVertexAttrib4dProcPtr) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (* glVertexAttrib4dvProcPtr) (GLuint index, const GLdouble *v);
typedef void (* glVertexAttrib4fProcPtr) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (* glVertexAttrib4fvProcPtr) (GLuint index, const GLfloat *v);
typedef void (* glVertexAttrib4ivProcPtr) (GLuint index, const GLint *v);
typedef void (* glVertexAttrib4sProcPtr) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (* glVertexAttrib4svProcPtr) (GLuint index, const GLshort *v);
typedef void (* glVertexAttrib4ubvProcPtr) (GLuint index, const GLubyte *v);
typedef void (* glVertexAttrib4uivProcPtr) (GLuint index, const GLuint *v);
typedef void (* glVertexAttrib4usvProcPtr) (GLuint index, const GLushort *v);
typedef void (* glVertexAttribPointerProcPtr) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (* glEnableVertexAttribArrayProcPtr) (GLuint index);
typedef void (* glDisableVertexAttribArrayProcPtr) (GLuint index);
typedef void (* glGetVertexAttribdvProcPtr) (GLuint index, GLenum pname, GLdouble *params);
typedef void (* glGetVertexAttribfvProcPtr) (GLuint index, GLenum pname, GLfloat *params);
typedef void (* glGetVertexAttribivProcPtr) (GLuint index, GLenum pname, GLint *params);
typedef void (* glGetVertexAttribPointervProcPtr) (GLuint index, GLenum pname, GLvoid **pointer);
typedef void (* glDeleteShaderProcPtr) (GLuint shader);
typedef void (* glDetachShaderProcPtr) (GLuint program, GLuint shader);
typedef GLuint (* glCreateShaderProcPtr) (GLenum type);
typedef void (* glShaderSourceProcPtr) (GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (* glCompileShaderProcPtr) (GLuint shader);
typedef GLuint (* glCreateProgramProcPtr) (void);
typedef void (* glAttachShaderProcPtr) (GLuint program, GLuint shader);
typedef void (* glLinkProgramProcPtr) (GLuint program);
typedef void (* glUseProgramProcPtr) (GLuint program);
typedef void (* glDeleteProgramProcPtr) (GLuint program);
typedef void (* glValidateProgramProcPtr) (GLuint program);
typedef void (* glUniform1fProcPtr) (GLint location, GLfloat v0);
typedef void (* glUniform2fProcPtr) (GLint location, GLfloat v0, GLfloat v1);
typedef void (* glUniform3fProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (* glUniform4fProcPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (* glUniform1iProcPtr) (GLint location, GLint v0);
typedef void (* glUniform2iProcPtr) (GLint location, GLint v0, GLint v1);
typedef void (* glUniform3iProcPtr) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (* glUniform4iProcPtr) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (* glUniform1fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform2fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform3fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform4fvProcPtr) (GLint location, GLsizei count, const GLfloat *value);
typedef void (* glUniform1ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform2ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform3ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniform4ivProcPtr) (GLint location, GLsizei count, const GLint *value);
typedef void (* glUniformMatrix2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef GLboolean (* glIsShaderProcPtr) (GLuint shader);
typedef GLboolean (* glIsProgramProcPtr) (GLuint program);
typedef void (* glGetShaderivProcPtr) (GLuint shader, GLenum pname, GLint *params);
typedef void (* glGetProgramivProcPtr) (GLuint program, GLenum pname, GLint *params);
typedef void (* glGetAttachedShadersProcPtr) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef void (* glGetShaderInfoLogProcPtr) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (* glGetProgramInfoLogProcPtr) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (* glGetUniformLocationProcPtr) (GLuint program, const GLchar *name);
typedef void (* glGetActiveUniformProcPtr) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (* glGetUniformfvProcPtr) (GLuint program, GLint location, GLfloat *params);
typedef void (* glGetUniformivProcPtr) (GLuint program, GLint location, GLint *params);
typedef void (* glGetShaderSourceProcPtr) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef void (* glBindAttribLocationProcPtr) (GLuint program, GLuint index, const GLchar *name);
typedef void (* glGetActiveAttribProcPtr) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef GLint (* glGetAttribLocationProcPtr) (GLuint program, const GLchar *name);
typedef void (* glStencilFuncSeparateProcPtr) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (* glStencilOpSeparateProcPtr) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
typedef void (* glStencilMaskSeparateProcPtr) (GLenum face, GLuint mask);

typedef void (* glUniformMatrix2x3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3x2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix2x4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4x2fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix3x4fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (* glUniformMatrix4x3fvProcPtr) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);


extern glGetProgramInfoLogProcPtr glGetProgramInfoLogPtr;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRangePtr;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRangePtr;
extern glGenBuffersProcPtr glGenBuffersPtr;
extern glBindBufferProcPtr	glBindBufferPtr;
extern glBufferDataProcPtr	glBufferDataPtr;
extern glBufferSubDataProcPtr glBufferSubDataPtr;
extern glVertexAttribPointerProcPtr glVertexAttribPointerPtr;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatusPtr;

extern PFNGLGENRENDERBUFFERSPROC glGenFramebuffersPtr;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteFramebuffersPtr;
extern PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1DPtr;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3DPtr;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbufferPtr;
extern glDrawBuffersProcPtr glDrawBuffersPtr;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPtr;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstancedPtr;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstancedPtr;

extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffersPtr;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffersPtr;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbufferPtr;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStoragePtr;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameterivPtr;

extern glEnableVertexAttribArrayProcPtr glEnableVertexAttribArrayPtr;
extern glDisableVertexAttribArrayProcPtr glDisableVertexAttribArrayPtr;
extern glBlendEquationSeparateProcPtr glBlendEquationSeparatePtr;
extern glBlendFuncSeparateProcPtr glBlendFuncSeparatePtr;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmapPtr;

extern glUnmapBufferProcPtr glUnmapBufferPtr;
extern glMapBufferProcPtr glMapBufferPtr;
extern glGetBufferParameterivProcPtr glGetBufferParameterivPtr;
extern glDeleteBuffersProcPtr glDeleteBuffersPtr;

extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocationPtr;
extern glCreateProgramProcPtr glCreateProgramPtr;
extern glGetActiveUniformProcPtr glGetActiveUniformPtr;
extern glGetProgramivProcPtr glGetProgramivPtr;
extern glGetUniformLocationProcPtr glGetUniformLocationPtr;
extern glLinkProgramProcPtr glLinkProgramPtr;
extern glUseProgramProcPtr glUseProgramPtr;
extern glAttachShaderProcPtr glAttachShaderPtr;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndexPtr;
extern glDetachShaderProcPtr glDetachShaderPtr;
extern glDeleteShaderProcPtr glDeleteShaderPtr;

extern glCompileShaderProcPtr glCompileShaderPtr;
extern glCreateShaderProcPtr glCreateShaderPtr;
extern glGetShaderInfoLogProcPtr glGetShaderInfoLogPtr;
extern glGetShaderivProcPtr glGetShaderivPtr;
extern glShaderSourceProcPtr glShaderSourcePtr;
extern glDeleteProgramProcPtr glDeleteProgramPtr;

extern PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr;
extern glBindAttribLocationProcPtr glBindAttribLocationPtr;
extern glGetActiveAttribProcPtr glGetActiveAttribPtr;
extern glGetAttribLocationProcPtr glGetAttribLocationPtr;
extern glActiveTextureProcPtr glActiveTexturePtr;

extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockNamePtr;
extern PFNGLBINDBUFFERRANGEPROC glBindBufferRangePtr;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBindingPtr;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBasePtr;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockivPtr;
extern glUniformMatrix4fvProcPtr glUniformMatrix4fvPtr;
extern glUniform1iProcPtr glUniform1iPtr;
extern glUniform1ivProcPtr glUniform1ivPtr;
extern glUniform1fProcPtr glUniform1fPtr;
extern glUniform1fvProcPtr glUniform1fvPtr;
extern glUniform2ivProcPtr glUniform2ivPtr;
extern glUniform2iProcPtr glUniform2iPtr;
extern glUniform2fvProcPtr glUniform2fvPtr;
extern glUniform2fProcPtr glUniform2fPtr;
extern glUniform3ivProcPtr glUniform3ivPtr;
extern glUniform3iProcPtr glUniform3iPtr;
extern glUniform3fvProcPtr glUniform3fvPtr;
extern glUniform3fProcPtr glUniform3fPtr;
extern glUniform4ivProcPtr glUniform4ivPtr;
extern glUniform4iProcPtr glUniform4iPtr;
extern glUniform4fvProcPtr glUniform4fvPtr;
extern glUniform4fProcPtr glUniform4fPtr;

extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisorPtr;

#endif
#endif
