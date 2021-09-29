//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ENUM_H_
#define _MASH_ENUM_H_

#include "MashDataTypes.h"

namespace mash
{
    enum eDEPTH_BUFFER_OPTIONS
    {
        /*!
            This will create a new depth buffer for a render target.
        */
        aDEPTH_OPTION_OWN_DEPTH,
        /*!
            The render target will share the main depth buffer for depth testing/writing.
            For this option to work correctly the render target must be created with the
            same size as the backbuffer. Use -1 for render target dimensions.
        */
        aDEPTH_OPTION_SHARE_MAIN_DEPTH,
        /*!
            The render target will contain no depth buffer.
        */
        aDEPTH_OPTION_NO_DEPTH
    };
    
    enum eSHADER_EFFECT_INCLUDES
    {
        aEFF_INC_DIRECTIONAL_LIGHTING,
        aEFF_INC_SPOT_LIGHTING,
        aEFF_INC_POINT_LIGHTING,
        aEFF_INC_LIGHT_SHADING,
        
        aEFF_INC_DIRECTIONAL_SHADOW_CASTER_VERTEX,
        aEFF_INC_DIRECTIONAL_SHADOW_CASTER_PIXEL,
        aEFF_INC_SPOT_SHADOW_CASTER_VERTEX,
        aEFF_INC_SPOT_SHADOW_CASTER_PIXEL,
        aEFF_INC_POINT_SHADOW_CASTER_VERTEX,
        aEFF_INC_POINT_SHADOW_CASTER_PIXEL,
        
        aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER,
        aEFF_INC_SPOT_SHADOW_RECEIVER,
        aEFF_INC_POINT_SHADOW_RECEIVER,
        
        aEFF_INC_LIGHT_STRUCTURES,
        
        //Names with a postfix _g are runtime generated
        aEFF_INC_FORWARD_RENDERED_LIGHTING_G,
        aEFF_INC_DIRECTIONAL_DEFERRED_LIGHTING_G,
        aEFF_INC_SPOT_DEFERRED_LIGHTING_G,
        aEFF_INC_POINT_DEFERRED_LIGHTING_G,
        
        aEFF_INC_COUNT
    };
    
	enum eSAVE_MESH_DATA_FLAGS
	{
		aSAVE_MESH_BONES = 1,
		aSAVE_MESH_TRIANGLE_BUFFFER = 2,
		aSAVE_MESH_RAW_GEOMETRY = 4,

		aSAVE_MESH_DATA_ALL = 0xFFFFFFFF
	};

	enum eSAVE_TEXTURE_FORMAT
	{
		aSAVE_TEX_FORMAT_BMP,
		aSAVE_TEX_FORMAT_DDS
	};

	enum eANTIALIAS_TYPE
	{
		aANTIALIAS_TYPE_NONE,
		aANTIALIAS_TYPE_X2,
		aANTIALIAS_TYPE_X4,
		aANTIALIAS_TYPE_X8
	};

	enum eBACKBUFFER_FORMAT
	{
		aBACKBUFFER_FORMAT_16BIT,
        aBACKBUFFER_FORMAT_32BIT,
	};

	enum eDEPTH_FORMAT
	{
		aDEPTH_FORMAT_24BIT,
		aDEPTH_FORMAT_32BIT
	};

    enum eMASH_DEVICE_TYPE
    {
        aDEVICE_TYPE_WINDOWS,
        aDEVICE_TYPE_APPLE,
        aDEVICE_TYPE_LINUX
    };

	enum eMASH_OPENGL_VERSION
	{
		aOGLVERSION_2_1,
        aOGLVERSION_3_0,
        aOGLVERSION_3_1,
        aOGLVERSION_3_2,
		aOGLVERSION_3_3
	};

	enum ePARTICLE_TYPE
	{
		aPARTICLE_CPU,
		aPARTICLE_GPU,
		aPARTICLE_MESH,
		aPARTICLE_CPU_SOFT_DEFERRED,
		aPARTICLE_GPU_SOFT_DEFERRED
	};

	enum ePARTICLE_EMITTER_TYPES
	{
		aPARTICLE_EMITTER_POINT
	};

