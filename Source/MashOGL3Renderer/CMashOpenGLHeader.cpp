//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLHeader.h"

//#ifdef MASH_WINDOWS
#if defined (MASH_LINUX) || defined (MASH_WINDOWS)
PFNGLGETSTRINGIPROC glGetStringiPtr = 0;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockNamePtr = 0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLogPtr = 0;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRangePtr = 0;
PFNGLMAPBUFFERRANGEPROC glMapBufferRangePtr = 0;
PFNGLBINDBUFFERRANGEPROC glBindBufferRangePtr = 0;
PFNGLGENBUFFERSPROC glGenBuffersPtr = 0;
PFNGLBINDBUFFERPROC	glBindBufferPtr = 0;
PFNGLBUFFERDATAPROC	glBufferDataPtr = 0;
PFNGLBUFFERSUBDATAPROC glBufferSubDataPtr = 0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointerPtr = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatusPtr = 0;

PFNGLGENRENDERBUFFERSPROC glGenFramebuffersPtr = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteFramebuffersPtr = 0;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1DPtr = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr = 0;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3DPtr = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbufferPtr = 0;
PFNGLDRAWBUFFERSPROC glDrawBuffersPtr = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPtr = 0;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstancedPtr = 0;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstancedPtr = 0;

PFNGLGENRENDERBUFFERSPROC glGenRenderbuffersPtr = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffersPtr = 0;
PFNGLBINDRENDERBUFFERPROC glBindRenderbufferPtr = 0;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStoragePtr = 0;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameterivPtr = 0;

PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArrayPtr = 0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArrayPtr = 0;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparatePtr = 0;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparatePtr = 0;
PFNGLGENERATEMIPMAPPROC glGenerateMipmapPtr = 0;

PFNGLUNMAPBUFFERPROC glUnmapBufferPtr = 0;
PFNGLMAPBUFFERPROC glMapBufferPtr = 0;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameterivPtr = 0;
PFNGLDELETEBUFFERSPROC glDeleteBuffersPtr = 0;

PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocationPtr = 0;
PFNGLCREATEPROGRAMPROC glCreateProgramPtr = 0;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniformPtr = 0;
PFNGLGETPROGRAMIVPROC glGetProgramivPtr = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocationPtr = 0;
PFNGLLINKPROGRAMPROC glLinkProgramPtr = 0;
PFNGLUSEPROGRAMPROC glUseProgramPtr = 0;
PFNGLATTACHSHADERPROC glAttachShaderPtr = 0;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndexPtr = 0;
PFNGLDETACHSHADERPROC glDetachShaderPtr = 0;
PFNGLDELETESHADERPROC glDeleteShaderPtr = 0;

PFNGLCOMPILESHADERPROC glCompileShaderPtr = 0;
PFNGLCREATESHADERPROC glCreateShaderPtr = 0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLogPtr = 0;
PFNGLGETSHADERIVPROC glGetShaderivPtr = 0;
PFNGLSHADERSOURCEPROC glShaderSourcePtr = 0;
PFNGLDELETEPROGRAMPROC glDeleteProgramPtr = 0;

PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr = 0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocationPtr = 0;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttribPtr = 0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocationPtr = 0;
PFNGLACTIVETEXTUREPROC glActiveTexturePtr = 0;

PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBindingPtr = 0;
PFNGLBINDBUFFERBASEPROC glBindBufferBasePtr = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockivPtr = 0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fvPtr = 0;
PFNGLUNIFORM1IPROC glUniform1iPtr = 0;
PFNGLUNIFORM1IVPROC glUniform1ivPtr = 0;
PFNGLUNIFORM1FPROC glUniform1fPtr = 0;
PFNGLUNIFORM1FVPROC glUniform1fvPtr = 0;
PFNGLUNIFORM2IVPROC glUniform2ivPtr = 0;
PFNGLUNIFORM2IPROC glUniform2iPtr = 0;
PFNGLUNIFORM2FVPROC glUniform2fvPtr = 0;
PFNGLUNIFORM2FPROC glUniform2fPtr = 0;
PFNGLUNIFORM3IVPROC glUniform3ivPtr = 0;
PFNGLUNIFORM3IPROC glUniform3iPtr = 0;
PFNGLUNIFORM3FVPROC glUniform3fvPtr = 0;
PFNGLUNIFORM3FPROC glUniform3fPtr = 0;
PFNGLUNIFORM4IVPROC glUniform4ivPtr = 0;
PFNGLUNIFORM4IPROC glUniform4iPtr = 0;
PFNGLUNIFORM4FVPROC glUniform4fvPtr = 0;
PFNGLUNIFORM4FPROC glUniform4fPtr = 0;

PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisorPtr = 0;
#elif defined (MASH_APPLE)
glGetProgramInfoLogProcPtr glGetProgramInfoLogPtr = 0;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRangePtr = 0;
PFNGLMAPBUFFERRANGEPROC glMapBufferRangePtr = 0;
glGenBuffersProcPtr glGenBuffersPtr = 0;
glBindBufferProcPtr	glBindBufferPtr = 0;
glBufferDataProcPtr	glBufferDataPtr = 0;
glBufferSubDataProcPtr glBufferSubDataPtr = 0;
glVertexAttribPointerProcPtr glVertexAttribPointerPtr = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatusPtr = 0;
PFNGLGENRENDERBUFFERSPROC glGenFramebuffersPtr = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteFramebuffersPtr = 0;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1DPtr = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr = 0;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3DPtr = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbufferPtr = 0;
glDrawBuffersProcPtr glDrawBuffersPtr = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPtr = 0;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstancedPtr = 0;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstancedPtr = 0;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffersPtr = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffersPtr = 0;
PFNGLBINDRENDERBUFFERPROC glBindRenderbufferPtr = 0;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStoragePtr = 0;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameterivPtr = 0;
glEnableVertexAttribArrayProcPtr glEnableVertexAttribArrayPtr = 0;
glDisableVertexAttribArrayProcPtr glDisableVertexAttribArrayPtr = 0;
glBlendEquationSeparateProcPtr glBlendEquationSeparatePtr = 0;
glBlendFuncSeparateProcPtr glBlendFuncSeparatePtr = 0;
PFNGLGENERATEMIPMAPPROC glGenerateMipmapPtr = 0;
glUnmapBufferProcPtr glUnmapBufferPtr = 0;
glMapBufferProcPtr glMapBufferPtr = 0;
glGetBufferParameterivProcPtr glGetBufferParameterivPtr = 0;
glDeleteBuffersProcPtr glDeleteBuffersPtr = 0;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocationPtr = 0;
glCreateProgramProcPtr glCreateProgramPtr = 0;
glGetActiveUniformProcPtr glGetActiveUniformPtr = 0;
glGetProgramivProcPtr glGetProgramivPtr = 0;
glGetUniformLocationProcPtr glGetUniformLocationPtr = 0;
glLinkProgramProcPtr glLinkProgramPtr = 0;
glUseProgramProcPtr glUseProgramPtr = 0;
glAttachShaderProcPtr glAttachShaderPtr = 0;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndexPtr = 0;
glDetachShaderProcPtr glDetachShaderPtr = 0;
glDeleteShaderProcPtr glDeleteShaderPtr = 0;
glCompileShaderProcPtr glCompileShaderPtr = 0;
glCreateShaderProcPtr glCreateShaderPtr = 0;
glGetShaderInfoLogProcPtr glGetShaderInfoLogPtr = 0;
glGetShaderivProcPtr glGetShaderivPtr = 0;
glShaderSourceProcPtr glShaderSourcePtr = 0;
glDeleteProgramProcPtr glDeleteProgramPtr = 0;
PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr = 0;
glBindAttribLocationProcPtr glBindAttribLocationPtr = 0;
glGetActiveAttribProcPtr glGetActiveAttribPtr = 0;
glGetAttribLocationProcPtr glGetAttribLocationPtr = 0;
glActiveTextureProcPtr glActiveTexturePtr = 0;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockNamePtr = 0;
PFNGLBINDBUFFERRANGEPROC glBindBufferRangePtr = 0;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBindingPtr = 0;
PFNGLBINDBUFFERBASEPROC glBindBufferBasePtr = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockivPtr = 0;
glUniformMatrix4fvProcPtr glUniformMatrix4fvPtr = 0;
glUniform1iProcPtr glUniform1iPtr = 0;
glUniform1ivProcPtr glUniform1ivPtr = 0;
glUniform1fProcPtr glUniform1fPtr = 0;
glUniform1fvProcPtr glUniform1fvPtr = 0;
glUniform2ivProcPtr glUniform2ivPtr = 0;
glUniform2iProcPtr glUniform2iPtr = 0;
glUniform2fvProcPtr glUniform2fvPtr = 0;
glUniform2fProcPtr glUniform2fPtr = 0;
glUniform3ivProcPtr glUniform3ivPtr = 0;
glUniform3iProcPtr glUniform3iPtr = 0;
glUniform3fvProcPtr glUniform3fvPtr = 0;
glUniform3fProcPtr glUniform3fPtr = 0;
glUniform4ivProcPtr glUniform4ivPtr = 0;
glUniform4iProcPtr glUniform4iPtr = 0;
glUniform4fvProcPtr glUniform4fvPtr = 0;
glUniform4fProcPtr glUniform4fPtr = 0;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisorPtr = 0;

#endif

