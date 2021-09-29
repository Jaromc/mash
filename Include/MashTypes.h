//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_OBJECTS_H_
#define _MASH_OBJECTS_H_

#include "MashDataTypes.h"
#include "MashString.h"
#include "MashEnum.h"
#include "MashVector3.h"
#include "MashVector2.h"
#include "MashVector4.h"
#include "MashMathHelper.h"
#include <cstring>

namespace mash
{
	class MashSceneNode;
	class MashTexture;
	class MashTextureState;
	class MashAnimationMixer;

	/*
		This item may be returned by some functions as a default
		in the case of bad params or other circumstances.
	*/
	static MashStringc g_staticDefaultString;

	struct sTexture
	{
		MashTexture *texture;
		MashTextureState *state;

		sTexture():texture(0), state(0){}
	};

	struct sLoadSceneSettings
	{
        /*!
            Sets the animation frame rate of any transform animations. This is only valid
            when loading COLLADA files.
        */
		uint32 frameRate;
        
        /*!
            If set to true then a dummy node will be created that holds all loaded
            nodes as its children.
        */
		bool createRootNode;
        
        /*!
            Sets what data will be saved within the mesh when MashMesh::DeleteInitialiseData()
            is called. These flags can be fund in eSAVE_MESH_DATA_FLAGS.
            This can also be set in MashMesh::SetSaveInitialiseDataFlags() after loading.
        */
		uint32 saveGeometryFlags;
        
        /*!
            Initialise mesh data is used for some mesh manipulation functions and material
            changes on initialization. Setting this to true will call MashMesh::DeleteInitialiseData()
            as soon as loading is done with it. This can be handy to reduce memory footprints with large 
            scene while loading. If loading from .nss files then you probably have already set the scene
            up how you want it so it would be safe to set this to true.
         
            Note if loading is being done within MashGameLoop::Initialise() then MashMesh::DeleteInitialiseData()
            will automatically be called when that function exits.
         
            saveGeometryFlags Can be set to decide what data is deleted when MashMesh::DeleteInitialiseData()
            is called.
        */
        bool deleteMeshInitialiseDataOnLoad;

		sLoadSceneSettings():frameRate(30), 
			createRootNode(true),
			saveGeometryFlags(0),
            deleteMeshInitialiseDataOnLoad(false){}
	};

	struct sSaveSceneSettings
	{
		int32 notUsed;

		sSaveSceneSettings():notUsed(0){}
	};

	struct sJoystickThreshold
	{
		f32 axis1;
		f32 axis2;
		f32 throttle1;
		f32 throttle2;
	};

	struct sControllerSensitivity
	{
		f32 mouseAxisX;
		f32 mouseAxisY;

		f32 joyAxis1X;
		f32 joyAxis1Y;
		f32 joyAxis2X;
		f32 joyAxis2Y;
		f32 throttle1;
		f32 throttle2;

		sControllerSensitivity():mouseAxisX(1.0f),
			mouseAxisY(1.0f),
			joyAxis1X(1.0f),
			joyAxis1Y(1.0f),
			joyAxis2X(1.0f),
			joyAxis2Y(1.0f),
			throttle1(1.0f),
			throttle2(1.0f){}
	};

	struct sAnimationClip
	{
		MashStringc name;
		uint32 start;
		uint32 end;
	};

	struct sInputValueData
	{
		union
		{
			int32 ivalue;
			f32 fvalue;
		};
	};

	struct sVertexStreamInit
	{
        /*!
            Data to initialise the buffer with. This may be NULL to initialise
            an empty buffer. dataSizeInBytes then becomes the size of the empty
            buffer.
        */
		const void *data;
        
        //! Data array size in bytes.
		uint32 dataSizeInBytes;
        
        //! Buffer usage.
		eUSAGE usage;
	};

	struct sTriangleRecord
	{
		uint32 adjacencyEdgeList[3];

		sTriangleRecord()
		{
			adjacencyEdgeList[0] = 0xffffffff;
			adjacencyEdgeList[1] = 0xffffffff;
			adjacencyEdgeList[2] = 0xffffffff;
		}
	};

	struct sTriangleSkinnngRecord
	{
		MashVector4 boneIndices[3];
		MashVector4 boneWeights[3];

