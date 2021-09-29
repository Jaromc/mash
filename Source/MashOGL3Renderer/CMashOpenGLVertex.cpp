//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLVertex.h"
#include "CMashOpenGLEffect.h"
#include "MashLog.h"
#include "MashTechnique.h"
#include "MashTechniqueInstance.h"
#include "MashMaterial.h"
#include "MashVideo.h"
#include "MashHelper.h"
namespace mash
{
	CMashOpenGLVertex::CMashOpenGLVertex(const sMashVertexElement *pMashVertexDecl,
		uint32 iElementCount,
		uint32 iVertexSizeInBytes):MashVertexIntermediate(pMashVertexDecl, iElementCount, iVertexSizeInBytes),
		m_isCompiled(false)
	{
	}

	CMashOpenGLVertex::~CMashOpenGLVertex()
	{
	}
	
	void CMashOpenGLVertex::OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency)
	{
		if (IsValid())
			return;

		MashMaterial *material = (MashMaterial*)dependency;
		MashEffect *effect = material->GetActiveTechnique()->GetTechnique()->GetEffect();

		if (!effect || !effect->IsValid())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"A valid effect program was not supplied.", 
					"CMashOpenGLVertex::_Compile");
			return;
		}

		const MashArray<sMashOpenGLVertexAttribute> &inputAttributes = ((CMashOpenGLEffect*)effect)->GetOpenGLVertexAttributes();
		if (inputAttributes.Empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"A vertex declaration was not found with the vertex program.", 
				"CMashOpenGLVertex::_Compile");

			return;
		}
		const uint32 attributeCount = inputAttributes.Size();

		m_openGLElements.Clear();
		m_openGLElements.Resize(attributeCount);

		#define BUFFER_OFFSET(i) ((int8 *)NULL + (i))
		
		for(uint32 attrib = 0; attrib < attributeCount; ++attrib)
		{
			bool found = false;
			for(uint32 elm = 0; elm < m_elementCount; ++elm)
			{
				if ((inputAttributes[attrib].type == m_mashVertexDecl[elm].usage) && 
					(inputAttributes[attrib].usageIndex == m_mashVertexDecl[elm].usageIndex))
				{
					m_openGLElements[attrib].attribute = inputAttributes[attrib].location;
					m_openGLElements[attrib].stream = m_mashVertexDecl[elm].stream;
					m_openGLElements[attrib].stepRate = m_mashVertexDecl[elm].instanceStepRate;
					m_openGLElements[attrib].size = mash::helpers::GetVertexDeclTypeElmCount(m_mashVertexDecl[elm].type);
					m_openGLElements[attrib].type = MashToOpenGLVertexDeclType(m_mashVertexDecl[elm].type);
					m_openGLElements[attrib].stride = GetStreamSizeInBytes(m_openGLElements[attrib].stream);

					if (m_mashVertexDecl[elm].type == aDECLTYPE_R8G8B8A8_UNORM)
						m_openGLElements[attrib].normalized = GL_TRUE;
					else
						m_openGLElements[attrib].normalized = GL_FALSE;

					m_openGLElements[attrib].pointer = BUFFER_OFFSET(m_mashVertexDecl[elm].stride);

					found = true;
					break;
				}
			}

			if (!found)
			{
				MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"CMashOpenGLVertex::_Compile",
                    "A vertex element '%s' was not found. Make sure this element is in your vertex effect and material declaration.", helpers::GetVertexUsageAsString(inputAttributes[attrib].type));
			}
		}

		m_isCompiled = true;

		/*
			Build anything relying on this.
		*/
		renderer->_OnDependencyCompiled(this);
	}

}