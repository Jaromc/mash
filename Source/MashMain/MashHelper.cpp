//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashHelper.h"
#include "MashLog.h"
#include "MashTechniqueInstance.h"
#include "MashFileManager.h"
#include "MashGenericScriptReader.h"
#include "MashTypes.h"
#include "MashStringHelper.h"
#include <cstdarg>
#include <cctype>
namespace mash
{
    namespace helpers
    {
	bool IsValidString(const char *s)
	{
		if (!s)
			return false;

		if (s[0] == 0)
			return false;

		return true;
	}

    void PrintToBuffer(int8 *buffer, size_t size, int8 *stringToWrite, ...)
    {
        va_list args;
		va_start(args, stringToWrite);
		vsnprintf(buffer, size, stringToWrite, args);
		va_end(args);
    }
    
	eMASH_STATUS ValidateAnimationKeyTypeToController(eMASH_CONTROLLER_TYPE controller, eANIM_KEY_TYPE key)
	{
		eMASH_STATUS status = aMASH_FAILED;

		switch(controller)
		{
		case aCONTROLLER_TRANSFORMATION:
			{
				if (key == aANIM_KEY_TRANSFORM)
					status = aMASH_OK;

				break;
			}
		}

		return status;
	}

	const int8* GetEffectProgramFileExtension()
	{
		static const int8 *ext = "eff";
		return ext;
	}

	bool IsFileANativeEffectProgram(const int8 *fileName)
	{
		MashStringc ext;
        GetFileExtention(fileName, ext);
		if (scriptreader::CompareStrings(GetEffectProgramFileExtension(), ext.GetCString()))
			return false;

		return true;
	}

	eSHADER_API_TYPE GetAPIFromShaderProfile(eSHADER_PROFILE profile)
	{
		switch(profile)
		{
		case aSHADER_PROFILE_VS_1_1:
		case aSHADER_PROFILE_VS_2_0:
		case aSHADER_PROFILE_VS_3_0:
		case aSHADER_PROFILE_PS_1_1:
		case aSHADER_PROFILE_PS_1_2:
		case aSHADER_PROFILE_PS_1_3:
		case aSHADER_PROFILE_PS_2_0:
		case aSHADER_PROFILE_PS_3_0:
			return aSHADERAPITYPE_D3D9;
		case aSHADER_PROFILE_VS_4_0:
		case aSHADER_PROFILE_VS_5_0:
		case aSHADER_PROFILE_PS_4_0:
		case aSHADER_PROFILE_PS_5_0:
		case aSHADER_PROFILE_GS_4_0:
		case aSHADER_PROFILE_GS_5_0:
			return aSHADERAPITYPE_D3D10;
		case aSHADER_PROFILE_VS_GLSL:
		case aSHADER_PROFILE_PS_GLSL:
		case aSHADER_PROFILE_GS_GLSL:
			return aSHADERAPITYPE_OPENGL;
		};

		return aSHADERAPITYPE_UNKNOWN;
	}

	const int8* GetShaderFileExtention(eSHADER_API_TYPE api)
	{
		switch(api)
		{
		case aSHADERAPITYPE_D3D9:
		case aSHADERAPITYPE_D3D10:
			return "hlsl";
		case aSHADERAPITYPE_OPENGL:
			return "glsl";
		};

		return 0;
	}

	uint32 GetVertexDeclTypeElmSize(mash::eVERTEX_DECLTYPE type)
	{
		switch((int32)type)
		{
		case mash::aDECLTYPE_R32_FLOAT:
				return sizeof(f32);
		case mash::aDECLTYPE_R32G32_FLOAT:
				return sizeof(f32);
		case mash::aDECLTYPE_R32G32B32_FLOAT:
				return sizeof(f32);
		case mash::aDECLTYPE_R32G32B32A32_FLOAT:
				return sizeof(f32);
		case mash::aDECLTYPE_R8G8B8A8_UNORM:
				return sizeof(int8);
		case mash::aDECLTYPE_R8G8B8A8_UINT:
				return sizeof(int8);
		case mash::aDECLTYPE_R16G16_SINT:
				return sizeof(int16);
		case mash::aDECLTYPE_R16G16B16A16_SINT:
				return sizeof(int16);
		};

		return 0;
	}