		sTriangleSkinnngRecord()
		{
			memset(boneIndices, 0, sizeof(MashVector4) * 3);
			memset(boneWeights, 0, sizeof(MashVector4) * 3);
		}
	};

	struct sTriPickResult
	{
		sTriPickResult():triangleIndex(0xffffffff), bufferIndex(0xffffffff), 
        collision(false),node(0),distance(mash::math::MaxFloat()){}

		uint32 triangleIndex;
		uint32 bufferIndex;

		f32 u, v, w;
		f32 distance;
		MashSceneNode *node;
		bool collision;
	};

	struct sIntersectingTriangleResult
	{
		sIntersectingTriangleResult():triangleIndex(0xffffffff), bufferIndex(0xffffffff){}

		uint32 triangleIndex;
		uint32 bufferIndex;
	};

	struct sMashViewPort
	{
		int32 x;
		int32 y;
		int32 width;
		int32 height;
		f32 minZ;
		f32 maxZ;
	};

	struct sEffectMacro
	{
		MashStringc name;
		MashStringc definition;

		sEffectMacro(){}
		sEffectMacro(const MashStringc &_name, const MashStringc &_definition):name(_name), definition(_definition){}
	};

	struct sEffectCompileArgs
	{
		MashStringc overrideLightShadingFile;
		eLIGHTING_TYPE lightingType;
		bool isShadowEffect;
		eLIGHTTYPE shadowEffectType;
		const sEffectMacro *macros;
		uint32 macroCount;

		sEffectCompileArgs():lightingType(aLIGHT_TYPE_NONE),
			isShadowEffect(false),
			shadowEffectType(aLIGHT_TYPE_COUNT),
			macros(0),
			macroCount(0){}
	};

	struct sRasteriserStates
	{
		eFILL_MODE fillMode;
		eCULL_MODE cullMode;
		bool depthTestingEnable;
		bool depthWritingEnabled;
		eDEPTH_COMPARISON depthComparison;
		f32 depthBias;
		f32 depthBiasClamp;
		f32 slopeScaledDepthBias;

		sRasteriserStates():fillMode(aFILL_SOLID),
		cullMode(aCULL_CCW),
		depthTestingEnable(true),
		depthWritingEnabled(true),
		depthComparison(aZCMP_LESS_EQUAL),
		depthBias(0.0f),
		depthBiasClamp(0.0f),
		slopeScaledDepthBias(0){}

		bool operator==(const sRasteriserStates &other)const
		{
			return (fillMode == other.fillMode && 
				cullMode == other.cullMode && 
				depthTestingEnable == other.depthTestingEnable &&
				depthWritingEnabled == other.depthWritingEnabled &&
				depthComparison == other.depthComparison &&
				depthBias == other.depthBias && 
				depthBiasClamp == other.depthBiasClamp &&
				slopeScaledDepthBias == other.slopeScaledDepthBias);
		}
	};

	struct sBlendStates
	{
		/*
			colourWriteMask is valid even if blendingEnabled is false.
		*/
		bool blendingEnabled;
		eBLEND srcBlend;
		eBLEND destBlend;
		eBLENDOP blendOp;
		eBLEND srcBlendAlpha;
		eBLEND destBlendAlpha;
		eBLENDOP blendOpAlpha;
		eCOLOUR_WRITE colourWriteMask;

		bool operator==(const sBlendStates &other)const
		{
			return (blendingEnabled == other.blendingEnabled &&
				srcBlend == other.srcBlend &&
				destBlend == other.destBlend &&
				blendOp == other.blendOp &&
				srcBlendAlpha == other.srcBlendAlpha &&
				destBlendAlpha == other.destBlendAlpha &&
				blendOpAlpha == other.blendOpAlpha && 
				colourWriteMask == other.colourWriteMask);
		}

		sBlendStates():blendingEnabled(false),
			srcBlend(aBLEND_ONE),
			destBlend(aBLEND_ZERO),
			blendOp(aBLENDOP_ADD),
			srcBlendAlpha(aBLEND_ONE),
			destBlendAlpha(aBLEND_ZERO),
			blendOpAlpha(aBLENDOP_ADD),
			colourWriteMask(aCOLOUR_WRITE_ALL){}
	};