	enum eDECAL_TYPE
	{
		aDECAL_STANDARD
	};

	enum eTRIANGLE_COLLIDER_TYPE
	{
		/*!
			Standard colliders simply search through all triangles until a collision is found. These
			are best use for simple meshes (triangle count < 10).
		*/
		aTRIANGLE_COLLIDER_STANDARD,
		/*!
			Colliders that use kd-trees are best used for meshes with large numbers of triangles.
			All the triangles are sorted into a tree structure based on their position. This makes
			searching much faster while reducing the number of collision tests.
		*/
		aTRIANGLE_COLLIDER_KD_TREE,
		/*!
			Lets the engine decide what collider to use based on the triangle count of the mesh. 
		*/
		aTRIANGLE_COLLIDER_AUTO
	};
    
	//! Animamation blending modes.
	enum eANIMATION_BLEND_MODE
	{
		/*!
			Blends two or more animations together
			using a weighted value between 0 - 1.
		*/
		aBLEND_BLEND,
		/*!
			Adds two or more animations together using the difference
			between the current frame and the bind pose. 
			This disregards all weighting values.
			Be sure to have at least one BLEND animation playing
			or else the result will probably be undesirable.
		*/
		aBLEND_ADDITIVE
	};

	//! Wrap mode for animation playback.
	enum eANIMATION_WRAP_MODE
	{
		/*!
			Plays the animation once then rewinds and
			disables the animation.
		*/
		aWRAP_PLAYONCE,
		/*!
			Plays the animation once but doesnt disable
			the animation when the end is reached. Handy 
			for procedural animation.
		*/
		aWRAP_CLAMP,
		/*!
			Loops the animation.
		*/
		aWRAP_LOOP,
		/*!
			Animation bounces from start to end.
		*/
		aWRAP_BOUNCE
	};

	enum eMATERIAL_COMPILER_FLAGS
	{
        //! Compiles nothing. A valid flag is needed.
		aMATERIAL_COMPILER_NOTHING = 0,
        //! Compiles forward rendered techniques.
		aMATERIAL_COMPILER_FORWARD_RENDERED = 1,
        //! Compiles any technique that has not been compiled.
		aMATERIAL_COMPILER_NON_COMPILED = 2,
        //! Compiles everything regardless of states.
		aMATERIAL_COMPILER_EVERYTHING = 4,
        //! Compiles techniques with auto lighting types.
		aMATERIAL_COMPILER_AUTOS = 8,
		//! Compiles directional shadow casters.
		aMATERIAL_COMPILER_DIRECTIONAL_SHADOW_CASTERS = 16,
		//! Compiles spot shadow casters.
		aMATERIAL_COMPILER_SPOT_SHADOW_CASTERS = 32,
		//! Compiles point shadow casters.
		aMATERIAL_COMPILER_POINT_SHADOW_CASTERS = 64
	};

	enum eMASH_CONTROLLER_TYPE
	{
		aCONTROLLER_TRANSFORMATION
		//Light colour controller
		//Particle controller
	};

	enum eANIM_KEY_TYPE
	{
		aANIM_KEY_TRANSFORM
	};

	enum eSHADOW_MAP_FORMAT
	{
		aSHADOW_FORMAT_16,
		aSHADOW_FORMAT_32
	};

	enum eLIGHTING_TYPE
	{
		aLIGHT_TYPE_NONE,
		aLIGHT_TYPE_AUTO,
		aLIGHT_TYPE_VERTEX,
		aLIGHT_TYPE_PIXEL,
		aLIGHT_TYPE_DEFERRED,
		aLIGHT_TYPE_LIGHT_MAP//not used
	};	
    
    enum eLIGHT_RENDERER_TYPE
    {
        aLIGHT_RENDERER_DEFERRED,
        aLIGHT_RENDERER_FORWARD,
        aLIGHT_RENDERER_NOT_SET
    };

	enum ePROGRAM_TYPE
	{
		aPROGRAM_VERTEX,
		aPROGRAM_GEOMETRY,
		aPROGRAM_PIXEL,
		aPROGRAM_UNKNOWN
	};

