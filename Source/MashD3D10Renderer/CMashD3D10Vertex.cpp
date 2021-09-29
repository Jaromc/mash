//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10Vertex.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10EffectProgram.h"
#include "CMashD3D10Helper.h"
#include "MashTechniqueInstance.h"
#include "MashTechnique.h"
#include "MashMaterial.h"
#include "MashEffect.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10Vertex::CMashD3D10Vertex(const sMashVertexElement *pMashVertexDecl,
		uint32 iElementCount,
		uint32 iVertexSizeInBytes):MashVertexIntermediate(pMashVertexDecl, iElementCount, iVertexSizeInBytes), 
		m_pVertexDeclaration(0)
	{
	}

	CMashD3D10Vertex::~CMashD3D10Vertex()
	{
		//! Release the D3D object.
		if (m_pVertexDeclaration)
			m_pVertexDeclaration->Release();
	}

	void CMashD3D10Vertex::OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency)
	{
		if (IsValid())
			return;

		MashMaterial *material = (MashMaterial*)dependency;

		MashEffect *effect = material->GetActiveTechnique()->GetTechnique()->GetEffect();

		if (!effect || !effect->IsValid())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"A valid effect program was not supplied.", 
					"CMashD3D10Vertex::OnDependencyCompiled");
			return;
		}

        MashEffectProgram *vertexProgram = effect->GetProgramByType(aPROGRAM_VERTEX);

		if (!vertexProgram)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"A vertex effect program was not supplied.", 
					"CMashD3D10Vertex::OnDependencyCompiled");
			return;
		}

		D3D10_INPUT_ELEMENT_DESC *pD3DVertexDecl = MASH_ALLOC_T_COMMON(D3D10_INPUT_ELEMENT_DESC, GetVertexElementCount());
		for(uint32 i = 0; i < m_elementCount; ++i)
		{
			pD3DVertexDecl[i].SemanticName = MashVertexUsageToString(m_mashVertexDecl[i].usage);
			pD3DVertexDecl[i].SemanticIndex = m_mashVertexDecl[i].usageIndex;
			pD3DVertexDecl[i].Format = MashToD3D10VertexDeclType(m_mashVertexDecl[i].type);
			pD3DVertexDecl[i].InputSlot = m_mashVertexDecl[i].stream;
			pD3DVertexDecl[i].AlignedByteOffset = m_mashVertexDecl[i].stride;
			pD3DVertexDecl[i].InputSlotClass = MashToD3D10Classification(m_mashVertexDecl[i].classification);
			pD3DVertexDecl[i].InstanceDataStepRate = m_mashVertexDecl[i].instanceStepRate;
		}

		ID3D10Blob *pIASignature = ((CMashD3D10EffectProgram*)vertexProgram)->GetProgramIASignature();

		if (FAILED(((CMashD3D10Renderer*)renderer)->GetD3D10Device()->CreateInputLayout(pD3DVertexDecl,
			m_elementCount,
			pIASignature->GetBufferPointer(),
			pIASignature->GetBufferSize(),
			&m_pVertexDeclaration)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to compile vertex using D3D::CreateInputLayout().", 
					"CMashD3D10Vertex::OnDependencyCompiled");

			MASH_FREE(pD3DVertexDecl);

			return;
		}

		/*
			Build anything relying on this.
		*/
		renderer->_OnDependencyCompiled(this);

		MASH_FREE(pD3DVertexDecl);
	}
}