	uint32 GetVertexDeclTypeElmCount(mash::eVERTEX_DECLTYPE type)
	{
		switch((int32)type)
		{
		case mash::aDECLTYPE_R32_FLOAT:
				return 1;
		case mash::aDECLTYPE_R32G32_FLOAT:
				return 2;
		case mash::aDECLTYPE_R32G32B32_FLOAT:
				return 3;
		case mash::aDECLTYPE_R32G32B32A32_FLOAT:
				return 4;
		case mash::aDECLTYPE_R8G8B8A8_UNORM:
				return 4;
		case mash::aDECLTYPE_R8G8B8A8_UINT:
				return 4;
		case mash::aDECLTYPE_R16G16_SINT:
				return 2;
		case mash::aDECLTYPE_R16G16B16A16_SINT:
				return 4;
		};

		return 0;
	}

	uint32 GetVertexDeclTypeSize(mash::eVERTEX_DECLTYPE type)
	{
		switch((int32)type)
		{
		case mash::aDECLTYPE_R32_FLOAT:
				return sizeof(f32);
		case mash::aDECLTYPE_R32G32_FLOAT:
				return sizeof(f32) * 2;
		case mash::aDECLTYPE_R32G32B32_FLOAT:
				return sizeof(f32) * 3;
		case mash::aDECLTYPE_R32G32B32A32_FLOAT:
				return sizeof(f32) * 4;
		case mash::aDECLTYPE_R8G8B8A8_UNORM:
				return sizeof(uint32);
		case mash::aDECLTYPE_R8G8B8A8_UINT:
				return sizeof(uint8) * 4;
		case mash::aDECLTYPE_R16G16_SINT:
				return sizeof(int16) * 2;
		case mash::aDECLTYPE_R16G16B16A16_SINT:
				return sizeof(int16) * 4;
		};

		return 0;
	}

	void GetVertexStreamStartEndIndex(uint32 stream, 
		const sMashVertexElement *vertexDecl, 
		uint32 elementCount, 
		uint32 &streamStart, 
		uint32 &streamEnd)
	{
		for(streamStart = 0; streamStart < elementCount; ++streamStart)
		{
			if (vertexDecl[streamStart].stream == stream)
				break;
		}

		for(streamEnd = streamStart; streamEnd < elementCount; ++streamEnd)
		{
			if (vertexDecl[streamEnd].stream != stream)
				break;
		}
	}
        
    const char* GetVertexUsageAsString(eVERTEX_DECLUSAGE usage)
    {
        switch(usage)
        {
            case aDECLUSAGE_POSITION:
                return "position";
            case aDECLUSAGE_BLENDWEIGHT:
                return "blend_weight";
            case aDECLUSAGE_BLENDINDICES:
                return "blend_indices";
            case aDECLUSAGE_NORMAL:
                return "normal";
            case aDECLUSAGE_TEXCOORD:
                return "texcoord";
            case aDECLUSAGE_CUSTOM:
                return "custom";
            case aDECLUSAGE_TANGENT:
                return "tangent";
            case aDECLUSAGE_BINORMAL:
                return "binormal";
            case aDECLUSAGE_COLOUR:
                return "colour";
            default:
                return 0;
        }
    }