	enum eSHADER_API_TYPE
	{
		aSHADERAPITYPE_D3D9,
		aSHADERAPITYPE_D3D10,
		aSHADERAPITYPE_OPENGL,
		aSHADERAPITYPE_UNKNOWN
	};

	enum eSHADER_PROFILE
	{
		aSHADER_PROFILE_VS_1_1,
		aSHADER_PROFILE_VS_2_0,
		aSHADER_PROFILE_VS_3_0,
		aSHADER_PROFILE_VS_4_0,
		aSHADER_PROFILE_VS_5_0,
		aSHADER_PROFILE_PS_1_1,
		aSHADER_PROFILE_PS_1_2,
		aSHADER_PROFILE_PS_1_3,
		aSHADER_PROFILE_PS_2_0,
		aSHADER_PROFILE_PS_3_0,
		aSHADER_PROFILE_PS_4_0,
		aSHADER_PROFILE_PS_5_0,
		aSHADER_PROFILE_GS_4_0,
		aSHADER_PROFILE_GS_5_0,

		aSHADER_PROFILE_VS_GLSL,
		aSHADER_PROFILE_PS_GLSL,
		aSHADER_PROFILE_GS_GLSL,

		aSHADER_PROFILE_UNKNOWN
	};

	enum eEFFECT_SEMANTICS
	{
		aEFFECT_WORLD_VIEW_PROJ,
		aEFFECT_VIEW_PROJECTION,
		aEFFECT_WORLD,
		aEFFECT_VIEW,
		aEFFECT_PROJECTION,
		aEFFECT_WORLD_INV_TRANSPOSE,
		aEFFECT_VIEW_INV_TRANSPOSE,
		aEFFECT_INV_VIEW_PROJ,
		aEFFECT_INV_VIEW,
		aEFFECT_INV_PROJECTION,
		aEFFECT_SAMPLER,
		aEFFECT_WORLD_VIEW_INV_TRANSPOSE,
		aEFFECT_CAMERA_NEAR_FAR,
		aEFFECT_SCENE_SHADOWMAP,
		aEFFECT_LIGHT,
		aEFFECT_GBUFFER_DIFFUSE_SAMPLER,
		aEFFECT_GBUFFER_SPECULAR_SAMPLER,
		aEFFECT_GBUFFER_DEPTH_SAMPLER,
		aEFFECT_GBUFFER_NORMAL_SAMPLER,
		aEFFECT_GBUFFER_LIGHT_SAMPLER,
		aEFFECT_GBUFFER_LIGHT_SPECULAR_SAMPLER,
		aEFFECT_BONE_PALETTE_ARRAY,
		aEFFECT_LIGHT_WORLD_POSITION,
		aEFFECT_WORLD_VIEW,
		aEFFECT_WORLD_POSITION,
		aEFFECT_SHADOWS_ENABLED,
		aEFFECT_UNDEFINED
	};

	static const int8 *const g_effectAutoNames[] = {
		"autoWorldViewProjection",
		"autoViewProjection",
		"autoWorld",
		"autoView",
		"autoProjection",
		"autoWorldInvTranspose",
		"autoViewInvTranspose",
		"autoInvViewProjection",
		"autoInvView",
		"autoInvProjection",
		"autoSampler",
		"autoWorldViewInvTranspose",
		"autoCameraNearFar",
		"autoSceneShadowMap",
		"autoLight",
		"autoGBufferDiffuseSampler",
		"autoGBufferSpecularSampler",
		"autoGBufferDepthSampler",
		"autoGBufferNormalSampler",
		"autoGBufferLightSampler",
		"autoGBufferLightSpecularSampler",
		"autoBonePalette",
		"autoLightWorldPosition",
		"autoWorldView",
		"autoWorldPosition",
		"autoShadowsEnabled",
		0
	};

	//these are matched up to eVERTEX_DECLUSAGE
	static const int8 *const g_pOpenGLAttributeNames[] = {
		"_inposition",
		"_inblendweight",
		"_inblendindices",
		"_innormal",
		"_intexcoord",
		"_incustom",
		"_intangent",
		"_inbinormal",
		"_incolor",
		0
	};

