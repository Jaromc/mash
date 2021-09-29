//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashCompileSettings.h"

#ifdef MASH_WINDOWS
#include "CMashD3D10Renderer.h"
#include "MashLog.h"
#include "MashSkin.h"
#include "CMashD3D10Helper.h"
#include "CMashD3D10RenderSurface.h"
#include "CMashD3D10Texture.h"
#include "CMashD3D10CubeTexture.h"
#include "CMashD3D10RenderSurface.h"
#include "CMashD3D10CubicRenderSurface.h"
#include "CMashD3D10VertexBuffer.h"
#include "CMashD3D10IndexBuffer.h"
#include "CMashD3D10Vertex.h"
#include "CMashD3D10Effect.h"
#include "CMashD3D10SkinManager.h"
#include "CMashD3D10TextureState.h"
#include "CMashD3D10MeshBuffer.h"
#include "CMashD3D10EffectProgram.h"
#include "MashMaterialDependentResource.h"
#include "MashMaterial.h"
#include "MashFileStream.h"
#include "MashTechnique.h"
#include "MashHelper.h"
#include "MashDevice.h"
#include "DXGI.h"

namespace mash
{
	const uint32 MAX_RENDER_TARGET_COUNT = 8;

	MashVideo* CreateMashD3D10Device()
	{
		CMashD3D10Renderer *pNewRenderer = MASH_NEW_COMMON CMashD3D10Renderer();
		return pNewRenderer;
	}


	CMashD3D10Renderer::CMashD3D10Renderer():MashVideoIntermediate(), m_hWnd(0), m_instancedRenderBufferCount(0), 
		m_instancedRenderStrides(0), m_instancedRenderOffsets(0), m_instancedRenderBuffers(0)
	{
		for(uint32 i = 0; i < m_currentVertexShaderResourceFlags.size(); ++i)
			m_currentVertexShaderResourceFlags.set(i, false);

		for(uint32 i = 0; i < m_currentPixelShaderResourceFlags.size(); ++i)
			m_currentPixelShaderResourceFlags.set(i, false);
	}

	CMashD3D10Renderer::~CMashD3D10Renderer()
	{
		D3DX10UnsetAllDeviceObjects(m_pDevice);

		if (m_pDefaultRenderTargetView)
		{
			m_pDefaultRenderTargetView->Release();
			m_pDefaultRenderTargetView = 0;
		}

		if (m_pDefaultDepthStencilView)
		{
			m_pDefaultDepthStencilView->Release();
			m_pDefaultDepthStencilView = 0;
		}

		MashArray<MashTextureState*>::Iterator samplerIter = m_samplerStates.Begin();
		MashArray<MashTextureState*>::Iterator samplerIterEnd = m_samplerStates.End();
		for(; samplerIter != samplerIterEnd; ++samplerIter)
		{
			if (*samplerIter)
				(*samplerIter)->Drop();
		}

		m_samplerStates.Clear();

		if (m_instancedRenderStrides)
		{
			MASH_FREE(m_instancedRenderStrides);
			m_instancedRenderStrides = 0;
		}

		if (m_instancedRenderOffsets)
		{
			MASH_FREE(m_instancedRenderOffsets);
			m_instancedRenderOffsets = 0;
		}

		for(uint32 i = 0; i < m_instancedRenderBufferCount; ++i)
		{
			if (m_instancedRenderBuffers[i])
				m_instancedRenderBuffers[i]->Release();
		}

		if ((m_instancedRenderBufferCount > 0) && m_instancedRenderBuffers)
		{
			MASH_FREE(m_instancedRenderBuffers);
			m_instancedRenderBuffers = 0;
		}

		if(m_pSwapChain)
		{
			m_pSwapChain->SetFullscreenState(false, NULL);
			m_pSwapChain->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
			m_pDevice = 0;
		}
	}

	MashMeshBuffer* CMashD3D10Renderer::_CreateMeshBuffer()
    {
        return MASH_NEW_COMMON CMashD3D10MeshBuffer(this);
    }

	eMASH_STATUS CMashD3D10Renderer::_Initialise(MashDevice *device, const sMashDeviceSettings &creationParameters, void *extraData)
	{
		if (!extraData)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                              "HWND missing.", 
                              "CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		m_hWnd = (HWND)extraData;

		MashFileManager *pFileManager = device->GetFileManager();

		DXGI_FORMAT screenFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		if (creationParameters.backbufferFormat == aBACKBUFFER_FORMAT_16BIT)
			screenFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

		/*
			old code for vsync. This code can be useful
			if we let the user select the hz as well?
		*/
		uint32 numerator = 60;
		uint32 denominator = 1;
		if (creationParameters.enableVSync)
		{
			bool refreshRateFound = false;
			IDXGIFactory1 *factory = 0;
			if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								  "Failed to enumerate device adapters.", 
								  "CMashD3D10Renderer::_Initialise");

				return aMASH_FAILED;
			}

			IDXGIAdapter *adapter = 0;
			//get the main adapter
			factory->EnumAdapters(0, &adapter);
			IDXGIOutput *mainMoniter = 0;
			//get the main moniter
			if (FAILED(adapter->EnumOutputs(0, &mainMoniter)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								  "Failed to enumerate device adapters.", 
								  "CMashD3D10Renderer::_Initialise");

				return aMASH_FAILED;
			}

			uint32 modeCount = 0;
			if (FAILED(mainMoniter->GetDisplayModeList(screenFormat, DXGI_ENUM_MODES_INTERLACED, &modeCount, 0)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								  "Failed to enumerate device adapters.", 
								  "CMashD3D10Renderer::_Initialise");

				return aMASH_FAILED;
			}

