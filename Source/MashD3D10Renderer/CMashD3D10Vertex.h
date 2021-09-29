//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_VERTEX_H_
#define _C_MASH_D3D10_VERTEX_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashVertexIntermediate.h"
#include <d3d10_1.h>
#include "MashMaterialDependentResource.h"

namespace mash
{
	class CMashD3D10Vertex : public MashVertexIntermediate, public MashMaterialDependentResource<CMashD3D10Vertex>
	{
	private:
		ID3D10InputLayout *m_pVertexDeclaration;

	public:
			CMashD3D10Vertex(const sMashVertexElement *pMashVertexDecl,
				uint32 iElementCount,
				uint32 iVertexSizeInBytes);
			~CMashD3D10Vertex();

			bool IsValid()const;
			void OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency);

			ID3D10InputLayout* GetD3D10Buffer()const;
	};

	inline bool CMashD3D10Vertex::IsValid()const
	{
		return (m_pVertexDeclaration)?true:false;
	}

	inline ID3D10InputLayout* CMashD3D10Vertex::GetD3D10Buffer()const
	{
		return m_pVertexDeclaration;
	}
}

#endif

#endif