	enum eADDITIONAL_EFFECT_SEMANTICS
	{
		aADDITIONAL_EFFECT_GUI_ALPHAMASK_THRESHOLD,
		aADDITIONAL_EFFECT_GUI_BASE_COLOUR,
		aADDITIONAL_EFFECT_GUI_FONT_COLOUR,
		aADDITIONAL_EFFECT_SOFT_PARTICLE_SCALE,
		aADDITIONAL_EFFECT_PARTICLE_BUFFER
	};

	static const int8 *const g_additionalEffectAutoNames[] = {
		"autoGUIAlphaMaskThreshold",
		"autoGUIBaseColour",
		"autoGUIFontColour",
		"autoSoftParticleScale",
		"autoParticleBuffer",
		0
	};

	enum eFILE_IO_MODE
	{
		aFILE_IO_TEXT,
		aFILE_IO_BINARY
	};

	enum eRESOURCE_TYPE
	{
		aRESOURCE_VERTEX,
		aRESOURCE_INDEX,
		aRESOURCE_TEXTURE,
		aRESOURCE_CUBE_TEXTURE,
		aRESOURCE_RENDER_SURFACE
	};

	enum eCUBEMAP_FACE
	{
		aCUBEMAP_FACE_POS_X,
		aCUBEMAP_FACE_NEG_X,
		aCUBEMAP_FACE_POS_Y,
		aCUBEMAP_FACE_NEG_Y,
		aCUBEMAP_FACE_POS_Z,
		aCUBEMAP_FACE_NEG_Z
	};

	enum eBUFFER_LOCK
	{
		aLOCK_WRITE = 0,//use this for writing to static and dynamic buffers. Dynamic buffers can gain a performance increase by using aLOCK_DISCARD instead.
		aLOCK_WRITE_DISCARD, //only used for dynamic buffers
		aLOCK_WRITE_NOOVERWRITE
	};

	enum eCULL
	{
		aCULL_CULLED,
		aCULL_CLIPPED,
		aCULL_VISIBLE
	};

	enum eCLASSIFY
	{
		aCLASS_FRONT,
		aCLASS_BEHIND,
		aCLASS_STRADDLE,
		aCLASS_COPLANAR
	};

	enum eMASH_STATUS
	{
		aMASH_FAILED,
		aMASH_OK
	};

	enum eCAMERA_TYPE
	{
		aCAMERA_TYPE_FIXED
	};

	enum eNODE_TYPE
	{
		aNODETYPE_CAMERA = 1,
		aNODETYPE_LIGHT = 2,
		aNODETYPE_PARTICLE_EMITTER = 4,
		aNODETYPE_ENTITY = 8,
		aNODETYPE_DUMMY = 16,
		aNODETYPE_DECAL = 32,
		aNODETYPE_BONE = 64,
		aNODETPYE_ALL = 0xFFFFFFFF
	};

	enum eRENDER_STAGE
	{
		/*!
            These are objects that makeup the final lit scene.
			This stage encapsulates all render passes.
		*/
		aRENDER_STAGE_SCENE,
        
		/*!
            These are object that will cast shadows.
			The shadow stage doesn't really pay any attention to render passes.
		*/
		aRENDER_STAGE_SHADOW
	};

	enum eRENDER_PASS
	{
		aPASS_TRANSPARENT,
		aPASS_SOLID,
		aPASS_DEFERRED,

		aPASS_TYPE_COUNT
	};

	//high level render pass
	enum eHLRENDER_PASS
	{
		aHLPASS_SCENE,
		aHLPASS_PARTICLES,
		aHLPASS_DECAL,

		aHLPASS_TYPE_COUNT
	};

	enum eFRUSTRUM
	{
		aVF_LEFT,
		aVF_RIGHT,
		aVF_TOP,
		aVF_BOTTOM,
		aVF_NEAR,
		aVF_FAR
	};

	enum eCLEAR_FLAG
	{
		aCLEAR_DEPTH = 1,
		aCLEAR_STENCIL = 2,
		aCLEAR_TARGET = 4
	};	