	struct sSamplerState
	{
		eSAMPLER type;//TODO : Remove type. Its not needed.
		eFILTER filter;
		eTEXTURE_ADDRESS uMode;
		eTEXTURE_ADDRESS vMode;
		int32 maxAnistropy;

		sSamplerState():maxAnistropy(1), filter(aFILTER_MIN_MAG_MIP_LINEAR),
			uMode(aTEXTURE_ADDRESS_CLAMP), vMode(aTEXTURE_ADDRESS_CLAMP){}

		bool operator==(const sSamplerState &other)const
		{
			return (type == other.type &&
				filter == other.filter &&
				uMode == other.uMode &&
				vMode == other.vMode &&
				maxAnistropy == other.maxAnistropy);
		}
	};	

	//Do not change the order of this structure!
	struct sMashVertexElement
	{
        /*!
            Stream 0 is known as the geoemtry stream. This is where all the mesh
            data is held. Other streams can be used for instance data or any other
            custom data to be accessed from the GPU.
        */
		int32 stream;
        
        //!This elements position in the vertex array in bytes.
		uint32 stride;
        
        //! Element type.
		eVERTEX_DECLTYPE type;
        
        //! Element usage.
		eVERTEX_DECLUSAGE usage;
        
        /*!
            If there are multiple eVERTEX_DECLUSAGE of the same type in a stream
            then this is the ith element of that type starting from 0. This number
            resets back to 0 for different streams.
        */
		int32 usageIndex;

        /*!
            Stream 0 is almost always aCLASSIFICATION_VERTEX_DATA. If there is instance
            data held in other streams then set this to aCLASSIFICATION_INSTANCE_DATA.
        */
		eVERTEX_CLASSIFICATION classification;
        
        /*!
            Only valid if the classification is aCLASSIFICATION_INSTANCE_DATA.
            Sets the number of instances to render before moving forward.
            Set this to 1 for instance data otherwise leave it set to 0.
        */
		uint32 instanceStepRate;

        //! Equals to.
		bool operator==(const sMashVertexElement &other)const
		{
			if ((stream == other.stream) &&
				(stride == other.stride) &&
				(type == other.type) &&
				(usage == other.usage) &&
				(usageIndex == other.usageIndex))
			{
				return true;
			}

			return false;
		}
	};

#ifdef _MASH_ABGR_COLOUR_FORMAT_

	struct sMashColour
	{
		uint32 colour;
		
		/*!
			Values should be in the range of 0 - 255
		*/
		sMashColour(uint32 r, uint32 g, uint32 b, uint32 a):
		colour((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)){}

			sMashColour():colour(0xFFFFFFFF){}

			sMashColour(uint32 c):colour(c){}

		void SetRed(uint32 r)
		{
			colour = ((r)&0xff) | (colour & 0xffffff00);
		}

		void SetBlue(uint32 b)
		{
			colour = (((b)&0xff)<<16) | (colour & 0xff00ffff);
		}

		void SetGreen(uint32 g)
		{
			colour = (((g)&0xff)<<8) | (colour & 0xffff00ff);
		}

		void SetAlpha(uint32 a)
		{
			colour = (((a)&0xff)<<24) | (colour & 0x00ffffff);
		}

		int32 GetRed()const
		{
			return colour & 0xff;
		}

		int32 GetBlue()const
		{
			return (colour >> 16) & 0xff;
		}

		int32 GetGreen()const
		{
			return (colour >> 8) & 0xff;
		}

		int32 GetAlpha()const
		{
			return colour >> 24;
		}

		bool operator==(const sMashColour &other)const
		{
			return (colour == other.colour);
		}
	};
#else
    struct sMashColour
	{
		uint32 colour;
		
		/*!
			Values should be in the range of 0 - 255
		*/
		sMashColour(uint32 r, uint32 g, uint32 b, uint32 a):
		colour((((r)&0xff)<<24)|(((g)&0xff)<<16)|(((b)&0xff)<<8)|((a)&0xff)){}

			sMashColour():colour(0xFFFFFFFF){}

			sMashColour(uint32 c):colour(c){}

		void SetRed(uint32 r)
		{
			colour = (((r)&0xff)<<24) | (colour & 0x00ffffff);
		}

