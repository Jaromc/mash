//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_HELPER_H_
#define _MASH_HELPER_H_

#include "MashEnum.h"

namespace mash
{
    class MashFileManager;
    struct sMashVertexElement;
    
	/*!
		Convenience and conversion methods. 
	*/
    namespace helpers
    {
		//! Returns true if the string in not NULL and contains more than 0 characters.
		bool IsValidString(const char *s);
        
        //! Sprintf style function.
        /*!
            \param buffer Memory to write to.
            \param size Memory size in bytes.
            \param stringToWrite String to write.
            \param ... string args.
        */
        void PrintToBuffer(int8 *buffer, size_t size, int8 *stringToWrite, ...);
        
        //! Sprintf style function.
        /*!
            \param buffer Memory to write to.
            \param size Memory size in bytes.
            \param num Number to convert to a string.
            \return pointer to buffer.
        */
        int8* NumberToString(int8 *buffer, size_t size, int32 num);
        int8* FloatToString(int8 *buffer, size_t size, f32 num);
        
        //! Returns the byte size of a vertex type.
        /*!
            \param type Vertex type.
            \return Type size in bytes.
        */
        uint32 GetVertexTypeSize(eVERTEX_DECLTYPE type);
        
        //! Returns the byte size of a format.
        /*!
            \param format Format type.
            \return Format size in bytes.
        */
        uint32 GetFormatSize(eFORMAT format);
        
        //! Calculates the primitive count from an index count.
        /*!
            Example, For two triangles, the index count passed in would be 6.
         
            \param primitive type Primitive type that is being used.
            \param indexCount The number of vertices.
        */
        uint32 GetPrimitiveCount(ePRIMITIVE_TYPE primitiveType, uint32 indexCount);
        
        //! Determines if a string is an openGL attrib.
        /*!
            Valid attribs are found in g_pOpenGLAttributeNames. This function will
            also return any number found at the end of the string.
         
            \param name Attrib name.
            \param usageOut Name string transformed into an enum.
            \param usageIndexOut Returns the number found at the end of the name.
            \return True if it's valid, false otherwise.
        */
        bool IsOpenGLInputAttribute(const int8 *name, eVERTEX_DECLUSAGE &usageOut, uint32 &usageIndexOut);
        
        //! Returns the string version used in GLSL files.
        /*!
            Example "#version 150".
         
            \param version Enum version.
            \return String.
        */
        const int8* GetGLSLVersionAsString(eMASH_OPENGL_VERSION version);
        
        //! Gets the pixel profile that matches the vertex profile.
        /*!
            \param profile Vertex profile to query.
            \return Pixel profile to match the vertex profile.
        */
        eSHADER_PROFILE GetPixelProfileFromVertexProfile(eSHADER_PROFILE profile);
        
        //! Gets the vertex decl element size in bytes.
        /*!
            Example, aDECLTYPE_R32G32_FLOAT will return 4 bytes.
         
            \param type Vertex decl type.
            \return Type size in bytes.
        */
        uint32 GetVertexDeclTypeElmSize(eVERTEX_DECLTYPE type);
        
        //! Gets the numbers of elements in the decl type.
        /*!
            Example, aDECLTYPE_R32G32_FLOAT will return 2.
         
            \param type Vertex decl type.
            \return Number of elements in the vertex decl.
        */
        uint32 GetVertexDeclTypeElmCount(eVERTEX_DECLTYPE type);
        
        //! Gets the total size of a vertex decl type in bytes.
        /*!
            Example, aDECLTYPE_R32G32_FLOAT will return 8 bytes.
         
            \param type Vertex decl type.
            \return Type size in bytes.
        */
        uint32 GetVertexDeclTypeSize(eVERTEX_DECLTYPE type);
        
        //! Gets a usage enum as a string.
        /*!
            \param usage Enum.
            \return Enum as string.
        */
        const char* GetVertexUsageAsString(eVERTEX_DECLUSAGE usage);
        