	enum eUSAGE
	{
        /*!
            Static resources are created for fastest access for the GPU.
            These resource cannot be altered after creation.
        */
		aUSAGE_STATIC = 1,
        
        /*!
            Dynamic resource can be read and written to from the CPU. This is handy
            for runtime dynamic meshes or custom texture data.
            These have slower performance than static resources.
        */
		aUSAGE_DYNAMIC = 2,
        
        /*!
            Used only for render surfaces.
        */
		aUSAGE_RENDER_TARGET = 4
	};

	/*
		Note, on some API's the RGBA format may transform into ARGB.
		For most applcations this shouldn't be an issue.
		If you require direct access to a textures bits and colour order is important,
		then it is recommended that you test your application on the target platform(s) 
		to assure correct access is made.
	*/
	enum eFORMAT
	{
		aFORMAT_RGBA8_UINT,
		aFORMAT_RGBA16_UINT,

		aFORMAT_RGBA8_SINT,
		aFORMAT_RGBA16_SINT,

		aFORMAT_RGBA16_FLOAT,
		aFORMAT_RGBA32_FLOAT,

		aFORMAT_R8_UINT,
		aFORMAT_R16_UINT,
		aFORMAT_R32_UINT,

		aFORMAT_R16_FLOAT,
		aFORMAT_R32_FLOAT,

		aFORMAT_RG16_FLOAT,
		aFORMAT_RG32_FLOAT,

		aFORMAT_DEPTH32_FLOAT
	};

	enum eINPUT_TYPE
	{
		aINPUT_KEYBOARD,
		aINPUT_MOUSE,
		aINPUT_JOYSTICK,

		aINPUT_COUNT
	};

	enum //eINPUT_KEY_MAP
	{
		aINPUTMAP_MOVE_HORIZONTAL,
		aINPUTMAP_MOVE_VERTICAL,
		aINPUTMAP_LOOK_HORIZONTAL,
		aINPUTMAP_LOOK_VERTICAL,
		aINPUTMAP_FIRE,
		aINPUTMAP_SECONDARY_FIRE,
		aINPUTMAP_JUMP,

		aINPUTMAP_USER_REGION,

		/*
			User defined actions
		*/

		aINPUTMAP_MAX = 50
	};

	enum eINPUT_CONTROLLER_TYPE
	{
		aINPUTCONTROLLER_KEYBOARD_MOUSE,
		aINPUTCONTROLLER_JOYSTICK,

		aINPUTCONTROLLER_COUNT
	};

	enum eINPUT_EVENT
	{
        aKEYEVENT_NONE = 0x00,
        
        //virtual keys
        aKEYEVENT_F1,
        aKEYEVENT_F2,
        aKEYEVENT_F3,
        aKEYEVENT_F4,
        aKEYEVENT_F5,
        aKEYEVENT_F6,
        aKEYEVENT_F7,
        aKEYEVENT_F8,
        aKEYEVENT_F9,
        aKEYEVENT_F10,
        aKEYEVENT_F11,
        aKEYEVENT_F12,
        aKEYEVENT_ESCAPE,
        aKEYEVENT_TAB,
        aKEYEVENT_RETURN,
        aKEYEVENT_LEFT,
        aKEYEVENT_RIGHT,
        aKEYEVENT_UP,
        aKEYEVENT_DOWN,
        aKEYEVENT_PGUP,
        aKEYEVENT_PGDOWN,
        aKEYEVENT_DELETE,
        aKEYEVENT_BACKSPACE,
        aKEYEVENT_HOME,
        aKEYEVENT_END,
		aKEYEVENT_SHIFT,
		aKEYEVENT_CTRL,
		aKEYEVENT_MENU,
     