		void SetBlue(uint32 b)
		{
			colour = (((b)&0xff)<<8) | (colour & 0xffff00ff);
		}

		void SetGreen(uint32 g)
		{
			colour = (((g)&0xff)<<16) | (colour & 0xff00ffff);
		}

		void SetAlpha(uint32 a)
		{
			colour = ((a)&0xff) | (colour & 0xffffff00);
		}

		int32 GetRed()const
		{
			return colour >> 24;
		}

		int32 GetBlue()const
		{
			return (colour >> 8) & 0xff;
		}

		int32 GetGreen()const
		{
			return (colour >> 16) & 0xff;
		}

		int32 GetAlpha()const
		{
			return colour & 0xff;
		}

		bool operator==(const sMashColour &other)const
		{
			return (colour == other.colour);
		}
	};
#endif
	//colours should be in the range of 0 - 1
	struct sMashColour4
	{
		union
		{
			struct
			{
				f32 r, g, b, a;
			};

			f32 v[4];
		};
        
		sMashColour4():r(0.0f), g(0.0f), b(0.0f),a(0.0f){}

		sMashColour4(f32 red, f32 green, f32 blue, f32 alpha)
		{				
			r = red;
			g = green;
			b = blue;
			a = alpha;
		}

		sMashColour4(const sMashColour4 &colour)
		{
			r = colour.r;
			g = colour.g;
			b = colour.b;
			a = colour.a;
		}

		sMashColour4(const sMashColour &colour)
		{
			const f32 inv = 1.0f / 255.0f;
			r = colour.GetRed() * inv;
			g = colour.GetGreen() * inv;
			b = colour.GetBlue() * inv;
			a = colour.GetAlpha() * inv;
		}

		void Set(f32 red, f32 green, f32 blue, f32 alpha)
		{				
			r = red;
			g = green;
			b = blue;
			a = alpha;
		}

		sMashColour4 Lerp(const sMashColour4 &to, f32 alpha)
		{
			sMashColour4 c;
			c.r = math::Lerp(r, to.r, alpha);
			c.g = math::Lerp(g, to.g, alpha);
			c.b = math::Lerp(b, to.b, alpha);
			c.a = math::Lerp(a, to.a, alpha);

			return c;
		}

		sMashColour ToColour()const
		{
			return sMashColour((uint32)(r * 255.0f),
				(uint32)(g * 255.0f),
				(uint32)(b * 255.0f),
				(uint32)(a * 255.0f));
		}

		sMashColour4 operator*(f32 f)const
		{
			return sMashColour4(r*f, g*f,b*f,a*f);
		}

		sMashColour4 operator/(f32 f)const
		{
			if (f == 0.0f)
				return sMashColour4(0.0f,0.0f,0.f,0.0f);

			f32 inv = 1.0f / f;
			return sMashColour4((r * inv), (g * inv), (b * inv), (a * inv));
		}

		bool operator==(const sMashColour4 &other)const
		{
			return ((a == other.a) && (b == other.b) && (g == other.g) && (r == other.r));
		}

		bool operator!=(const sMashColour4 &other)const
		{
			return ((a != other.a) || (b != other.b) || (g != other.g) || (r != other.r));
		}

		sMashColour4 operator*(const sMashColour4 &other)const
		{
			return sMashColour4((r * other.r), (g * other.g), (b * other.b), (a * other.a));
		}

		sMashColour4 operator-(const sMashColour4 &other)const
		{
			return sMashColour4((r - other.r),(g - other.g), (b - other.b), (a - other.a));
		}

		sMashColour4 operator+(const sMashColour4 &other)const
		{
			return sMashColour4((r + other.r), (g + other.g), (b + other.b), (a + other.a));
		}

		sMashColour4& operator*=(const sMashColour4 &other)
		{				
			b *= other.b;
			g *= other.g;
			r *= other.r;
			a *= other.a;
			return *this;
		}

		sMashColour4& operator-=(const sMashColour4 &other)
		{				
			b -= other.b;
			g -= other.g;
			r -= other.r;
			a -= other.a;
			return *this;
		}

		sMashColour4& operator+=(const sMashColour4 &other)
		{				
			b += other.b;
			g += other.g;
			r += other.r;
			a += other.a;
			return *this;
		}
	};

