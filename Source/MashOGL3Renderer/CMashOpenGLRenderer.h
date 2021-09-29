//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_3_RENDERER_H_
#define _C_MASH_OPENGL_3_RENDERER_H_

#include "MashDataTypes.h"

#include "MashVideoIntermediate.h"
#include "CMashOpenGLHeader.h"
#include "MashTypes.h"

namespace mash
{
    class MashDevice;
    
    class CGLContextObj;
    
	class CMashOpenGLRenderer : public MashVideoIntermediate
	{
	private:
		struct sOpenGLState_p1e
		{
			void (CALLBACK *func)(GLenum cap);
			GLenum param;
		};

		struct sOpenGLState_p1b
		{
			void (CALLBACK *func)(GLboolean cap);
			GLboolean param;
		};

		struct sOpenGLState_p2e
		{
			void (CALLBACK *func)(GLenum ea, GLenum eb);
			GLenum paramB;
			GLenum paramA;
		};

		struct sOpenGLState_p2f
		{
			void (CALLBACK *func)(GLfloat fa, GLfloat fb);
			GLfloat paramA;
			GLfloat paramB;
		};

		struct sRasterizerStateData
		{
			/*
				depth bias:
				Offset = m * slopeScaledDepthBias + depthBias
				
				glPolygonOffset(slopeScaledDepthBias, depthBias)
			*/

			sRasteriserStates stateData;
			sOpenGLState_p2e fillMode;
			sOpenGLState_p1e cullMode;
			sOpenGLState_p1e depthTestingEnabled;
			sOpenGLState_p1b depthWritingEnabled;
			sOpenGLState_p1e depthComparison;
			sOpenGLState_p2f depthBias;
			sOpenGLState_p1e multisample;
		};

		struct sBlendStateData
		{
			sBlendStates stateData;
			sOpenGLState_p1e blendEnable;
			GLenum srcBlend;
			GLenum destBlend;
			GLenum srcBlendAlpha;
			GLenum destBlendAlpha;
			GLenum blendOp;
			GLenum blendOpAlpha;

			GLboolean mr, mg, mb, ma;
		};
	private:
#ifdef MASH_WINDOWS
		HWND m_hWnd;
		HDC m_hdc;
		HGLRC m_hrc;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        MashDevice *m_device;
#endif

		eMASH_OPENGL_VERSION m_oglVersion;

		mash::MashVector2 m_backbufferSize;
		MashArray<sRasterizerStateData> m_rasterizerStates;
		MashArray<sBlendStateData> m_blendStates;
		MashArray<MashTextureState*> m_samplerStates;
        MashMaterial *m_writeSharedDepthMaterial;

		/*
			Unfortunatly we cant control the default targets like in DX.
			So we need to create another 'default' buffer and use it.

			All render surfaces that are created without a depth buffer will
			have this buffer set automatically.
		*/
        MashRenderSurface *m_defaultRenderTarget;

		MashTexture* LoadTextureFromFile(const MashStringc &fileName);
        bool IsExtensionSupported(const char *extList, const char *extension);
	public:
		CMashOpenGLRenderer();
		~CMashOpenGLRenderer();

		eMASH_STATUS SaveScreenShotToFile(eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const;
		eMASH_STATUS SaveTextureToFile(const MashTexture *texture, eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const;

		eMASH_STATUS OnResolutionChange(uint32 width, uint32 height);
		eMASH_STATUS _Initialise(MashDevice *device, const sMashDeviceSettings &creationParameters, void *extraData);

		eMASH_STATUS SetRasteriserState(int32 rasterizerState);
		eMASH_STATUS SetBlendState(int32 blendState);
		int32 AddRasteriserState(const sRasteriserStates &state);
		int32 AddBlendState(const sBlendStates &state);
		MashTextureState* AddSamplerState(const sSamplerState &state);

		const sBlendStates* GetBlendState(int32 iIndex)const;
		const sRasteriserStates* GetRasterizerState(int32 iIndex)const;

		void SetFillColour(const sMashColour4 &fillColour);

		eMASH_STATUS BeginRender();
		eMASH_STATUS EndRender();

		eMASH_STATUS SetScreenResolution(bool fullscreen, uint32 width, uint32 height);
		eMASH_STATUS SetViewport(const sMashViewPort &viewport);

		//TODO : This should be an empty function because the mesh buffer takes care of it.
		eMASH_STATUS SetVertexFormat(MashVertex *pVertex);

		MashVertex* _CreateVertexType(MashMaterial *material,
			const sMashVertexElement *vertexDecl,
			uint32 iDeclCount);

		eMASH_STATUS SetRenderTargetDefault();

		
		/*
			TODO : Add a parameter for init data
		*/
		MashTexture* AddTexture(const MashStringc &sName,
											uint32 iWidth, 
											uint32 iHeight, 
											bool useMipmaps, 
											eUSAGE usage, 
											eFORMAT format);

		MashTexture* AddCubeTexture(const MashStringc &sName,
											uint32 iSize,
											bool useMipmaps, 
											eUSAGE usage, 
											eFORMAT format);

        MashMeshBuffer* _CreateMeshBuffer();
        
		MashMeshBuffer* CreateMeshBuffer(const sVertexStreamInit *initVertexStreamData,
			uint32 initVertexStreamCount,
            const MashVertex *vertexDecl, 
			const void *indexData = 0, 
			uint32 indexCount = 0, 
			eFORMAT indexFormat = aFORMAT_R16_UINT,
			eUSAGE indexUsage = aUSAGE_STATIC);


		MashVertexBuffer* CreateVertexBuffer(const void *pData, uint32 iVertexCount, eUSAGE usage);
		MashIndexBuffer* CreateIndexBuffer(const void *pData, uint32 iIndexCount, eUSAGE usage, eFORMAT format); 

		MashIndexBuffer* CreateIndexBufferBySize(const void *pData, uint32 iSizeInBytes, eUSAGE usage, eFORMAT format);

		MashRenderSurface* CreateRenderSurface(int32 iWidth, int32 iHeight, const eFORMAT *pFormats,
			uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOptions, eFORMAT eDepthFormat = aFORMAT_DEPTH32_FLOAT);

		MashRenderSurface* CreateCubicRenderSurface(uint32 iSize, bool useMipmaps,
			eFORMAT eTextureFormat, bool bUseDepth, eFORMAT eDepthFormat);

		eMASH_STATUS ClearTarget(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth = 1.0f);

		eMASH_STATUS DrawIndexedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 iIndexCount,
					uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType);

		eMASH_STATUS DrawVertexList(const MashMeshBuffer *buffers, uint32 iVertexCount,
			uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType);

		eMASH_STATUS DrawVertexInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount);

		eMASH_STATUS DrawIndexedInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 indexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount);

		eSHADER_API_TYPE GetCurrentAPI()const;

		eMASH_OPENGL_VERSION GetOGLVersion()const;
		const mash::MashVector2 GetBackBufferSize(bool returnActiveRenderSurfaceSize = false)const;
		uint32 GetOGLDefaultDepthBuffer();

	};

	inline eMASH_OPENGL_VERSION CMashOpenGLRenderer::GetOGLVersion()const
	{
		return m_oglVersion;
	}

	inline eSHADER_API_TYPE CMashOpenGLRenderer::GetCurrentAPI()const
	{
		return aSHADERAPITYPE_OPENGL;
	}

	_MASH_EXPORT MashVideo* CreateMashOpenGL3Device(const void*);
}

#endif