        //! Gets the indexes of a streams start and end points in an array.
        /*!
            \param stream Stream to qurey.
            \param vertexDecl Vertex array to search. It is assumed streams are ordered from smallest to largest stream.
            \param elementCount Number of elements in the array.
            \param streamStart Returns the index of the start of the stream.
            \param streamEnd Returns the index of the end of the stream.
        */
        void GetVertexStreamStartEndIndex(uint32 stream, const sMashVertexElement *vertexDecl, uint32 elementCount, uint32 &streamStart, uint32 &streamEnd);
        
        //! Gets the auto name minus any numbers.
        /*!
            An auto name is in the format of autoSomeParam0.
         
            If a number exists at the end of the auto name then it will be removed from the
            final name and the number will be returned seperatly.
         
            \param parameterName Raw auto param name.
            \param newNameOut Name minus any numbers if any.
            \param indexOut If an index was at the end of the string then its output here.
            \param isPartOfBuffer True if the name contains an '.' character.            
        */
        void GetAutoEffectParameterName(const int8 *parameterName, int8 *newNameOut, uint32 &indexOut, bool &isPartOfBuffer);
        
        //! Gets a string shader profile returned as an enum.
        /*!
            Example, ps_2_0, glslv, etc...
            
            \param profile String profile.
            \return Profile as an enum.
        */
        eSHADER_PROFILE GetShaderProfileFromString(const int8 *profile);
        
        //! Gets a shader profile as a string.
        /*!
            \param profile Shader profile.
            \return ps_2_0, glslv, etc...
        */
        const int8* GetShaderProfileString(eSHADER_PROFILE profile);
        
        //! Gets a program type from a profile.
        /*!
            \param Profile Shader profile.
            \return Program type for the profile.
        */
        ePROGRAM_TYPE GetEffectProgramTypeFromProfile(eSHADER_PROFILE profile);
        
        //! Gets the shader extension for a given api.
        /*!
            \param api API to query.
            \return hlsl or glsl.
        */
        const int8* GetShaderFileExtention(eSHADER_API_TYPE api);
        
        //! Gets the api that matches the shader profile.
        /*!
            \param profile Shader profile.
            \return Shader api to match the profile.
        */
        eSHADER_API_TYPE GetAPIFromShaderProfile(eSHADER_PROFILE profile);
        
        //! Returns true if the extention is a native hlsl or glsl extension.
        /*!
            \param fileName filename including the extention.
            \return True if its in native hlsl or glsl extention. False if its in custom engine format.
        */
        bool IsFileANativeEffectProgram(const int8 *fileName);
        
        //! Gets the file extention for the engines custom shaders.
        /*!
            \return Engines custom shader extention.
        */
        const int8* GetEffectProgramFileExtension();
        
        //! Matches a key to its controller.
        /*!
            \param controller Animation controller type.
            \param key Animation key type.
            \return Ok if the key is valid for the controller. Failed otherwise.
        */
        eMASH_STATUS ValidateAnimationKeyTypeToController(eMASH_CONTROLLER_TYPE controller, eANIM_KEY_TYPE key);

    //////////////////////////////////////////////////////////////
    //Test for fundamental types
    //////////////////////////////////////////////////////////////
        template<class T>
        struct MashIsFundamental
        {
            enum {value = 0};
        };

        template<>
        struct MashIsFundamental<int8>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<int16>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<int32>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<int64>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<uint8>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<uint16>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<uint32>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<uint64>
        {
            enum {value = 1};
        };

        template<>
        struct MashIsFundamental<bool>
        {
            enum {value = 1};
        };

    //////////////////////////////////////////////////////////////
    //Test specific data types
    //////////////////////////////////////////////////////////////
        template<class T>
        struct MashIsInt8
        {
            enum {value = 0};
        };

        template<>
        struct MashIsInt8<int8>
        {
            enum {value = 1};
        };

        template<class T>
        struct MashIsUInt8
        {
            enum {value = 0};
        };

        template<>
        struct MashIsUInt8<uint8>
        {
            enum {value = 1};
        };
    }
}

#endif