	/*
		IMPORTANT information for filling out these values manually.
		- outerCone = cos(angleInRadians)
		- innerCone = cos(angleInRadians)
	*/
	struct sMashLight
	{
		sMashColour4 ambient;
		sMashColour4 diffuse;
		sMashColour4 specular;
		MashVector3 position;
		f32 __pad1;
		/*
			The viewSpacePosition will only be valid if the
			light node has not been culled from the current scene.
		*/
		MashVector3 viewSpacePosition;
		f32 __pad2;
		MashVector3 direction;
		f32 __pad3;
		MashVector3 viewSpaceDirection;
		f32 __pad4;

		//point and spot light values
		MashVector3 atten; //xyz = 0, 1, 2
		f32 __pad5;
		f32 range;
		//spot light values
		f32 outerCone;
		f32 innerCone;
		f32 falloff;
	};

	struct sParticleSettings
	{
		uint32 maxParticleCount;
		MashVector3 gravity;
		uint32 particlesPerSecond;

		/*
			Modifies how much of the emitters velocity is
			added to each particles velocity. This number should be in the
			range of 0.0 - 1.0.
		*/
		f32 emitterVelocityWeight;

		MashVector3 minVelocity;
		MashVector3 maxVelocity;

		f32 minDuration;
		f32 maxDuration;
		
		f32 minStartSize;
		f32 maxStartSize;
		f32 minEndSize;
		f32 maxEndSize;
		
		f32 minRotateSpeed;
		f32 maxRotateSpeed;

		/*
			The larger the clipping distance, the higher this number needs to be.
			Eg, far - near == 10000, softParticleScale >= 100
		*/
		f32 softParticleScale;
		/*
			advances the particle system to the given time when the system
			is first created.
		*/
		uint32 startTime;

		sMashColour4 minStartColour;
		sMashColour4 maxStartColour;
		sMashColour4 minEndColour;
		sMashColour4 maxEndColour;

		sParticleSettings()
		{
			maxParticleCount = 0;
			gravity = MashVector3(0.0f, -9.8f, 0.0f);
			particlesPerSecond = 0;
			minVelocity = MashVector3(-2.0f, 5.0f, -2.0f);
			maxVelocity = MashVector3(2.0f, 10.0f, 2.0f);
			minStartColour = sMashColour4(1.0f, 1.0f, 1.0f, 1.0f);
			maxStartColour = sMashColour4(1.0f, 1.0f, 1.0f, 1.0f);
			minEndColour = sMashColour4(1.0f, 1.0f, 1.0f, 0.0f);
			maxEndColour = sMashColour4(1.0f, 1.0f, 1.0f, 0.0f);
			minDuration = 2.0f;
			maxDuration = 5.0f;
			minStartSize = 1.0f;
			maxStartSize = 1.0f;
			minEndSize = 1.0f;
			maxEndSize = 1.0f;
			minRotateSpeed = 0.0f;
			maxRotateSpeed = 0.0f;
			softParticleScale = 10.0f;
			startTime = 0.0f;
			emitterVelocityWeight = 0.0f;
		}

	};

	class MashVertexPosTex
	{
	public:
		MashVertexPosTex()
		{
			//create position element
			m_elements[0].stream = 0;
			m_elements[0].stride = 0;
			m_elements[0].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[0].usage = aDECLUSAGE_POSITION;
			m_elements[0].usageIndex = 0;
			m_elements[0].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[0].instanceStepRate = 0;
			//create texture element
			m_elements[1].stream = 0;
			m_elements[1].stride = 12;
			m_elements[1].type = aDECLTYPE_R32G32_FLOAT;
			m_elements[1].usage = aDECLUSAGE_TEXCOORD;
			m_elements[1].usageIndex = 0;
			m_elements[1].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[1].instanceStepRate = 0;
		};

		struct sMashVertexPosTex
		{
			sMashVertexPosTex(){}
			sMashVertexPosTex(const MashVector3 &vPosition,
				const MashVector2 &vTex = MashVector2(0.0f,0.0f))/*:sMashVertex(vPosition)*/
			{
				position.x = vPosition.x;
				position.y = vPosition.y;
				position.z = vPosition.z;
				
				texCoord.x = vTex.x;
				texCoord.y = vTex.y;
			}

