//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_RENDERER_H_
#define _C_MASH_D3D10_RENDERER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashVideoIntermediate.h"
#ifndef __MINGW32__
#include <windows.h>
#endif
#include <d3d10_1.h>
#include "MashArray.h"
#include "MashTypes.h"
#include "MashCreationParameters.h"
#include <bitset>
namespace mash
{
	class MashSkin;

	const uint32 g_maxD3D10ShaderResourceViewIndex = 128;
	const uint32 g_maxD3D10ShaderSamplers = 16;

	class _MASH_EXPORT CMashD3D10Renderer : public MashVideoIntermediate
	{
	private:
		struct sRasterizerStateData
		{
			ID3D10RasterizerState* pD3D10RasterizerState;
			ID3D10DepthStencilState *pD3D10DepthStencilState;
			sRasteriserStates stateData;

			sRasterizerStateData(ID3D10RasterizerState* _pD3D10RasterizerState,
				ID3D10DepthStencilState *_pD3D10DepthStencilState,
				const sRasteriserStates &_stateData):pD3D10RasterizerState(_pD3D10RasterizerState),
				pD3D10DepthStencilState(_pD3D10DepthStencilState),
				stateData(_stateData){}
		};

		struct sBlendStateData
		{
			ID3D10BlendState1* pD3D10BlendState;
			sBlendStates stateData;

			sBlendStateData(ID3D10BlendState1* _pD3D10BlendState,
				const sBlendStates &_stateData):pD3D10BlendState(_pD3D10BlendState),
				stateData(_stateData){}
		};

	private:
		HWND m_hWnd;
		ID3D10Device1 *m_pDevice;
		IDXGISwapChain *m_pSwapChain;
		ID3D10RenderTargetView *m_pDefaultRenderTargetView;
		ID3D10DepthStencilView *m_pDefaultDepthStencilView;
		uint32 m_vsyncLevel;

		MashArray<sRasterizerStateData> m_rasterizerStates;
		MashArray<sBlendStateData> m_blendStates;
		MashArray<MashTextureState*> m_samplerStates;

		DXGI_SWAP_CHAIN_DESC m_swapChainDesc;
		D3D10_TEXTURE2D_DESC m_depthTextureDesc;
		D3D10_DEPTH_STENCIL_VIEW_DESC m_depthStencilViewDesc;

		eANTIALIAS_TYPE m_antialiasingType;

		uint32 m_instancedRenderBufferCount;
		uint32 *m_instancedRenderStrides;
		uint32 *m_instancedRenderOffsets;
		ID3D10Buffer **m_instancedRenderBuffers;

		/*
			These values are used to determine what resource slots need
			to be set back to NULL on certain operations.
		*/
		std::bitset<g_maxD3D10ShaderResourceViewIndex> m_currentVertexShaderResourceFlags;
		std::bitset<g_maxD3D10ShaderResourceViewIndex> m_currentPixelShaderResourceFlags;
		//shouldnt need to be larger than a uchar
		MashArray<uint8> m_currentVertexShaderResourceIndices;
		MashArray<uint8> m_currentPixelShaderResourceIndices;
		