        //the following keys are mapped to their ASCII values
		aKEYEVENT_SPACE = 0x20,
        aKEYEVENT_EXCLAM,
        aKEYEVENT_QUOTE,
        aKEYEVENT_HASH,
        aKEYEVENT_DOLLAR,
        aKEYEVENT_PERCENT,
        aKEYEVENT_AMPERSAND,
        aKEYEVENT_APOSTROPHE,
        aKEYEVENT_LEFTPAREN,
        aKEYEVENT_RIGHTPARAN,
        aKEYEVENT_ASTERISK,
        aKEYEVENT_PLUS,
        aKEYEVENT_COMMA,
        aKEYEVENT_MINUS,
        aKEYEVENT_PERIOD,
        aKEYEVENT_SLASH,
        aKEYEVENT_0,
        aKEYEVENT_1,
        aKEYEVENT_2,
        aKEYEVENT_3,
        aKEYEVENT_4,
        aKEYEVENT_5,
        aKEYEVENT_6,
        aKEYEVENT_7,
        aKEYEVENT_8,
        aKEYEVENT_9,
        aKEYEVENT_COLON,
        aKEYEVENT_SEMICOLON,
        aKEYEVENT_LESS,
        aKEYEVENT_EQUALS,
        aKEYEVENT_GREATER,
        aKEYEVENT_QUESTION,
        aKEYEVENT_AT,
        aKEYEVENT_A,
        aKEYEVENT_B,
        aKEYEVENT_C,
        aKEYEVENT_D,
        aKEYEVENT_E,
        aKEYEVENT_F,
        aKEYEVENT_G,
        aKEYEVENT_H,
        aKEYEVENT_I,
        aKEYEVENT_J,
        aKEYEVENT_K,
        aKEYEVENT_L,
        aKEYEVENT_M,
        aKEYEVENT_N,
        aKEYEVENT_O,
        aKEYEVENT_P,
        aKEYEVENT_Q,
        aKEYEVENT_R,
        aKEYEVENT_S,
        aKEYEVENT_T,
        aKEYEVENT_U,
        aKEYEVENT_V,
        aKEYEVENT_W,
        aKEYEVENT_X,
        aKEYEVENT_Y,
        aKEYEVENT_Z,
        aKEYEVENT_LEFTBRACKET,
        aKEYEVENT_BACKSLASH,
        aKEYEVENT_RIGHTBRACKET,
        aKEYEVENT_CARET,
        aKEYEVENT_UNDERSCORE,
        aKEYEVENT_GRAVE = 0x60,
        /*
            Lower case from 0x61 - 0x7A
         */
        aKEYEVENT_LBRACE = 0x7B,
        aKEYEVENT_PIPE,
        aKEYEVENT_RBRACE,
        aKEYEVENT_TILDE = 0x7E,
		//End ASCII range

		aJOYEVENT_POVUP = 127,//DPAD_UP
		aJOYEVENT_POVDOWN,//DPAD_DOWN
		aJOYEVENT_POVLEFT,//DPAD_LEFT
		aJOYEVENT_POVRIGHT,//DPAD_RIGHT
		aJOYEVENT_B0,//START
		aJOYEVENT_B1,//BACK
		aJOYEVENT_B2,//LEFT THUMB STICK
		aJOYEVENT_B3,//RIGHT THUMB STICK
		aJOYEVENT_B4,//LEFT SHOULDER
		aJOYEVENT_B5,//RIGHT SHOULDER
		aJOYEVENT_B6,//A
		aJOYEVENT_B7,//B
		aJOYEVENT_B8,//X
		aJOYEVENT_B9,//Y
		aJOYEVENT_AXIS_1_X,
		aJOYEVENT_AXIS_1_Y,
		aJOYEVENT_AXIS_2_X,
		aJOYEVENT_AXIS_2_Y,
		aJOYEVENT_THROTTLE_1,
		aJOYEVENT_THROTTLE_2,

		aJOYEVENT_CONNECT,
		aJOYEVENT_DISCONNECT,
        aJOYEVENT_UNKNOWN,

		aMOUSEEVENT_B1,
		aMOUSEEVENT_B2,
		aMOUSEEVENT_B3,
		aMOUSEEVENT_AXISX,
		aMOUSEEVENT_AXISY,
		aMOUSEEVENT_AXISZ,

		aKEYEVENT_SIZE
	};
    