	eSHADER_PROFILE GetShaderProfileFromString(const int8 *profile)
	{
		if (strcmp(profile, "vs_1_1") == 0)
			return aSHADER_PROFILE_VS_1_1;
		else if (strcmp(profile, "vs_2_0") == 0)
			return aSHADER_PROFILE_VS_2_0;
		else if (strcmp(profile, "vs_3_0") == 0)
			return aSHADER_PROFILE_VS_3_0;
		else if (strcmp(profile, "vs_4_0") == 0)
			return aSHADER_PROFILE_VS_4_0;
		else if (strcmp(profile, "vs_5_0") == 0)
			return aSHADER_PROFILE_VS_5_0;
		else if (strcmp(profile, "ps_1_1") == 0)
			return aSHADER_PROFILE_PS_1_1;
		else if (strcmp(profile, "ps_1_2") == 0)
			return aSHADER_PROFILE_PS_1_2;
		else if (strcmp(profile, "ps_1_3") == 0)
			return aSHADER_PROFILE_PS_1_3;
		else if (strcmp(profile, "ps_2_0") == 0)
			return aSHADER_PROFILE_PS_2_0;
		else if (strcmp(profile, "ps_3_0") == 0)
			return aSHADER_PROFILE_PS_3_0;
		else if (strcmp(profile, "ps_4_0") == 0)
			return aSHADER_PROFILE_PS_4_0;
		else if (strcmp(profile, "ps_5_0") == 0)
			return aSHADER_PROFILE_PS_5_0;
		else if (strcmp(profile, "gs_4_0") == 0)
			return aSHADER_PROFILE_GS_4_0;
		else if (strcmp(profile, "gs_5_0") == 0)
			return aSHADER_PROFILE_GS_5_0;

		else if (strcmp(profile, "glslv") == 0)
			return aSHADER_PROFILE_VS_GLSL;
		else if (strcmp(profile, "glslg") == 0)
			return aSHADER_PROFILE_GS_GLSL;
		else if (strcmp(profile, "glslp") == 0)
			return aSHADER_PROFILE_PS_GLSL;

		return aSHADER_PROFILE_UNKNOWN;
	}

	const int8* GetShaderProfileString(eSHADER_PROFILE profile)
	{
		switch(profile)
		{
		case aSHADER_PROFILE_VS_1_1:
			return "vs_1_1";
		case aSHADER_PROFILE_VS_2_0:
			return "vs_2_0";
		case aSHADER_PROFILE_VS_3_0:
			return "vs_3_0";
		case aSHADER_PROFILE_VS_4_0:
			return "vs_4_0";
		case aSHADER_PROFILE_VS_5_0:
			return "vs_5_0";
		case aSHADER_PROFILE_PS_1_1:
			return "ps_1_1";
		case aSHADER_PROFILE_PS_1_2:
			return "ps_1_2";
		case aSHADER_PROFILE_PS_1_3:
			return "ps_1_3";
		case aSHADER_PROFILE_PS_2_0:
			return "ps_2_0";
		case aSHADER_PROFILE_PS_3_0:
			return "ps_3_0";
		case aSHADER_PROFILE_PS_4_0:
			return "ps_4_0";
		case aSHADER_PROFILE_PS_5_0:
			return "ps_5_0";
		case aSHADER_PROFILE_GS_4_0:
			return "gs_4_0";
		case aSHADER_PROFILE_GS_5_0:
			return "gs_5_0";
		case aSHADER_PROFILE_VS_GLSL:
			return "glslv";
		case aSHADER_PROFILE_GS_GLSL:
			return "glslg";
		case aSHADER_PROFILE_PS_GLSL:
			return "glslp";
		};

		return "";
	}

	ePROGRAM_TYPE GetEffectProgramTypeFromProfile(eSHADER_PROFILE profile)
	{
		ePROGRAM_TYPE programType = aPROGRAM_UNKNOWN;

		switch(profile)
		{
		case aSHADER_PROFILE_VS_1_1:
		case aSHADER_PROFILE_VS_2_0:
		case aSHADER_PROFILE_VS_3_0:
		case aSHADER_PROFILE_VS_4_0:
		case aSHADER_PROFILE_VS_5_0:
		case aSHADER_PROFILE_VS_GLSL:
			programType = aPROGRAM_VERTEX;
			break;
		case aSHADER_PROFILE_PS_1_1:
		case aSHADER_PROFILE_PS_1_2:
		case aSHADER_PROFILE_PS_1_3:
		case aSHADER_PROFILE_PS_2_0:
		case aSHADER_PROFILE_PS_3_0:
		case aSHADER_PROFILE_PS_4_0:
		case aSHADER_PROFILE_PS_5_0:
		case aSHADER_PROFILE_PS_GLSL:
			programType = aPROGRAM_PIXEL;
			break;
		case aSHADER_PROFILE_GS_4_0:
		case aSHADER_PROFILE_GS_5_0:
		case aSHADER_PROFILE_GS_GLSL:
			programType = aPROGRAM_GEOMETRY;
			break;
		};

		return programType;
	}