			MashVector3 position;
			MashVector2 texCoord;

			bool operator==(const sMashVertexPosTex &other)const
			{
				return ((position == other.position) &&
						(texCoord == other.texCoord));
			}
		};

		sMashVertexElement m_elements[2];
	};

	class MashVertexStandard
	{
	public:
		MashVertexStandard()
		{
			//create position element
			m_elements[0].stream = 0;
			m_elements[0].stride = 0;
			m_elements[0].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[0].usage = aDECLUSAGE_POSITION;
			m_elements[0].usageIndex = 0;
			m_elements[0].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[0].instanceStepRate = 0;
			//create normal element
			m_elements[1].stream = 0;
			m_elements[1].stride = 12;
			m_elements[1].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[1].usage = aDECLUSAGE_NORMAL;
			m_elements[1].usageIndex = 0;
			m_elements[1].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[1].instanceStepRate = 0;
			//create texture element
			m_elements[2].stream = 0;
			m_elements[2].stride = 24;
			m_elements[2].type = aDECLTYPE_R32G32_FLOAT;
			m_elements[2].usage = aDECLUSAGE_TEXCOORD;
			m_elements[2].usageIndex = 0;
			m_elements[2].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[2].instanceStepRate = 0;
		};

		struct sMashVertexStandard
		{
			sMashVertexStandard(){}
			sMashVertexStandard(const MashVector3 &vPosition,
				const MashVector3 &vNormal = MashVector3(0.0f,0.0f,0.0f),
				const MashVector2 &vTex = MashVector2(0.0f,0.0f))
			{
				position.x = vPosition.x;
				position.y = vPosition.y;
				position.z = vPosition.z;

				normal.x = vNormal.x;
				normal.y = vNormal.y;
				normal.z = vNormal.z;
				
				texCoord.x = vTex.x;
				texCoord.y = vTex.y;
			}

			MashVector3 position;
			MashVector3 normal;
			MashVector2 texCoord;

			bool operator==(const sMashVertexStandard &other)const
			{
				return ((position == other.position) &&
						(normal == other.normal) &&
						(texCoord == other.texCoord));
			}
		};

		sMashVertexElement m_elements[3];
	};

	class MashVertexMeshConversion
	{
	public:
		MashVertexMeshConversion()
		{
			//create position element
			m_elements[0].stream = 0;
			m_elements[0].stride = 0;
			m_elements[0].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[0].usage = aDECLUSAGE_POSITION;
			m_elements[0].usageIndex = 0;
			m_elements[0].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[0].instanceStepRate = 0;
			//create normal element
			m_elements[1].stream = 0;
			m_elements[1].stride = 12;
			m_elements[1].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[1].usage = aDECLUSAGE_NORMAL;
			m_elements[1].usageIndex = 0;
			m_elements[1].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[1].instanceStepRate = 0;
			//create tangent element
			m_elements[2].stream = 0;
			m_elements[2].stride = 24;
			m_elements[2].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[2].usage = aDECLUSAGE_TANGENT;
			m_elements[2].usageIndex = 0;
			m_elements[2].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[2].instanceStepRate = 0;
			//create texture element
			m_elements[3].stream = 0;
			m_elements[3].stride = 36;
			m_elements[3].type = aDECLTYPE_R32G32_FLOAT;
			m_elements[3].usage = aDECLUSAGE_TEXCOORD;
			m_elements[3].usageIndex = 0;
			m_elements[3].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[3].instanceStepRate = 0;
			//create colour element
			m_elements[4].stream = 0;
			m_elements[4].stride = 44;
			m_elements[4].type = aDECLTYPE_R8G8B8A8_UNORM;
			m_elements[4].usage = aDECLUSAGE_COLOUR;
			m_elements[4].usageIndex = 0;
			m_elements[4].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[4].instanceStepRate = 0;
		};