	enum eGUI_EVENT
	{
		aGUIEVENT_MOUSE_ENTER,
		aGUIEVENT_MOUSE_EXIT,
		aGUIEVENT_INPUTFOCUS,
		aGUIEVENT_LOST_INPUTFOCUS,
		aGUIEVENT_BTN_DOWN,
		aGUIEVENT_BTN_UP_CONFIRM,
		aGUIEVENT_BTN_UP_CANCEL,
		aGUIEVENT_CB_TOGGLE_OFF,
		aGUIEVENT_CB_TOGGLE_ON,
		aGUIEVENT_SB_VALUE_CHANGE,
		aGUIEVENT_SB_BUTTON_RELEASE,
		aGUIEVENT_TC_TAB_CHANGE,
		aGUIEVENT_LB_SELECTION_CHANGE,
		aGUIEVENT_LB_SELECTION_CONFIRMED,
		aGUIEVENT_POPUP_SELECTION,
		aGUIEVENT_OPENFILE_SELECTED,
		aGUIEVENT_TB_TEXT_CHANGE,
		aGUIEVENT_TB_CONFIRM,
		aGUIEVENT_TREE_SELECTION_CHANGE,
		aGUIEVENT_TREE_SELECTION_CONFIRMED,
		aGUIEVENT_WINDOW_CLOSE,
		aGUIEVENT_WINDOW_MINIMIZE,
		aGUIEVENT_WINDOW_MAXIMIZE,
		aGUIEVENT_MENUBAR_SELECTION,
		aGUIEVENT_POPUP_ZERO_ITEMS
	};

	enum eCOLOUR_WRITE
	{
        aCOLOUR_WRITE_NONE = 0,
		aCOLOUR_WRITE_RED = 1,
		aCOLOUR_WRITE_GREEN = 2,
		aCOLOUR_WRITE_BLUE = 4,
		aCOLOUR_WRITE_ALPHA = 8,
		aCOLOUR_WRITE_ALL = (aCOLOUR_WRITE_RED | aCOLOUR_WRITE_GREEN |
			aCOLOUR_WRITE_BLUE | aCOLOUR_WRITE_ALPHA)
	};

	enum eBLENDOP
	{
		aBLENDOP_ADD,
		aBLENDOP_MAX,
		aBLENDOP_MIN,
		aBLENDOP_REV_SUBTRACT,
		aBLENDOP_SUBTRACT
	};

	enum eBLEND
	{			
		aBLEND_SRC_ALPHA,
		aBLEND_INV_SRC_ALPHA,
		aBLEND_DEST_ALPHA,
		aBLEND_DEST_COLOR,
		aBLEND_INV_DEST_ALPHA,
		aBLEND_INV_SRC_COLOR,
		aBLEND_ONE,
		aBLEND_SRC_ALPHA_SAT,
		aBLEND_SRC_COLOR,
		aBLEND_ZERO,
		aBLEND_INV_DEST_COLOR
	};

	enum eSAMPLER
	{
		aSAMPLER1D,
		aSAMPLER2D,
		aSAMPLER3D,
		aSAMPLERCUBE
	};

	enum eFILTER
	{
		aFILTER_MIN_MAG_MIP_POINT,
		aFILTER_MIN_MAG_MIP_LINEAR,

		aFILTER_MIN_MAG_MIP_ANISOTROPIC,

		//disable mipmapping
		aFILTER_MIN_MAG_POINT,
		aFILTER_MIN_MAG_LINEAR,
	};

	enum eTEXTURE_ADDRESS
	{
		aTEXTURE_ADDRESS_WRAP,
		aTEXTURE_ADDRESS_CLAMP
	};

	enum eCULL_MODE
	{
		aCULL_NONE,
		aCULL_CW,
		aCULL_CCW
	};

	enum eFILL_MODE
	{
		aFILL_WIRE_FRAME,
		aFILL_SOLID
	};	

	enum eDEPTH_COMPARISON
	{
		aZCMP_NEVER,
		aZCMP_LESS,
		aZCMP_EQUAL,
		aZCMP_LESS_EQUAL,
		aZCMP_GREATER,
		aZCMP_NOT_EQUAL,
		aZCMP_GREATER_EQUAL,
		aZCMP_ALWAYS
	};