	eSHADER_PROFILE GetPixelProfileFromVertexProfile(eSHADER_PROFILE profile)
	{
		switch(profile)
		{
		case aSHADER_PROFILE_VS_1_1:
			return aSHADER_PROFILE_PS_1_1;
		case aSHADER_PROFILE_VS_2_0:
			return aSHADER_PROFILE_PS_2_0;
		case aSHADER_PROFILE_VS_3_0:
			return aSHADER_PROFILE_PS_3_0;
		case aSHADER_PROFILE_VS_4_0:
			return aSHADER_PROFILE_PS_4_0;
		case aSHADER_PROFILE_VS_5_0:
			return aSHADER_PROFILE_PS_5_0;
		case aSHADER_PROFILE_VS_GLSL:
			return aSHADER_PROFILE_PS_GLSL;
		};

		//default
		return aSHADER_PROFILE_PS_1_1;
	}

	void GetAutoEffectParameterName(const int8 *parameterName, int8 *newNameOut, uint32 &indexOut, bool &isPartOfBuffer)
	{
		if (!parameterName)
			return;
        
        isPartOfBuffer = false;
		indexOut = 0;
		const uint32 stringLen = strlen(parameterName);
		int32 newNameIndex = 0;
		int8 buffer[256];
		int32 bufferIndex = 0;

        /*
            First check for an index at the very end of the param.
            If an index exists then its starting point is found and
            used as the end of the auto name and the start of the index.
        */
        int32 firstNumPos = stringLen;
        if (stringLen > 0)
        {   
            int32 nextChar = stringLen-1;
            while ((nextChar > -1) && (isdigit(parameterName[nextChar])))
            {
                --firstNumPos;
                --nextChar;
            } 
        }
        
        bool arrayOpen = false;
        //get the auto name
		for(int32 i = 0; i < firstNumPos; ++i)
		{
			int8 c = parameterName[i];
            if (c == '[')
            {
                arrayOpen = true;
            }
            else if (c == ']')
            {
                //TODO : If array data is not needed then this loop can quit here.
                arrayOpen = false;
                continue;
            }
            else if (c == '.')
            {
                //stops reading the name at this point
                isPartOfBuffer = true;
                break;
            }
            else if (!arrayOpen)
            {
                newNameOut[newNameIndex++] = c;
            }
		}
        
        //get the auto index
        for(int32 i = firstNumPos; i < stringLen; ++i)
        {
            buffer[bufferIndex++] = parameterName[i];
        }

		if (bufferIndex > 0)
		{
            buffer[bufferIndex] = 0;//null terminator
			indexOut = atoi(buffer);
		}

		//add null terminator
		newNameOut[newNameIndex] = 0;
	}

	bool IsOpenGLInputAttribute(const int8 *name, eVERTEX_DECLUSAGE &usageOut, uint32 &usageIndexOut)
	{
		if (!name)
			return false;

		usageIndexOut = 0;

		const uint32 nameBufferSize = 256;
		int8 nameBuffer[nameBufferSize];
		strncpy(nameBuffer, name, nameBufferSize);

		uint32 it = 0;
		while(nameBuffer[it] != '\0')
		{
			nameBuffer[it] = tolower(nameBuffer[it]);
			++it;
		}

		for(uint32 i = 0; i < aVERTEX_DECLUSAGE_COUNT; ++i)
		{
			int8 *strPos = strstr(nameBuffer, g_pOpenGLAttributeNames[i]);
			if (strPos)
			{
				//is there a usage number supplied?
				const uint32 keyWordLen = strlen(g_pOpenGLAttributeNames[i]);
				const uint32 paramNameLen = strlen(nameBuffer);
				const uint32 strLenDifference = paramNameLen - keyWordLen;

				if (strLenDifference > 0)
				{
					const uint32 numberBufferSize = 5;
					int8 numberBuffer[numberBufferSize];
					memset(numberBuffer, 0, numberBufferSize);
					uint32 numIndex = 0;
					uint32 strIndex = 0;
					for(; (strIndex < strLenDifference) && (numIndex < numberBufferSize); ++numIndex, ++strIndex)
					{
						if (isdigit(nameBuffer[numIndex + keyWordLen]))
							numberBuffer[numIndex] = nameBuffer[numIndex + keyWordLen];
					}

					numberBuffer[numIndex] = 0;
					usageIndexOut = atoi(numberBuffer);
				}

				usageOut = (eVERTEX_DECLUSAGE)i;
				return true;
			}
		}

		return false;
	}