		struct sMashVertexMeshConversion
		{
			sMashVertexMeshConversion(){}
			sMashVertexMeshConversion(const MashVector3 &vPosition,
				const MashVector3 &vNormal = MashVector3(0.0f,0.0f,0.0f),
				const MashVector3 &vTangent = MashVector3(0.0f,0.0f,0.0f),
				const MashVector2 &vTex = MashVector2(0.0f,0.0f),
				const sMashColour &col = sMashColour(255, 255, 255, 255))
			{
				position.x = vPosition.x;
				position.y = vPosition.y;
				position.z = vPosition.z;

				normal.x = vNormal.x;
				normal.y = vNormal.y;
				normal.z = vNormal.z;

				tangent.x = vTangent.x;
				tangent.y = vTangent.y;
				tangent.z = vTangent.z;
				
				texCoord.x = vTex.x;
				texCoord.y = vTex.y;

				colour = col;
			}

			MashVector3 position;
			MashVector3 normal;
			MashVector3 tangent;
			MashVector2 texCoord;
			sMashColour colour;

			bool operator==(const sMashVertexMeshConversion &other)const
			{
				return ((position == other.position) &&
						(normal == other.normal) &&
						(tangent == other.tangent) &&
						(texCoord == other.texCoord) &&
						(colour == other.colour));
			}
		};

		sMashVertexElement m_elements[5];
	};


	class MashVertexColour
	{
	public:
		MashVertexColour()
		{
			//create position element
			m_elements[0].stream = 0;
			m_elements[0].stride = 0;
			m_elements[0].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[0].usage = aDECLUSAGE_POSITION;
			m_elements[0].usageIndex = 0;
			m_elements[0].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[0].instanceStepRate = 0;
			//create colour element
			m_elements[1].stream = 0;
			m_elements[1].stride = 12;
			m_elements[1].type = aDECLTYPE_R8G8B8A8_UNORM;//aDECLTYPE_R32_UINT;
			m_elements[1].usage = aDECLUSAGE_COLOUR;
			m_elements[1].usageIndex = 0;
			m_elements[1].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[1].instanceStepRate = 0;
		};

		struct sMashVertexColour
		{
			sMashVertexColour(){}
			sMashVertexColour(const MashVector3 &vPosition,
				const sMashColour &col = sMashColour(255, 255, 255, 255))
			{
				position.x = vPosition.x;
				position.y = vPosition.y;
				position.z = vPosition.z;

				colour = col;
			}

			MashVector3 position;
			sMashColour colour;

			bool operator==(const sMashVertexColour &other)const
			{
				return ((position == other.position) &&
						(colour == other.colour));
			}
		};

		sMashVertexElement m_elements[2];
	};

	class MashVertexGUI
	{
	public:
		MashVertexGUI()
		{
			//create position element
			m_elements[0].stream = 0;
			m_elements[0].stride = 0;
			m_elements[0].type = aDECLTYPE_R32G32B32_FLOAT;
			m_elements[0].usage = aDECLUSAGE_POSITION;
			m_elements[0].usageIndex = 0;
			m_elements[0].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[0].instanceStepRate = 0;

			m_elements[1].stream = 0;
			m_elements[1].stride = 12;
			m_elements[1].type = aDECLTYPE_R32G32_FLOAT;
			m_elements[1].usage = aDECLUSAGE_TEXCOORD;
			m_elements[1].usageIndex = 0;
			m_elements[1].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[1].instanceStepRate = 0;

			m_elements[2].stream = 0;
			m_elements[2].stride = 20;
			m_elements[2].type = aDECLTYPE_R8G8B8A8_UNORM;
			m_elements[2].usage = aDECLUSAGE_COLOUR;
			m_elements[2].usageIndex = 0;
			m_elements[2].classification = aCLASSIFICATION_VERTEX_DATA;
			m_elements[2].instanceStepRate = 0;
		};

		struct sMashVertexGUI
		{
			sMashVertexGUI(){}
			sMashVertexGUI(const MashVector3 &vPosition,
				const MashVector2 &vBaseTex = MashVector2(-1.0f,-1.0f),
				sMashColour c = sMashColour(255,255,255,255))
			{
				position.x = vPosition.x;
				position.y = vPosition.y;
				position.z = vPosition.z;
				
				baseTexCoord.x = vBaseTex.x;
				baseTexCoord.y = vBaseTex.y;

				colour = c;
			}

			MashVector3 position;
			MashVector2 baseTexCoord;
			sMashColour colour;
		};

		sMashVertexElement m_elements[3];
	};
}

#endif