	enum eVERTEX_DECLTYPE
	{
		aDECLTYPE_R32_FLOAT,
		aDECLTYPE_R32G32_FLOAT,
		aDECLTYPE_R32G32B32_FLOAT,
		aDECLTYPE_R32G32B32A32_FLOAT,
		aDECLTYPE_R8G8B8A8_UNORM,
		aDECLTYPE_R8G8B8A8_UINT,
		aDECLTYPE_R16G16_SINT,
		aDECLTYPE_R16G16B16A16_SINT,
		
		aVERTEX_DECLTYPE_COUNT
	};

	enum eVERTEX_CLASSIFICATION
	{
		aCLASSIFICATION_VERTEX_DATA,
		aCLASSIFICATION_INSTANCE_DATA
	};

	enum eVERTEX_DECLUSAGE
	{
		aDECLUSAGE_POSITION,
		aDECLUSAGE_BLENDWEIGHT,
		aDECLUSAGE_BLENDINDICES,
		aDECLUSAGE_NORMAL,
		aDECLUSAGE_TEXCOORD,
		aDECLUSAGE_CUSTOM,
		aDECLUSAGE_TANGENT,
		aDECLUSAGE_BINORMAL,//Shouldn't be used.	
		aDECLUSAGE_COLOUR,

		aVERTEX_DECLUSAGE_COUNT
	};

	enum ePRIMITIVE_TYPE
	{
		aPRIMITIVE_TRIANGLE_STRIP,
		aPRIMITIVE_TRIANGLE_LIST,
		aPRIMITIVE_POINT_LIST,
		aPRIMITIVE_LINE_STRIP,
		aPRIMITIVE_LINE_LIST
	};

	enum eCOLOUR
	{
		aCOLOUR_BLACK = 0xFF000000,
		aCOLOUR_RED = 0xFFFF0000,
		aCOLOUR_GREEN = 0xFF00FF00,
		aCOLOUR_BLUE = 0xFF0000FF,
		aCOLOUR_YELLOW = 0xFFFFFF00,
		aCOLOUR_AQUA = 0xFF00FFFF,
		aCOLOUR_AQUAMARINE = 0x7FFFD4FF,
		aCOLOUR_DEEP_SKY_BLUE = 0x00BFFFFF,
		aCOLOUR_DODGER_BLUE = 0x1E90FFFF,
		aCOLOUR_ROYAL_BLUE = 0x4169E1FF,
		aCOLOUR_DARK_SLATE_BLUE = 0x483D8BFF,
		aCOLOUR_VIOLET = 0xFFFF00FF,
		aCOLOUR_GREY = 0xFFC0C0C0,
		aCOLOUR_WHITE = 0xFFFFFFFF
	};

	enum eLIGHTTYPE
	{			
		aLIGHT_POINT,
		aLIGHT_SPOT,
		aLIGHT_DIRECTIONAL,
		aLIGHT_TYPE_COUNT
	};

	//used for (de)serialise
	enum //eSHADOW_CASTER_TYPE
	{
		aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD,
		aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM,
		aSHADOW_CASTER_SPOT_STANDARD,
		aSHADOW_CASTER_SPOT_ESM,
		aSHADOW_CASTER_POINT_STANDARD,
		aSHADOW_CASTER_POINT_STANDARD_FILTERED,
		aSHADOW_CASTER_POINT_ESM,
		aSHADOW_CASTER_CUSTOM_RANGE
        /*
         *  User range
         */
	};

	enum eCOMPARE
	{
		aCOMP_ALWAYS,
		aCOMP_EQUAL,
		aCOMP_GREATER,
		aCOMP_GREATEREQUAL,
		aCOMP_LESS,
		aCOMP_LESSEQUAL,
		aCOMP_NEVER,
		aCOMP_NOTEQUAL
	};

	enum eCMPFUNC
	{
		aCMP_NEVER,
		aCMP_LESS,
		aCMP_EQUAL,
		aCMP_LESSEQUAL,
		aCMP_GREATER,
		aCMP_NOTEQUAL,
		aCMP_GREATEREQUAL,
		aCMP_ALWAYS
	};
}

#endif