		std::bitset<g_maxD3D10ShaderSamplers> m_currentVertexShaderSamplerFlags;
		std::bitset<g_maxD3D10ShaderSamplers> m_currentPixelShaderSamplerFlags;
		//shouldnt need to be larger than a uchar
		MashArray<uint8> m_currentVertexShaderSamplerIndices;
		MashArray<uint8> m_currentPixelShaderSamplerIndices;

		
		MashTexture* LoadTextureFromFile(const MashStringc &fileName);
		MashTexture* _CreateTexture(const MashStringc &sName, ID3D10Texture2D *pD3D10Texture, const D3D10_TEXTURE2D_DESC &desc);
		eMASH_STATUS _SaveTextureToFile(ID3D10Texture2D *d3dTexture, eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const;
	public:
		CMashD3D10Renderer();
		~CMashD3D10Renderer();

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

		eMASH_STATUS BeginRender();
		eMASH_STATUS EndRender();

		void SetD3D10VertexShaderResourceView(uint32 index, ID3D10ShaderResourceView *view);
		void SetD3D10PixelShaderResourceView(uint32 index, ID3D10ShaderResourceView *view);
		void SetD3D10PixelShaderSampler(uint32 index, const MashTextureState *state);
		void SetD3D10VertexShaderSampler(uint32 index, const MashTextureState *state);
		void ResetUsedShaderResources();
		void ResetUsedBuffers();

		const mash::MashVector2 GetBackBufferSize(bool returnActiveRenderSurfaceSize = false)const;

		//eMASH_STATUS EnableFullScreen(bool enable);
		eMASH_STATUS SetScreenResolution(bool fullscreen, uint32 width, uint32 height);

		//eMASH_STATUS SetVertexType(int32 vertexType);
		eMASH_STATUS SetVertexFormat(MashVertex *pVertex);

		MashMeshBuffer* _CreateMeshBuffer();
		MashVertex* _CreateVertexType(MashMaterial *material,
				const sMashVertexElement *vertexDecl,
				uint32 iDeclCount);

		eMASH_STATUS SetRenderTargetDefault();

		eMASH_STATUS SetViewport(const sMashViewPort &viewport);

		eMASH_STATUS ValidateTextureSize(const mash::MashVector2 &in, mash::MashVector2 &out);

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

		MashTexture* CreateCubeTexture(const MashStringc &sName, const D3D10_TEXTURE2D_DESC &desc);
		MashTexture* CreateTexture(const MashStringc &sName, const D3D10_TEXTURE2D_DESC &desc);
		MashVertexBuffer* CreateVertexBuffer(ID3D10Buffer *pVertexBuffer, const D3D10_BUFFER_DESC &desc);
		MashIndexBuffer* CreateIndexBuffer(ID3D10Buffer *pIndexBuffer, eFORMAT eFormat, const D3D10_BUFFER_DESC &desc);

		MashMeshBuffer* CreateMeshBuffer(const sVertexStreamInit *initVertexStreamData,
				uint32 initVertexStreamCount,
				const MashVertex *vertexDecl, 
				const void *indexData = 0, 
				uint32 indexCount = 0, 
				eFORMAT indexFormat = aFORMAT_R16_UINT,
				eUSAGE indexUsage = aUSAGE_STATIC);

		MashVertexBuffer* CreateVertexBuffer(const void *pData, uint32 iSizeInBytes, eUSAGE usage);
		MashIndexBuffer* CreateIndexBuffer(const void *pData, uint32 iIndexCount, eUSAGE usage, eFORMAT format); 
		MashIndexBuffer* CreateIndexBufferBySize(const void *pData, uint32 iSizeInBytes, eUSAGE usage, eFORMAT format);

		MashRenderSurface* CreateRenderSurface(int32 iWidth, int32 iHeight, const eFORMAT *pFormats,
			uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOption, eFORMAT eDepthFormat = aFORMAT_DEPTH32_FLOAT);

		MashRenderSurface* CreateCubicRenderSurface(uint32 iSize, bool useMipmaps,
			eFORMAT eTextureFormat, bool bUseDepth, eFORMAT eDepthFormat);

		eMASH_STATUS ClearTarget(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth = 1.0f);

		eMASH_STATUS DrawIndexedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 iIndexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType);

		eMASH_STATUS DrawVertexList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType);

		eMASH_STATUS DrawVertexInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount);

		eMASH_STATUS DrawIndexedInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 indexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount);

		eSHADER_API_TYPE GetCurrentAPI()const;

		ID3D10Device* GetD3D10Device()const;
		ID3D10DepthStencilView* GetD3D10DefaultDepthStencilView()const;
	};

	inline ID3D10DepthStencilView* CMashD3D10Renderer::GetD3D10DefaultDepthStencilView()const
	{
		return m_pDefaultDepthStencilView;
	}

	inline eSHADER_API_TYPE CMashD3D10Renderer::GetCurrentAPI()const
	{
		return aSHADERAPITYPE_D3D10;
	}

	inline ID3D10Device* CMashD3D10Renderer::GetD3D10Device()const
	{
		return m_pDevice;
	}
}

#endif

#endif