	const int8* GetGLSLVersionAsString(eMASH_OPENGL_VERSION version)
	{
		switch(version)
		{
		case aOGLVERSION_2_1:
			return "#version 120";
        case aOGLVERSION_3_0:
            return "#version 130";
        case aOGLVERSION_3_1:
            return "#version 140";
        case aOGLVERSION_3_2:
            return "#version 150";
		case aOGLVERSION_3_3:
			return "#version 330";
		};

		return 0;
	}
    
    int8* NumberToString(int8 *buffer, size_t size, int32 num)
    {
        PrintToBuffer(buffer, size, "%d", num);
        return buffer;
    }
    
    int8* FloatToString(int8 *buffer, size_t size, f32 num)
    {
        PrintToBuffer(buffer, size, "%f", num);
        return buffer;
    }

	uint32 GetVertexTypeSize(mash::eVERTEX_DECLTYPE type)
	{
		switch(type)
		{
		case mash::aDECLTYPE_R32_FLOAT:
			{
				return sizeof(f32);
			}
		case mash::aDECLTYPE_R32G32_FLOAT:
			{
				return sizeof(f32) * 2;
			}
		case mash::aDECLTYPE_R32G32B32_FLOAT:
			{
				return sizeof(f32) * 3;
			}
		case mash::aDECLTYPE_R32G32B32A32_FLOAT:
			{
				return sizeof(f32) * 4;
			}
		case mash::aDECLTYPE_R8G8B8A8_UNORM:
			{
				return sizeof(int32);
			}
		case mash::aDECLTYPE_R8G8B8A8_UINT:
			{
				return sizeof(uint8) * 4;
			}
		case mash::aDECLTYPE_R16G16_SINT:
			{
				return sizeof(int16) * 2;
			}
		case mash::aDECLTYPE_R16G16B16A16_SINT:
			{
				return sizeof(int16) * 4;
			}
		default:

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Unable to determine vertex declaration type.", 
						"GetVertexTypeSize");
		};

		return 0;
	}

	uint32 GetFormatSize(eFORMAT format)
	{
		switch(format)
		{
		case mash::aFORMAT_RGBA8_UINT:
			{
				return sizeof(uint8) * 4;
			}
		case mash::aFORMAT_RGBA16_UINT:
			{
				return sizeof(uint16) * 4;
			}
		case mash::aFORMAT_RGBA8_SINT:
			{
				return sizeof(int8) * 4;
			}
		case mash::aFORMAT_RGBA16_SINT:
			{
				return sizeof(int16) * 4;
			}
		case mash::aFORMAT_RGBA16_FLOAT:
			{
				return sizeof(int16) * 4;
			}
		case mash::aFORMAT_RGBA32_FLOAT:
			{
				return sizeof(int32) * 4;
			}
		case mash::aFORMAT_R8_UINT:
			{
				return sizeof(uint8);
			}
		case mash::aFORMAT_R16_UINT:
			{
				return sizeof(uint16);
			}
		case mash::aFORMAT_R32_UINT:
			{
				return sizeof(uint32);
			}
		case mash::aFORMAT_R16_FLOAT:
			{
				return sizeof(int16);
			}
		case mash::aFORMAT_R32_FLOAT:
			{
				return sizeof(int32);
			}
		case mash::aFORMAT_DEPTH32_FLOAT:
			{
				return sizeof(int32);
			}
		default:
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Unable to determine format type.", 
						"GetFormatSize");
		};

		return 0;
	}

	uint32 GetPrimitiveCount(ePRIMITIVE_TYPE primitiveType, uint32 indexCount)
	{
		if (indexCount == 0)
			return 0;

		switch(primitiveType)
		{
		case aPRIMITIVE_LINE_LIST:
			return indexCount / 2;
		case aPRIMITIVE_POINT_LIST:
			return indexCount;
		case aPRIMITIVE_TRIANGLE_LIST:
			return indexCount / 3;
		case aPRIMITIVE_LINE_STRIP:
			return indexCount - 1;
		case aPRIMITIVE_TRIANGLE_STRIP:
			return math::Max<uint32>(0, indexCount - 2);
		}

		return 0;
	}
    }
}