			DXGI_MODE_DESC *displayModeList = (DXGI_MODE_DESC*)MASH_ALLOC_COMMON(sizeof(DXGI_MODE_DESC) * modeCount);
			if (FAILED(mainMoniter->GetDisplayModeList(screenFormat, DXGI_ENUM_MODES_INTERLACED, &modeCount, displayModeList)))
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								  "Failed to enumerate device adapters.", 
								  "CMashD3D10Renderer::_Initialise");

				return aMASH_FAILED;
			}

			for(uint32 i = 0; i < modeCount; ++i)
			{
				if (displayModeList[i].Width == creationParameters.screenWidth &&
					displayModeList[i].Height == creationParameters.screenHeight)
				{
					numerator = displayModeList[i].RefreshRate.Numerator;
					denominator = displayModeList[i].RefreshRate.Denominator;
					refreshRateFound = true;
					//break;//dont break because we want the highest hz
				}
			}

			if (!refreshRateFound)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, 
								  "Failed to find a refresh rate to match the selected resolution.\
								  The refresh rate has been set to 60hz.",
								  "CMashD3D10Renderer::_Initialise");
			}

			MASH_FREE(displayModeList);

			if (factory)
				factory->Release();
		}

		

		sMashViewPort viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = creationParameters.screenWidth;
		viewport.height = creationParameters.screenHeight;
		viewport.minZ = 0.000f;
		viewport.maxZ = 1.0f;

		m_vsyncLevel = (creationParameters.enableVSync)?1:0;

		m_defaultRenderTargetViewport = viewport;

		ZeroMemory(&m_swapChainDesc, sizeof(m_swapChainDesc));
		ZeroMemory(&m_depthTextureDesc, sizeof(m_depthTextureDesc));
		ZeroMemory(&m_depthStencilViewDesc, sizeof(m_depthStencilViewDesc));

		m_swapChainDesc.BufferCount = 2;//double buffer
		m_swapChainDesc.BufferDesc.Width = creationParameters.screenWidth;
		m_swapChainDesc.BufferDesc.Height = creationParameters.screenHeight;
		m_swapChainDesc.BufferDesc.Format = screenFormat;

		if (creationParameters.enableVSync)
		{
			m_swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
			m_swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else
		{
			m_swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			m_swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
		m_swapChainDesc.OutputWindow = m_hWnd;
		m_swapChainDesc.SampleDesc.Quality = 0;
		m_swapChainDesc.Windowed = !creationParameters.fullScreen;
		m_swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		m_antialiasingType = creationParameters.antiAliasType;
			
		switch(creationParameters.antiAliasType)
		{
		case aANTIALIAS_TYPE_X2:
			m_swapChainDesc.SampleDesc.Count = 2;
			m_depthTextureDesc.SampleDesc.Count = 2;
			m_depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
			break;
		case aANTIALIAS_TYPE_X4:
			m_swapChainDesc.SampleDesc.Count = 4;
			m_depthTextureDesc.SampleDesc.Count = 4;
			m_depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
			break;
		case aANTIALIAS_TYPE_X8:
			m_swapChainDesc.SampleDesc.Count = 8;
			m_depthTextureDesc.SampleDesc.Count = 8;
			m_depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
			break;
		default:
			m_swapChainDesc.SampleDesc.Count = 1;
			m_depthTextureDesc.SampleDesc.Count = 1;
			m_depthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
		}

		m_swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		m_swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		UINT iCreateDeviceFlags = 0;
		#ifdef MASH_DEBUG
			iCreateDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
		#endif

		D3D10_FEATURE_LEVEL1 featureLevel = D3D10_FEATURE_LEVEL_10_1;
		if (FAILED(D3D10CreateDeviceAndSwapChain1(NULL,
			D3D10_DRIVER_TYPE_HARDWARE, //use hardware rendering
			NULL, //must be NULL for hardware rendering. Software rendering use only
			iCreateDeviceFlags, //creatation flags
			featureLevel,
			D3D10_1_SDK_VERSION, //SDK version
			&m_swapChainDesc, //swap chain description
			&m_pSwapChain, //swap cain
			&m_pDevice))) //d3d device
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 renderer.", 
					"CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		ID3D10Texture2D *pBackBuffer = 0;

		if (FAILED(m_pSwapChain->GetBuffer(0,
			__uuidof(ID3D10Texture2D),
			(LPVOID*)&pBackBuffer)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to get d3d10 back buffer.", 
					"CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		/*
			resource views are a resource that can be bound to the graphics
			pipeline at a specific stage. In this case, we are creating a
			render target view resource that will be used as the back buffer
		*/
		if (FAILED(m_pDevice->CreateRenderTargetView(pBackBuffer,
			NULL, 
			&m_pDefaultRenderTargetView)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default render target.", 
					"CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		pBackBuffer->Release();

		//create depth stencil texture
		m_depthTextureDesc.Width = creationParameters.screenWidth;
		m_depthTextureDesc.Height = creationParameters.screenHeight;
		m_depthTextureDesc.MipLevels = 1;
		m_depthTextureDesc.ArraySize = 1;
		m_depthTextureDesc.SampleDesc.Quality = 0;
		m_depthTextureDesc.Usage = D3D10_USAGE_DEFAULT;
		m_depthTextureDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		m_depthTextureDesc.CPUAccessFlags = 0;
		m_depthTextureDesc.MiscFlags = 0;

		if (creationParameters.backbufferFormat == aDEPTH_FORMAT_32BIT)
			m_depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		else
			m_depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		ID3D10Texture2D *pDepthStencil = 0;
		if (FAILED(m_pDevice->CreateTexture2D(&m_depthTextureDesc, NULL, &pDepthStencil)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default depth texture.", 
					"CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		m_depthStencilViewDesc.Format = m_depthTextureDesc.Format;
		m_depthStencilViewDesc.Texture2D.MipSlice = 0;
		if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencil, &m_depthStencilViewDesc, &m_pDefaultDepthStencilView)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default depth buffer.", 
					"CMashD3D10Renderer::_Initialise");

			return aMASH_FAILED;
		}

		/*
			now we have created both the render target and depth stencil buffer, 
			we can bind them both to the pipeline so they will be used
			at render time.
		*/
		m_pDevice->OMSetRenderTargets(1, //specify that we will only be using 1 render target
			&m_pDefaultRenderTargetView, m_pDefaultDepthStencilView);

		SetViewport(viewport);

		m_skinManager = MASH_NEW_COMMON CMashD3D10SkinManager(this);
		if (m_skinManager->_Initialise(creationParameters) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Skin manager failed to initialise.", "CMashD3D10Renderer::_Initialise");
			return aMASH_FAILED;
		}

		return MashVideoIntermediate::_InitialiseCommon(pFileManager);
	}

	eMASH_STATUS CMashD3D10Renderer::ValidateTextureSize(const mash::MashVector2 &in, mash::MashVector2 &out)
	{
		out = in;
		return aMASH_OK;
	}

	const sBlendStates* CMashD3D10Renderer::GetBlendState(int32 iIndex)const
	{
		if (iIndex < 0 || iIndex >= m_blendStates.Size())
			iIndex = m_defaultBlendState;

		return &m_blendStates[iIndex].stateData;
	}

	const sRasteriserStates* CMashD3D10Renderer::GetRasterizerState(int32 iIndex)const
	{
		if (iIndex < 0 || iIndex >= m_rasterizerStates.Size())
			iIndex = m_defaultRasterizerState;

		return &m_rasterizerStates[iIndex].stateData;
	}

	int32 CMashD3D10Renderer::AddRasteriserState(const sRasteriserStates &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_rasterizerStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (m_rasterizerStates[i].stateData == state)
				return i;
		}

		D3D10_RASTERIZER_DESC rasterizerState;

		rasterizerState.CullMode = MashToD3D10CullMode(state.cullMode);
		rasterizerState.FillMode = MashToD3D10FillMode(state.fillMode);
		rasterizerState.FrontCounterClockwise = false;//CW vertex order means front face
		rasterizerState.DepthBias = state.depthBias;
		rasterizerState.DepthBiasClamp = state.depthBiasClamp;
		rasterizerState.SlopeScaledDepthBias = state.slopeScaledDepthBias;
		rasterizerState.DepthClipEnable = true; //always use depth clipping
		rasterizerState.ScissorEnable = false; //never use scissor testing
		
		if (m_antialiasingType != aANTIALIAS_TYPE_NONE)
			rasterizerState.MultisampleEnable = true;//state.multisampleEnable;
		else
			rasterizerState.MultisampleEnable = false;

		/*
			If multisample is false than the line antialiasing algorithm is used.
			Else, lines are drawn as quads, which may affect performance?
			See D3D10_RASTERIZER_DESC page on the web.

			TODO : force MultisampleEnable to false if AntialiasedLineEnable is true?
		*/
		rasterizerState.AntialiasedLineEnable = false; //dont use AA on line drawings
	 
		ID3D10RasterizerState* pRS;
		if (FAILED(m_pDevice->CreateRasterizerState(&rasterizerState, &pRS)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create rasterizer state.", 
					"CMashD3D10Renderer::CreateRasterizerState");
			return -1;
		}

		D3D10_DEPTH_STENCIL_DESC depthStencilStates;
		depthStencilStates.DepthEnable = state.depthTestingEnable;
		depthStencilStates.DepthFunc = MashToD3D10DepthFunc(state.depthComparison);
		depthStencilStates.DepthWriteMask = (state.depthWritingEnabled)?D3D10_DEPTH_WRITE_MASK_ALL:D3D10_DEPTH_WRITE_MASK_ZERO;
		depthStencilStates.StencilEnable = FALSE;
		depthStencilStates.StencilReadMask = D3D10_DEFAULT_STENCIL_READ_MASK;
		depthStencilStates.StencilWriteMask = D3D10_DEFAULT_STENCIL_WRITE_MASK;

		ID3D10DepthStencilState* pDSS;
		if (FAILED(m_pDevice->CreateDepthStencilState(&depthStencilStates, &pDSS)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create depth stencil state.", 
					"CMashD3D10Renderer::CreateRasterizerState");
			return -1;
		}

		m_rasterizerStates.PushBack(sRasterizerStateData(pRS, pDSS, state));

		return (m_rasterizerStates.Size() - 1);
	}

	int32 CMashD3D10Renderer::AddBlendState(const sBlendStates &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_blendStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (m_blendStates[i].stateData == state)
				return i;
		}

		D3D10_BLEND_DESC1 blendState;
		blendState.AlphaToCoverageEnable = FALSE;
		blendState.IndependentBlendEnable = FALSE;
		D3D10_RENDER_TARGET_BLEND_DESC1 *blendRenderTarget = &blendState.RenderTarget[0];
		blendRenderTarget->BlendEnable = (state.blendingEnabled==true)?TRUE:FALSE;
		blendRenderTarget->SrcBlend = MashToD3D10BlendState(state.srcBlend);
		blendRenderTarget->DestBlend = MashToD3D10BlendState(state.destBlend);
		blendRenderTarget->BlendOp = MashToD3D10BlendOp(state.blendOp);
		blendRenderTarget->SrcBlendAlpha = MashToD3D10BlendState(state.srcBlendAlpha);
		blendRenderTarget->DestBlendAlpha = MashToD3D10BlendState(state.destBlendAlpha);
		blendRenderTarget->BlendOpAlpha = MashToD3D10BlendOp(state.blendOpAlpha);
		blendRenderTarget->RenderTargetWriteMask = MashToD3D10ColourWriteMask(state.colourWriteMask);

		//initilaize all other buffers to default values
		for(uint32 i = 1; i < 8; ++i)
		{
			blendState.RenderTarget[i].BlendEnable = FALSE;
			blendState.RenderTarget[i].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		}
	 
		ID3D10BlendState1* pBlendState = 0;
		if (FAILED(m_pDevice->CreateBlendState1(&blendState, &pBlendState)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create blend state.", 
					"CMashD3D10Renderer::CreateBlendState");
			return -1;
		}

		m_blendStates.PushBack(sBlendStateData(pBlendState, state));

		return (m_blendStates.Size() - 1);
	}

	MashTextureState* CMashD3D10Renderer::AddSamplerState(const sSamplerState &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_samplerStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (*(m_samplerStates[i]->GetSamplerState()) == state)
				return m_samplerStates[i];
		}

		/*
			Is mipmapping enabled?
		*/
		float maxLod = FLT_MAX;
		if (state.filter == aFILTER_MIN_MAG_POINT || 
			state.filter == aFILTER_MIN_MAG_LINEAR)
			maxLod = 0.0f;

		D3D10_SAMPLER_DESC desc;
		desc.Filter = MashToD3D10SamplerFilter(state.filter);
		desc.AddressU = MashToD3D10SamplerAddress(state.uMode);
		desc.AddressV = MashToD3D10SamplerAddress(state.vMode);
		desc.AddressW = D3D10_TEXTURE_ADDRESS_CLAMP;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = maxLod;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = mash::math::Clamp<int32>(1, 16, state.maxAnistropy);
		desc.ComparisonFunc = D3D10_COMPARISON_NEVER;
		desc.BorderColor[0] = 0.0f;
		desc.BorderColor[1] = 0.0f;
		desc.BorderColor[2] = 0.0f;
		desc.BorderColor[3] = 0.0f;

		ID3D10SamplerState *D3D9SamplerState = 0;
		if (FAILED(m_pDevice->CreateSamplerState(&desc, &D3D9SamplerState)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create sampler state.", 
					"CMashD3D10Renderer::CreateSamplerState");
			return 0;
		}

		CMashD3D10TextureState *pNewSamplerData = MASH_NEW_COMMON CMashD3D10TextureState(D3D9SamplerState, state);
		m_samplerStates.PushBack(pNewSamplerData);

		return pNewSamplerData;
	}

	eMASH_STATUS CMashD3D10Renderer::SetRasteriserState(int32 iState)
	{
		if (!m_lockRasterizerState)
		{
			if (iState < 0 || iState >= m_rasterizerStates.Size())
				iState = m_defaultRasterizerState;

			/*
				Only update the state if the new state
				has different values
			*/
			//if (m_currentRasterizerState != iState)
			{
				m_pDevice->RSSetState(m_rasterizerStates[iState].pD3D10RasterizerState);
				m_pDevice->OMSetDepthStencilState(m_rasterizerStates[iState].pD3D10DepthStencilState, 0);
				m_currentRasterizerState = iState;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::SetBlendState(int32 iState)
	{
		if (!m_lockBlendState)
		{
			if (iState < 0 && iState >= m_blendStates.Size())
				iState = m_defaultBlendState;

			/*
				Only update the state if the new state
				has different values
			*/
			//if (m_currentBlendState != iState)
			{
				m_pDevice->OMSetBlendState(m_blendStates[iState].pD3D10BlendState, 0, 0xffffffff);

				m_currentBlendState = iState;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::BeginRender()
	{
		MashVideoIntermediate::BeginRender();

		m_pDevice->ClearRenderTargetView(m_pDefaultRenderTargetView, m_FillColour.v);
		m_pDevice->ClearDepthStencilView(m_pDefaultDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::EndRender()
	{
		MashVideoIntermediate::EndRender();

		m_pSwapChain->Present(m_vsyncLevel, 0);

		return aMASH_OK;
	}

	const mash::MashVector2 CMashD3D10Renderer::GetBackBufferSize(bool returnActiveRenderSurfaceSize)const
	{
		if (!returnActiveRenderSurfaceSize || !m_currentRenderSurface)
		{
			return mash::MashVector2(m_swapChainDesc.BufferDesc.Width, m_swapChainDesc.BufferDesc.Height);
		}
		else
		{
			return m_currentRenderSurface->GetDimentions();
		}
	}

	void CMashD3D10Renderer::SetD3D10VertexShaderResourceView(uint32 index, ID3D10ShaderResourceView *view)
	{
#ifdef MASH_DEBUG
		if (index > g_maxD3D10ShaderResourceViewIndex)
		{
			MASH_LOG_BOUNDS_ERROR(index, 0, g_maxD3D10ShaderResourceViewIndex, "index", "CMashInputManager::SetD3D10VertexShaderResourceView")
			return;
		}
#endif
		m_pDevice->VSSetShaderResources(index, 1, &view);
	
		if (!m_currentVertexShaderResourceFlags.test(index))
		{
			m_currentVertexShaderResourceFlags.set(index);
			m_currentVertexShaderResourceIndices.PushBack(index);
		}
	}

	void CMashD3D10Renderer::SetD3D10PixelShaderResourceView(uint32 index, ID3D10ShaderResourceView *view)
	{
#ifdef MASH_DEBUG
		if (index > g_maxD3D10ShaderResourceViewIndex)
		{
			MASH_LOG_BOUNDS_ERROR(index, 0, g_maxD3D10ShaderResourceViewIndex, "index", "CMashInputManager::SetD3D10PixelShaderResourceView")
			return;
		}
#endif

		m_pDevice->PSSetShaderResources(index, 1, &view);

		if (!m_currentPixelShaderResourceFlags.test(index))
		{
			m_currentPixelShaderResourceFlags.set(index);
			m_currentPixelShaderResourceIndices.PushBack(index);
		}
	}

	void CMashD3D10Renderer::SetD3D10PixelShaderSampler(uint32 index, const MashTextureState *state)
	{
#ifdef MASH_DEBUG
		if (index > g_maxD3D10ShaderSamplers)
		{
			MASH_LOG_BOUNDS_ERROR(index, 0, g_maxD3D10ShaderSamplers, "index", "CMashInputManager::SetD3D10PixelShaderSampler")
			return;
		}
#endif

		ID3D10SamplerState *D3DSampler = (ID3D10SamplerState*)((CMashD3D10TextureState*)state)->GetD3D10SamplerState();
		m_pDevice->PSSetSamplers(index, 1, &D3DSampler);

		if (!m_currentPixelShaderSamplerFlags.test(index))
		{
			m_currentPixelShaderSamplerFlags.set(index);
			m_currentPixelShaderSamplerIndices.PushBack(index);
		}
	}

	void CMashD3D10Renderer::SetD3D10VertexShaderSampler(uint32 index, const MashTextureState *state)
	{
#ifdef MASH_DEBUG
		if (index > g_maxD3D10ShaderSamplers)
		{
			MASH_LOG_BOUNDS_ERROR(index, 0, g_maxD3D10ShaderSamplers, "index", "CMashInputManager::SetD3D10VertexShaderSampler")
			return;
		}
#endif

		ID3D10SamplerState *D3DSampler = (ID3D10SamplerState*)((CMashD3D10TextureState*)state)->GetD3D10SamplerState();
		m_pDevice->VSSetSamplers(index, 1, &D3DSampler);

		if (!m_currentVertexShaderSamplerFlags.test(index))
		{
			m_currentVertexShaderSamplerFlags.set(index);
			m_currentVertexShaderSamplerIndices.PushBack(index);
		}
	}

	void CMashD3D10Renderer::ResetUsedShaderResources()
	{
		//Reset resource views
		ID3D10ShaderResourceView *const nullViews[1] = {NULL};
		if (!m_currentVertexShaderResourceIndices.Empty())
		{
			const uint32 vertexShaderCount = m_currentVertexShaderResourceIndices.Size();
			for(uint32 i = 0; i < vertexShaderCount; ++i)
			{
				m_pDevice->VSSetShaderResources(m_currentVertexShaderResourceIndices[i], 1, nullViews);
				m_currentVertexShaderResourceFlags.reset(m_currentVertexShaderResourceIndices[i]);
			}

			m_currentVertexShaderResourceIndices.Clear();
		}

		if (!m_currentPixelShaderResourceIndices.Empty())
		{
			const uint32 pixelShaderCount = m_currentPixelShaderResourceIndices.Size();
			for(uint32 i = 0; i < pixelShaderCount; ++i)
			{
				m_pDevice->PSSetShaderResources(m_currentPixelShaderResourceIndices[i], 1, nullViews);
				m_currentPixelShaderResourceFlags.reset(m_currentPixelShaderResourceIndices[i]);
			}

			m_currentPixelShaderResourceIndices.Clear();
		}

		//reset sampler states
		if (!m_currentVertexShaderSamplerIndices.Empty())
		{
			const uint32 vertexShaderCount = m_currentVertexShaderSamplerIndices.Size();
			for(uint32 i = 0; i < vertexShaderCount; ++i)
			{
				m_pDevice->VSSetShaderResources(m_currentVertexShaderSamplerIndices[i], 1, nullViews);
				m_currentVertexShaderSamplerFlags.reset(m_currentVertexShaderSamplerIndices[i]);
			}

			m_currentVertexShaderSamplerIndices.Clear();
		}

		if (!m_currentPixelShaderSamplerIndices.Empty())
		{
			const uint32 pixelShaderCount = m_currentPixelShaderSamplerIndices.Size();
			for(uint32 i = 0; i < pixelShaderCount; ++i)
			{
				m_pDevice->PSSetShaderResources(m_currentPixelShaderSamplerIndices[i], 1, nullViews);
				m_currentPixelShaderSamplerFlags.reset(m_currentPixelShaderSamplerIndices[i]);
			}

			m_currentPixelShaderSamplerIndices.Clear();
		}
	}

	void CMashD3D10Renderer::ResetUsedBuffers()
	{
		m_pDevice->IASetIndexBuffer(0,  DXGI_FORMAT_R16_UINT, 0);

		if (m_instancedRenderBufferCount > 1)
		{
			for(uint32 i = 0; i < m_instancedRenderBufferCount; ++i)
			{
				m_instancedRenderBuffers[i] = 0;
				m_instancedRenderStrides[i] = 0;
				m_instancedRenderOffsets[i] = 0;
			}

			//reset slots back to zero
			m_pDevice->IASetVertexBuffers(0, m_instancedRenderBufferCount, m_instancedRenderBuffers, m_instancedRenderStrides, m_instancedRenderOffsets);
		}
		else
		{
			uint32 iStride = 0;
			uint32 iOffset = 0;
			ID3D10Buffer *vb[1] = {0};
			m_pDevice->IASetVertexBuffers(0, 1, vb, &iStride, &iOffset);
		}
	}

	eMASH_STATUS CMashD3D10Renderer::_SaveTextureToFile(ID3D10Texture2D *d3dTexture, eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const
	{
		D3DX10_IMAGE_FILE_FORMAT d3dFormat = D3DX10_IFF_BMP;
		switch(outputFormat)
		{
		case aSAVE_TEX_FORMAT_BMP:
			d3dFormat = D3DX10_IFF_BMP;
			break;
		case aSAVE_TEX_FORMAT_DDS:
			d3dFormat = D3DX10_IFF_DDS;
			break;
		}

		if (FAILED(D3DX10SaveTextureToFile(d3dTexture, d3dFormat, file.GetCString())))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to save texture to file.", 
					"CMashD3D10Renderer::_SaveTextureToFile");

			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::SaveScreenShotToFile(eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const
	{
		ID3D10Texture2D *pBackBuffer = 0;

		if (FAILED(m_pSwapChain->GetBuffer(0,
			__uuidof(ID3D10Texture2D),
			(LPVOID*)&pBackBuffer)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to get d3d10 back buffer.", 
					"CMashD3D10Renderer::SaveScreenShotToFile");

			return aMASH_FAILED;
		}

		if (!pBackBuffer)
			return aMASH_OK;

		D3D10_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize = 1;
		texDesc.BindFlags = 0;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = m_swapChainDesc.BufferDesc.Format;
		texDesc.Width = m_swapChainDesc.BufferDesc.Width;
		texDesc.Height = m_swapChainDesc.BufferDesc.Height;
		texDesc.MipLevels = 1;
		texDesc.MiscFlags = 0;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D10_USAGE_DEFAULT;

		ID3D10Texture2D *newTexture;
		if (FAILED(m_pDevice->CreateTexture2D(&texDesc, 0, &newTexture)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create temp texture buffer.", 
					"CMashD3D10Renderer::SaveScreenShotToFile");
			return aMASH_FAILED;
		}

		if (m_antialiasingType != aANTIALIAS_TYPE_NONE)
		{
			m_pDevice->ResolveSubresource(newTexture, 0, pBackBuffer, 0, m_swapChainDesc.BufferDesc.Format);
		}
		else
		{
			m_pDevice->CopyResource(newTexture, pBackBuffer);
		}
		
		eMASH_STATUS returnVal = _SaveTextureToFile(newTexture, outputFormat, file);
		newTexture->Release();
		return returnVal;
	}

	eMASH_STATUS CMashD3D10Renderer::SaveTextureToFile(const MashTexture *texture, 
		eSAVE_TEXTURE_FORMAT outputFormat, 
		const MashStringc &file)const
	{
		if (!texture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"No texture pointer given.", 
					"CMashD3D10Renderer::SaveTextureToFile");

			return aMASH_FAILED;
		}

		ID3D10Texture2D *d3dTexture = 0;

		if (texture->GetType() == eRESOURCE_TYPE::aRESOURCE_TEXTURE)
			d3dTexture = ((CMashD3D10Texture*)texture)->GetD3D10Buffer();
		else
			d3dTexture = ((CMashD3D10CubeTexture*)texture)->GetD3D10Buffer();

		if (!d3dTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"The given texture does not contain a valid D3D texture pointer.", 
					"CMashD3D10Renderer::SaveTextureToFile");

			return aMASH_FAILED;
		}

		return _SaveTextureToFile(d3dTexture, outputFormat, file);
	}

	eMASH_STATUS CMashD3D10Renderer::OnResolutionChange(uint32 width, uint32 height)
	{
		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashD3D10Renderer::OnResolutionChange",
					"Resolution changing to %u, %u.", width, height);

		ResetUsedShaderResources();
		ResetUsedBuffers();

		m_swapChainDesc.BufferDesc.Width = width;
		m_swapChainDesc.BufferDesc.Height = height;

		m_pDevice->OMSetRenderTargets(0, 0, 0);

		//clean up any other references
		if (OnPreResolutionChangeIntermediate() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to resize window.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		if (m_pDefaultRenderTargetView)
			while(m_pDefaultRenderTargetView->Release()){}

		if (m_pDefaultDepthStencilView)
			while(m_pDefaultDepthStencilView->Release()){}

		m_pDevice->ClearState();
  
		m_pDefaultRenderTargetView = 0;
		m_pDefaultDepthStencilView = 0;

		if (FAILED(m_pSwapChain->ResizeBuffers(m_swapChainDesc.BufferCount,
			m_swapChainDesc.BufferDesc.Width,
			m_swapChainDesc.BufferDesc.Height,
			m_swapChainDesc.BufferDesc.Format,
			m_swapChainDesc.Flags)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to resize window.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		ID3D10Texture2D *pBackBuffer = 0;

		if (FAILED(m_pSwapChain->GetBuffer(0,
			__uuidof(ID3D10Texture2D),
			(LPVOID*)&pBackBuffer)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to get d3d10 back buffer.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		if (!pBackBuffer)
			return aMASH_OK;

		if (FAILED(m_pDevice->CreateRenderTargetView(pBackBuffer,
			NULL, 
			&m_pDefaultRenderTargetView)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default render target.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		pBackBuffer->Release();

		//create depth stencil texture
		m_depthTextureDesc.Width = m_swapChainDesc.BufferDesc.Width;
		m_depthTextureDesc.Height = m_swapChainDesc.BufferDesc.Height;

		ID3D10Texture2D *pDepthStencil = 0;
		if (FAILED(m_pDevice->CreateTexture2D(&m_depthTextureDesc, NULL, &pDepthStencil)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default depth texture.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencil, &m_depthStencilViewDesc, &m_pDefaultDepthStencilView)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create d3d10 default depth buffer.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		m_pDevice->OMSetRenderTargets(1, //specify that we will only be using 1 render target
			&m_pDefaultRenderTargetView, m_pDefaultDepthStencilView);
		
		sMashViewPort viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = m_swapChainDesc.BufferDesc.Width;
		viewport.height = m_swapChainDesc.BufferDesc.Height;
		viewport.minZ = 0.0f;
		viewport.maxZ = 1.0f;

		m_defaultRenderTargetViewport = viewport;

		SetViewport(viewport);

		/*
			Update render targets and other common attribs
		*/
		if (OnPostResolutionChangeIntermediate(m_swapChainDesc.BufferDesc.Width, m_swapChainDesc.BufferDesc.Height) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to change resolution.", 
					"CMashD3D10Renderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
					"Resolution change succeeded.", 
					"CMashD3D10Renderer::OnResolutionChange");

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::SetScreenResolution(bool fullscreen, uint32 width, uint32 height)
	{
		bool backbufferSizeNeedsUpdating = (m_swapChainDesc.BufferDesc.Width != width) || (m_swapChainDesc.BufferDesc.Height != height);
		if (backbufferSizeNeedsUpdating || (m_swapChainDesc.Windowed != !fullscreen))
		{
			m_swapChainDesc.BufferDesc.Width = width;
			m_swapChainDesc.BufferDesc.Height = height;

			if (m_swapChainDesc.Windowed != !fullscreen)
			{
				if (fullscreen)
				{
					m_swapChainDesc.Windowed = false;
				}
				else
				{
					RECT rect = {0 , 0, m_swapChainDesc.BufferDesc.Width, m_swapChainDesc.BufferDesc.Height};
					const DWORD dwstyle = WS_OVERLAPPEDWINDOW;
					const DWORD dwexstyle = WS_EX_APPWINDOW;
					AdjustWindowRectEx(&rect, dwstyle, false, dwexstyle);
					m_swapChainDesc.Windowed = true;

					//after SetWindowLongPtr we need to call SetWindowPos
					//for the changes to take effect.
					SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 
						rect.right, 
						rect.bottom,
						SWP_NOZORDER | SWP_SHOWWINDOW);

					//change the window style
					SetWindowLongPtr(m_hWnd, GWL_STYLE, dwstyle);

					ShowWindow(m_hWnd,SW_SHOW);
				}

				if (FAILED(m_pSwapChain->ResizeTarget(&m_swapChainDesc.BufferDesc)))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to change resolution in call SwapChain::ResizeTarget.", 
						"CMashD3D10Renderer::SetScreenResolution");

					return aMASH_FAILED;
				}

				//we need to force the values back cause OnResolutionChange() will recieve wrong, windowed values
				m_swapChainDesc.BufferDesc.Width = width;
				m_swapChainDesc.BufferDesc.Height = height;
				if (FAILED(m_pSwapChain->SetFullscreenState(!m_swapChainDesc.Windowed, 0)))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to change resolution in call SwapChain::SetFullscreenState.", 
						"CMashD3D10Renderer::SetScreenResolution");

					return aMASH_FAILED;
				}
			}
			
			if (backbufferSizeNeedsUpdating)
			{
				uint32 oldRefreshRateDenom = m_swapChainDesc.BufferDesc.RefreshRate.Denominator;
				uint32 oldRefreshRateNum = m_swapChainDesc.BufferDesc.RefreshRate.Numerator;

				m_swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
				m_swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;

				if (FAILED(m_pSwapChain->ResizeTarget(&m_swapChainDesc.BufferDesc/*&displayModeList[correctDisplayModeIndex]*/)))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to change resolution in call SwapChain::ResizeTarget.", 
						"CMashD3D10Renderer::SetScreenResolution");

					return aMASH_FAILED;
				}

				m_swapChainDesc.BufferDesc.RefreshRate.Denominator = oldRefreshRateDenom;
				m_swapChainDesc.BufferDesc.RefreshRate.Numerator = oldRefreshRateNum;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::SetViewport(const sMashViewPort &viewport)
	{
		if ((viewport.height == m_viewPort.height) &&
			(viewport.width == m_viewPort.width) &&
			(viewport.maxZ == m_viewPort.maxZ) &&
			(viewport.minZ == m_viewPort.minZ) &&
			(viewport.x == m_viewPort.x) &&
			(viewport.y == m_viewPort.y))
		{
			return aMASH_OK;
		}

		D3D10_VIEWPORT vp;
		vp.Width = viewport.width;
		vp.Height = viewport.height;
		vp.MinDepth = mash::math::Max<float>(viewport.minZ, 0.000f);//was set to 0.001
		vp.MaxDepth = viewport.maxZ;
		vp.TopLeftX = viewport.x;
		vp.TopLeftY = viewport.y;

		m_pDevice->RSSetViewports(1, &vp);
		
		m_viewPort = viewport;

		_OnViewportChange();

		/*
			TODO : Remove this when the viewport issue is resolved.
			atm, d3d10 render surfaces need to have the viewport adjusted
			to the render target size for rendering to be correct.
		*/
		//if (!m_currentRenderSurface)
		//	m_defaultRenderTargetViewport = viewport;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::SetVertexFormat(MashVertex *pVertex)
	{
		if (!pVertex)
		{
			m_pDevice->IASetInputLayout(0);
			m_renderInfo->SetVertex(0);
		}
		else if (m_renderInfo->GetVertex() != pVertex)
		{
			m_pDevice->IASetInputLayout(((CMashD3D10Vertex*)pVertex)->GetD3D10Buffer());
			m_renderInfo->SetVertex(pVertex);
		}

		return aMASH_OK;
	}

	MashVertex* CMashD3D10Renderer::_CreateVertexType(MashMaterial *material,
				const sMashVertexElement *vertexDecl,
				uint32 iDeclCount)
	{
		/*
			TODO : Make sure vertex decl stream are in incremental order
		*/

		MashVertex *pVertex = 0;
		//look for prev loaded vertex decl
		const int32 iVertexCount = m_vertexTypes.Size();
		for(int32 i = 0; i < iVertexCount; ++i)
		{
			if (m_vertexTypes[i]->IsEqual(vertexDecl, iDeclCount))
			{
				pVertex = m_vertexTypes[i];
				break;
			}
		}

		if (!pVertex)
		{
			const int32 numElements = iDeclCount;
			if (numElements <= 0)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create vertex type. Empty declaration.", 
						"CMashD3D10Renderer::CreateVertexType");
				return 0;
			}

			uint32 iVertexSizeInBytes = 0;
			for(uint32 i = 0; i < numElements; ++i)
				iVertexSizeInBytes += helpers::GetVertexDeclTypeSize(vertexDecl[i].type);

			pVertex = MASH_NEW_COMMON CMashD3D10Vertex(vertexDecl, numElements, iVertexSizeInBytes);
			m_vertexTypes.PushBack(pVertex);

			_AddCompileDependency(material, (CMashD3D10Vertex*)pVertex);
		}

		return pVertex;
	}

	eMASH_STATUS CMashD3D10Renderer::SetRenderTargetDefault()
	{
		if (m_currentRenderSurface)
		{
			SetRenderTarget(0, 0);

			ID3D10RenderTargetView *rtViews[1] = {m_pDefaultRenderTargetView};
			m_pDevice->OMSetRenderTargets(1, rtViews, m_pDefaultDepthStencilView);

			SetViewport(m_defaultRenderTargetViewport);
		}

		return aMASH_OK;
	}

	MashMeshBuffer* CMashD3D10Renderer::CreateMeshBuffer(const sVertexStreamInit *initVertexStreamData,
				uint32 initVertexStreamCount,
				const MashVertex *vertexDecl, 
				const void *indexData, 
				uint32 indexCount, 
				eFORMAT indexFormat,
				eUSAGE indexUsage)
	{
		//needs to be at least one stream
		if (!initVertexStreamCount || !initVertexStreamData)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid vertex stream count or null pointer given. Mesh buffer was not created.", 
					"CMashD3D10Renderer::CreateMeshBuffer");

			return 0;
		}

		if (initVertexStreamCount > vertexDecl->GetStreamCount())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"The init vertex stream count is greater than the vertex declarations stream count. It must be less than or equal to.", 
					"CMashD3D10Renderer::CreateMeshBuffer");

			return 0;
		}

		MashArray<MashVertexBuffer*> vertexStreams;
		bool error = false;

		for(uint32 i = 0; i < vertexDecl->GetStreamCount(); ++i)
		{
			if ((initVertexStreamCount > i) && (initVertexStreamData[i].dataSizeInBytes > 0))
			{
				MashVertexBuffer *vertexBuffer = CreateVertexBuffer(initVertexStreamData[i].data, initVertexStreamData[i].dataSizeInBytes, initVertexStreamData[i].usage);
				if (!vertexBuffer)
					error = true;
				else
					vertexStreams.PushBack(vertexBuffer);
			}
		}

		if (error)
		{
			for(uint32 i = 0; i < vertexStreams.Size(); ++i)
			{
				MASH_DELETE vertexStreams[i];
			}

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create vertex buffers.", 
					"CMashD3D10Renderer::CreateMeshBuffer");

			return 0;
		}

		MashIndexBuffer *indexBuffer = 0;
		if (indexData && (indexCount > 0))
		{
			indexBuffer = CreateIndexBuffer(indexData, indexCount, indexUsage, indexFormat);
			if (!indexBuffer)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create index buffers.", 
					"CMashD3D10Renderer::CreateMeshBuffer");

				return 0;
			}
		}

		CMashD3D10MeshBuffer *newBuffer = MASH_NEW_COMMON CMashD3D10MeshBuffer(this, vertexStreams, indexBuffer, (MashVertex*)vertexDecl);
		return newBuffer;
	}

	MashVertexBuffer* CMashD3D10Renderer::CreateVertexBuffer(ID3D10Buffer *pVertexBuffer, const D3D10_BUFFER_DESC &desc)
	{
		CMashD3D10VertexBuffer *pNewBuffer = MASH_NEW_COMMON CMashD3D10VertexBuffer(this, pVertexBuffer, desc);
		
		return pNewBuffer;
	}

	MashIndexBuffer* CMashD3D10Renderer::CreateIndexBuffer(ID3D10Buffer *pIndexBuffer, eFORMAT eFormat, const D3D10_BUFFER_DESC &desc)
	{

		CMashD3D10IndexBuffer *pNewBuffer = MASH_NEW_COMMON CMashD3D10IndexBuffer(this, pIndexBuffer, eFormat, desc);
		
		return pNewBuffer;
	}

	MashTexture* CMashD3D10Renderer::_CreateTexture(const MashStringc &sName, ID3D10Texture2D *pD3D10Texture, const D3D10_TEXTURE2D_DESC &desc)
	{
		ID3D10ShaderResourceView *pResourceView = 0;
		if (desc.Format != DXGI_FORMAT_D32_FLOAT)//Temp fix
		{
			D3D10_TEXTURE2D_DESC texDesc;
			pD3D10Texture->GetDesc(&texDesc);

			D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = desc.Format;
			viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = texDesc.MipLevels;
			viewDesc.Texture2D.MostDetailedMip = 0;
			if (FAILED(m_pDevice->CreateShaderResourceView(pD3D10Texture, &viewDesc, &pResourceView)))
			{
				pD3D10Texture->Release();

				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create texture resource view.", 
						"CMashD3D10Renderer::_CreateTexture");
				return 0;
			}
		}

		bool useMips = false;
		if (desc.MipLevels == 0)
			useMips = true;

		CMashD3D10Texture *pNewTexture = MASH_NEW_COMMON CMashD3D10Texture(this, pD3D10Texture, pResourceView, desc, m_textureIDCounter++, useMips, sName);

		m_textures.insert(std::make_pair(sName, pNewTexture));
		//pNewTexture->Grab();//grab a copy for this list

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashD3D10Renderer::_CreateTexture", 
					"Created new 2D texture '%s'.", sName.GetCString());

		return pNewTexture;
	}

	MashTexture* CMashD3D10Renderer::CreateTexture(const MashStringc &sName, const D3D10_TEXTURE2D_DESC &desc)
	{
		MashStringc sNewName;
		if (ValidateTextureName(sName, sNewName) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Texture creataion failed. The name given was not unique.", 
					"CMashD3D10Renderer::CreateTexture");
			return 0;
		}

		ID3D10Texture2D *pD3D10Texture = 0;
		if (FAILED(m_pDevice->CreateTexture2D(&desc, 0, &pD3D10Texture)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create texture.", 
					"CMashD3D10Renderer::CreateTexture");
			return 0;
		}

		return _CreateTexture(sNewName, pD3D10Texture, desc);
	}

	MashTexture* CMashD3D10Renderer::CreateCubeTexture(const MashStringc &sName, const D3D10_TEXTURE2D_DESC &desc)
	{
		MashStringc sNewName;
		if (ValidateTextureName(sName, sNewName) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Texture creataion failed. The name given was not unique.", 
					"CMashD3D10Renderer::CreateCubeTexture");
			return 0;
		}

		ID3D10Texture2D *pD3D10Texture = 0;
		if (FAILED(m_pDevice->CreateTexture2D(&desc, 0, &pD3D10Texture)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create texture.", 
					"CMashD3D10Renderer::CreateCubeTexture");
			return 0;
		}

		ID3D10ShaderResourceView *pResourceView = 0;
		if (desc.Format != DXGI_FORMAT_D32_FLOAT)//Temp fix
		{
			D3D10_TEXTURE2D_DESC texDesc;
			pD3D10Texture->GetDesc(&texDesc);

			D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = desc.Format;
			viewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.TextureCube.MipLevels = texDesc.MipLevels;
			viewDesc.TextureCube.MostDetailedMip = 0;
			if (FAILED(m_pDevice->CreateShaderResourceView(pD3D10Texture, &viewDesc, &pResourceView)))
			{
				pD3D10Texture->Release();

				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create texture resource view.", 
						"CMashD3D10Renderer::CreateCubeTexture");
				return 0;
			}
		}

		bool useMips = false;
		if (desc.MipLevels == 0)
			useMips = true;

		CMashD3D10CubeTexture *pNewTexture = MASH_NEW_COMMON CMashD3D10CubeTexture(this, pD3D10Texture, pResourceView, desc, m_textureIDCounter++, useMips, sName);

		m_textures.insert(std::make_pair(sNewName, pNewTexture));
	//	pNewTexture->Grab();//grab a copy for this list

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashD3D10Renderer::CreateCubeTexture.", 
					"Created new cube texture '%s'.", sNewName.GetCString());

		return pNewTexture;
	}

	MashTexture* CMashD3D10Renderer::AddTexture(const MashStringc &sName,
											uint32 iWidth, 
											uint32 iHeight, 
											bool useMipmaps, 
											eUSAGE usage, 
											eFORMAT format)
	{
		D3D10_TEXTURE2D_DESC desc;
		desc.Width = iWidth;
		desc.Height = iHeight;
		
		desc.ArraySize = 1;
		desc.Format = MashToD3D10Format(format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = MashToD3D10Usage(usage);
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (useMipmaps)
		{
			desc.MiscFlags |= D3D10_RESOURCE_MISC_GENERATE_MIPS;
			desc.BindFlags |= D3D10_BIND_RENDER_TARGET;//this flag is needed to create mipmaps
			desc.MipLevels = 0;
		}
		else
		{
			desc.MipLevels = 1;
		}

		if (usage == aUSAGE_DYNAMIC)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}

		return CreateTexture(sName, desc);
	}

	MashTexture* CMashD3D10Renderer::AddCubeTexture(const MashStringc &sName,
											uint32 iSize,
											bool useMipmaps, 
											eUSAGE usage, 
											eFORMAT format)
	{
		D3D10_TEXTURE2D_DESC desc;
		desc.Width = iSize;
		desc.Height = iSize;
		desc.ArraySize = 6;
		desc.Format = MashToD3D10Format(format);
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = MashToD3D10Usage(usage);
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

		if (useMipmaps)
		{
			desc.MiscFlags |= D3D10_RESOURCE_MISC_GENERATE_MIPS;
			desc.BindFlags |= D3D10_BIND_RENDER_TARGET;//this flag is needed to create mipmaps
			desc.MipLevels = 0;
		}
		else
		{
			desc.MipLevels = 1;
		}

		if (usage == aUSAGE_DYNAMIC)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}

		return CreateCubeTexture(sName, desc);
	}

	MashVertexBuffer* CMashD3D10Renderer::CreateVertexBuffer(const void *pData, uint32 iSizeInBytes, eUSAGE usage)
	{
		D3D10_BUFFER_DESC desc;
		desc.Usage = MashToD3D10Usage(usage);
		desc.ByteWidth = iSizeInBytes;
		desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (usage == aUSAGE_DYNAMIC)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}

		D3D10_SUBRESOURCE_DATA *initData = 0;

		D3D10_SUBRESOURCE_DATA initDataMem;
		initDataMem.pSysMem = pData;

		if (pData)
			initData = &initDataMem;

		ID3D10Buffer *pVertexBuffer = 0;
		HRESULT hr = m_pDevice->CreateBuffer(&desc, initData, &pVertexBuffer);
		if (FAILED(hr))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to get vertex buffer from D3D::CreateBuffer.", 
					"CMashD3D10Renderer::CreateVertexBuffer");
			return 0;
		}

		return CreateVertexBuffer(pVertexBuffer, desc);
	}

	MashIndexBuffer* CMashD3D10Renderer::CreateIndexBuffer(const void *pData, uint32 iIndexCount, eUSAGE usage, eFORMAT format)
	{
		int32 iElementSizeInBytes = 0;
		if (format == eFORMAT::aFORMAT_R16_UINT)
			iElementSizeInBytes = sizeof(int16);
		else
		{
			iElementSizeInBytes = sizeof(int32);
			//make sure the format is valid
			format = eFORMAT::aFORMAT_R32_UINT;
		}

		D3D10_BUFFER_DESC desc;
		desc.Usage = MashToD3D10Usage(usage);
		desc.ByteWidth = iIndexCount * iElementSizeInBytes;
		desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (usage == aUSAGE_DYNAMIC)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}

		D3D10_SUBRESOURCE_DATA *initData = 0;

		D3D10_SUBRESOURCE_DATA initDataMem;
		initDataMem.pSysMem = pData;

		if (pData)
			initData = &initDataMem;

		ID3D10Buffer *pIndexBuffer = 0;
		if (FAILED(m_pDevice->CreateBuffer(&desc, initData, &pIndexBuffer)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to get index buffer from D3D::CreateBuffer.", 
					"CMashD3D10Renderer::CreateIndexBuffer");
			return 0;
		}

		return CreateIndexBuffer(pIndexBuffer, format, desc);
	}

	MashIndexBuffer* CMashD3D10Renderer::CreateIndexBufferBySize(const void *pData, uint32 iSizeInBytes, eUSAGE usage, eFORMAT format)
	{
		int32 iElementSizeInBytes = 0;
		if (format == aFORMAT_R16_UINT)
			iElementSizeInBytes = sizeof(int16);
		else
		{
			iElementSizeInBytes = sizeof(int32);
			format = aFORMAT_R32_UINT;
		}

		D3D10_BUFFER_DESC desc;
		desc.Usage = MashToD3D10Usage(usage);
		desc.ByteWidth = iSizeInBytes;
		desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (usage == aUSAGE_DYNAMIC)
		{
			desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}

		D3D10_SUBRESOURCE_DATA *pInitData = 0;
		D3D10_SUBRESOURCE_DATA initData;
		initData.pSysMem = pData;

		if (pData)
			pInitData = &initData;

		ID3D10Buffer *pIndexBuffer = 0;
		if (FAILED(m_pDevice->CreateBuffer(&desc, pInitData, &pIndexBuffer)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to get index buffer from D3D::CreateBuffer.", 
					"CMashD3D10Renderer::CreateIndexBufferBySize");
			return 0;
		}

		return CreateIndexBuffer(pIndexBuffer, format, desc);
	}

	MashRenderSurface* CMashD3D10Renderer::CreateRenderSurface(int32 iWidth, int32 iHeight, const eFORMAT *pFormats,
			uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOption, eFORMAT eDepthFormat)
	{
		if (iTargetCount < 1)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render surface. Target count must be greater than 0.", 
					"CMashD3D10Renderer::CreateRenderSurface");
			return 0;
		}

		if (iTargetCount > MAX_RENDER_TARGET_COUNT)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render surface. The system does not support the number of targets requested.", 
					"CMashD3D10Renderer::CreateRenderSurface");
			return 0;
		}

		bool fitToScreen = false;
		uint32 newWidth = 0;
		uint32 newHeight = 0;
		if (iWidth <= 0 || iHeight <= 0)
		{
			newWidth = (uint32)m_defaultRenderTargetViewport.width;
			newHeight = (uint32)m_defaultRenderTargetViewport.height;

			fitToScreen = true;
		}
		else
		{
			newWidth = (uint32)iWidth;
			newHeight = (uint32)iHeight;
		}

		CMashD3D10RenderSurface *pSurface = MASH_NEW_COMMON CMashD3D10RenderSurface(fitToScreen);

		if (pSurface->CreateSurface(this, newWidth, newHeight, pFormats, iTargetCount, useMipmaps, depthOption, eDepthFormat) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create render surface.", 
					"CMashD3D10Renderer::CreateRenderSurface");
			MASH_DELETE pSurface;
			pSurface = 0;

			return 0;
		}

		m_renderSurfaces.PushBack(pSurface);

		return pSurface;
	}

	MashRenderSurface* CMashD3D10Renderer::CreateCubicRenderSurface(uint32 iSize, bool useMipmaps,
			eFORMAT eTextureFormat, bool bUseDepth, eFORMAT eDepthFormat)
	{
		CMashD3D10CubicRenderSurface *pSurface = MASH_NEW_COMMON CMashD3D10CubicRenderSurface();

		if (pSurface->CreateSurface(this, iSize, useMipmaps, eTextureFormat, bUseDepth, eDepthFormat) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create cubic render surface.", 
					"CMashD3D10Renderer::CreateCubicRenderSurface");
			MASH_DELETE pSurface;
			pSurface = 0;

			return 0;
		}

		return pSurface;
	}

	MashTexture* CMashD3D10Renderer::LoadTextureFromFile(const MashStringc &fileName)
	{
		MashFileStream *pFileStream = m_pFileManager->CreateFileStream();
		if (pFileStream->LoadFile(fileName.GetCString(), aFILE_IO_BINARY) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
				"CMashD3D10Renderer::LoadTextureFromFile",
				"Failed to load texture '%s'.", fileName.GetCString());

			pFileStream->Destroy();
			return 0;
		}

		ID3D10Texture2D *pD3D10Texture = 0;
		HRESULT hr;
		if (FAILED(D3DX10CreateTextureFromMemory(m_pDevice, 
			pFileStream->GetData(), 
			pFileStream->GetDataSizeInBytes(), 
			0, 
			0, 
			(ID3D10Resource**)&pD3D10Texture, 
			&hr)))
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
				"CMashD3D10Renderer::LoadTextureFromFile",
				"Failed to load texture '%s'.", fileName.GetCString());

			return 0;
		}

		pFileStream->Destroy();
		pFileStream = 0;

		D3D10_TEXTURE2D_DESC desc;
		pD3D10Texture->GetDesc(&desc);

		return _CreateTexture(fileName, pD3D10Texture, desc);
	}

	eMASH_STATUS CMashD3D10Renderer::ClearTarget(uint32 iClearFlags, const sMashColour4 &colour, float fZDepth)
	{
		if (!m_currentRenderSurface)
		{

			if ((iClearFlags & aCLEAR_DEPTH) || (iClearFlags & aCLEAR_STENCIL))
				m_pDevice->ClearDepthStencilView(m_pDefaultDepthStencilView, MashToD3D10ClearFlags(iClearFlags)/*D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL*/, fZDepth, 0);

			if (iClearFlags & aCLEAR_TARGET)
				m_pDevice->ClearRenderTargetView(m_pDefaultRenderTargetView, colour.v);
			
		}
		else
		{
			m_currentRenderSurface->_ClearTargets(iClearFlags, colour, fZDepth);
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::DrawIndexedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 iIndexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType)
	{
		MashVertex *vertexDeclaration = buffer->GetVertexDeclaration();
		/*
			The vertex format is set here because opengl sets it from within a vao.
		*/
		if (SetVertexFormat(vertexDeclaration) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to set vertex format for rendering.", 
					"CMashD3D10Renderer::DrawIndexedList");

			return aMASH_FAILED;
		}

		m_pDevice->IASetPrimitiveTopology(MashToD3D10Primitive(ePrimType));

		CMashD3D10VertexBuffer *pD3DVertexBuffer = (CMashD3D10VertexBuffer*)buffer->GetVertexBuffer();
		CMashD3D10IndexBuffer *pD3DIndexBuffer = (CMashD3D10IndexBuffer*)buffer->GetIndexBuffer();

		uint32 iStride = vertexDeclaration->GetStreamSizeInBytes(0);
		uint32 iOffset = 0;

		ID3D10Buffer *vb[1] = {pD3DVertexBuffer->GetD3D10Buffer()};
		m_pDevice->IASetVertexBuffers(0, buffer->GetVertexBufferCount(), vb, &iStride, &iOffset);
		m_pDevice->IASetIndexBuffer(pD3DIndexBuffer->GetD3D10Buffer(),  pD3DIndexBuffer->GetD3D10Format(), 0);

		m_pDevice->DrawIndexed(iIndexCount, 0, 0);
		++m_currentDrawCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::DrawVertexList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType)
	{
		MashVertex *vertexDeclaration = buffer->GetVertexDeclaration();
		/*
			The vertex format is set here because opengl sets it from within a vao.
		*/
		if (SetVertexFormat(vertexDeclaration) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to set vertex format for rendering.", 
					"CMashD3D10Renderer::DrawVertexList");

			return aMASH_FAILED;
		}

		m_pDevice->IASetPrimitiveTopology(MashToD3D10Primitive(ePrimType));

		CMashD3D10VertexBuffer *pD3DVertexBuffer = (CMashD3D10VertexBuffer*)buffer->GetVertexBuffer();

		uint32 iStride = vertexDeclaration->GetStreamSizeInBytes(0);
		uint32 iOffset = 0;

		ID3D10Buffer *vb[1] = {pD3DVertexBuffer->GetD3D10Buffer()};
		m_pDevice->IASetVertexBuffers(0, 1, vb, &iStride, &iOffset);

		m_pDevice->Draw(iVertexCount, 0);
		++m_currentDrawCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::DrawVertexInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount)
	{
		if (instanceCount == 0)
			return aMASH_OK;

		MashVertex *vertexDeclaration = buffer->GetVertexDeclaration();
		/*
			The vertex format is set here because opengl sets it from within a vao.
		*/
		if (SetVertexFormat(vertexDeclaration) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to set vertex format for rendering.", 
					"CMashD3D10Renderer::DrawVertexInstancedList");

			return aMASH_FAILED;
		}

		if (m_instancedRenderBufferCount < instanceCount)
		{
			if (m_instancedRenderBuffers)
				MASH_FREE(m_instancedRenderBuffers);

			m_instancedRenderBuffers = MASH_ALLOC_T_COMMON(ID3D10Buffer*, instanceCount);
			m_instancedRenderStrides = MASH_ALLOC_T_COMMON(UINT, instanceCount);
			m_instancedRenderOffsets = MASH_ALLOC_T_COMMON(UINT, instanceCount);

			memset(m_instancedRenderBuffers, 0, sizeof(ID3D10Buffer*) * instanceCount);
			memset(m_instancedRenderStrides, 0, sizeof(UINT) * instanceCount);
			memset(m_instancedRenderOffsets, 0, sizeof(UINT) * instanceCount);

			m_instancedRenderBufferCount = instanceCount;
		}

		/*
			TODO : Debug build checking to make sure the number of vertex buffers
			match the number of streams in a vertex declaration.
		*/

		uint32 vertexBufferCount = buffer->GetVertexBufferCount();
		for(uint32 i = 0; i < vertexBufferCount; ++i)
		{
			m_instancedRenderBuffers[i] = ((CMashD3D10VertexBuffer*)buffer->GetVertexBuffer(i))->GetD3D10Buffer();
			m_instancedRenderStrides[i] = vertexDeclaration->GetStreamSizeInBytes(i);
		}

		m_pDevice->IASetVertexBuffers(0, vertexBufferCount, m_instancedRenderBuffers, m_instancedRenderStrides, m_instancedRenderOffsets);
		m_pDevice->IASetPrimitiveTopology(MashToD3D10Primitive(ePrimType));

		m_pDevice->DrawInstanced(iVertexCount, instanceCount, 0, 0);
		++m_currentDrawCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashD3D10Renderer::DrawIndexedInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 indexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount)
	{
		if (instanceCount == 0)
			return aMASH_OK;

		MashVertex *vertexDeclaration = buffer->GetVertexDeclaration();
		/*
			The vertex format is set here because opengl sets it from within a vao.
		*/
		if (SetVertexFormat(vertexDeclaration) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to set vertex format for rendering.", 
					"CMashD3D10Renderer::DrawIndexedInstancedList");

			return aMASH_FAILED;
		}

		uint32 vertexBufferCount = buffer->GetVertexBufferCount();
		if (m_instancedRenderBufferCount < vertexBufferCount)
		{
			if (m_instancedRenderBuffers)
				MASH_FREE(m_instancedRenderBuffers);

			m_instancedRenderBuffers = MASH_ALLOC_T_COMMON(ID3D10Buffer*, vertexBufferCount);
			m_instancedRenderStrides = MASH_ALLOC_T_COMMON(UINT, vertexBufferCount);
			m_instancedRenderOffsets = MASH_ALLOC_T_COMMON(UINT, vertexBufferCount);

			memset(m_instancedRenderBuffers, 0, sizeof(ID3D10Buffer*) * vertexBufferCount);
			memset(m_instancedRenderStrides, 0, sizeof(UINT) * vertexBufferCount);
			memset(m_instancedRenderOffsets, 0, sizeof(UINT) * vertexBufferCount);

			m_instancedRenderBufferCount = vertexBufferCount;
		}

		/*
			TODO : Debug build checking to make sure the number of vertex buffers
			match the number of streams in a vertex declaration.
		*/
		for(uint32 i = 0; i < vertexBufferCount; ++i)
		{
			CMashD3D10VertexBuffer *vertexBuffer = (CMashD3D10VertexBuffer*)buffer->GetVertexBuffer(i);
			if (vertexBuffer)
			{
				m_instancedRenderBuffers[i] = vertexBuffer->GetD3D10Buffer();
				m_instancedRenderStrides[i] = vertexDeclaration->GetStreamSizeInBytes(i);
			}
		}

		m_pDevice->IASetVertexBuffers(0, vertexBufferCount, m_instancedRenderBuffers, m_instancedRenderStrides, m_instancedRenderOffsets);
		m_pDevice->IASetPrimitiveTopology(MashToD3D10Primitive(ePrimType));

		CMashD3D10IndexBuffer *pD3DIndexBuffer = (CMashD3D10IndexBuffer*)buffer->GetIndexBuffer();
		m_pDevice->IASetIndexBuffer(pD3DIndexBuffer->GetD3D10Buffer(),  pD3DIndexBuffer->GetD3D10Format(), 0);

		m_pDevice->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
		++m_currentDrawCount;

		return aMASH_OK;
	}
}

#endif