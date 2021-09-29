//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_CREATION_PARAMETERS_H_
#define _MASH_CREATION_PARAMETERS_H_

#include "MashEnum.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	class MashVideo;
	class MashGUIManager;
	class MashScriptManager;
	class MashPhysics;
    class MashFileManager;

	struct sMashDeviceSettings
	{
		MashVideo* (*rendererFunctPtr)();
		MashGUIManager* (*guiManagerFunctPtr)();
		MashPhysics* (*physicsManagerFunctPtr)();
		MashScriptManager* (*scriptManagerFunctPtr)();
        
        /*!
            This will be called when the file manager is initialised but before
            most other engine objects are created. This allows you to load engine
            depended resources from memory into the virtual file system. You may 
            do this, for example, when archiving resources (textures, materials, etc...) 
            to hide them from end users.
         
            Note, it is important to only interact with the file manager within this callback
            because other engine components may not yet be initialised.
         
            The vfs may have files added and removed after initialising if needed.
        */
        void (*virtualFileLoaderPtr)(MashFileManager *);
		
		bool fullScreen;
		bool enableVSync;
		eBACKBUFFER_FORMAT backbufferFormat;
		eDEPTH_FORMAT depthFormat;
        
        /*!
            This is the rate at which the scene will be updated per second. Ideally this should
            be a value at which your application can maintain. For example, 1.0 / 30.0 would 
            update the scene every 30th of a second. It would be best if the fixed step is set
			to 1.0/30 (30 fps) while maintaining 60 fps render rate. This provides the smoothest
			animations because transforms are also interpolated between updates.
        */
        f32 fixedTimeStep;

		/*!
			If this is a windowed app then this is the size of the backbuffer and window.
			If this is a fullscreen app then this is the size of the backbuffer only.
		*/
		uint32 screenWidth;
		uint32 screenHeight;

		/*!
			These will be added to the file manager. This allows you to use file names only
			in your app. The file manager will search the root paths when looking for files.
		*/
		MashArray<MashStringc> rootPaths;

		/*!
			Anti Aliasing is only valid in forward rendering mode.
			If Anti Aliasing is needed for deferred rendering then post processing is needed.
		*/
		eANTIALIAS_TYPE antiAliasType;

		/*!
			Any techniques whos lighting mode is set to auto will adopt this
            lighting mode.
		*/
		eLIGHTING_TYPE preferredLightingMode;

		/*!
			If the prefered lighting mode is deferred then this automatically
			gets forced to true. The user would only need to set this if the preferred lighting
			mode was something other than deferred and a material(s) explicitly state they want
			to use the deferred renderer. In this case certain aspects of the engine will be setup
			to handle any deferred rendering.
			
			If you are only using the forward renderer then keep this set to false. This may help
            to decrease memory consumption.
		*/
		bool enableDeferredRender;
        
        /*!
            Here you can state your custom gui style XML file.
            Leave this empty to load the default style.
        */
		MashStringc guiStyle;

		/*!
            For debugging purposes.
			This is the file and path where debug data will be written to.
		*/
		MashStringc debugFilePath;

		/*!
			For debugging purposes.
			Native hight level shader code will be saved to this location.
		*/
		MashStringc compiledShaderOutputDirectory;
		
		/*!
			For debugging purposes.
			Generated runtime effect save location. This is the shader before conversion into 
			native high level format.
		*/
		MashStringc intermediateShaderOutputDirectory;

		sMashDeviceSettings():rendererFunctPtr(0),
			guiManagerFunctPtr(0),
			physicsManagerFunctPtr(0),
			scriptManagerFunctPtr(0),
            virtualFileLoaderPtr(0),
			fullScreen(false),
			enableVSync(true),
			screenWidth(800),
			screenHeight(600),
            fixedTimeStep(1.0f / 30.0f),
			backbufferFormat(aBACKBUFFER_FORMAT_16BIT),
			depthFormat(aDEPTH_FORMAT_24BIT),
			antiAliasType(aANTIALIAS_TYPE_NONE),
			preferredLightingMode(mash::aLIGHT_TYPE_PIXEL),
			enableDeferredRender(false),
            guiStyle(""),
			compiledShaderOutputDirectory(""),
			intermediateShaderOutputDirectory(""),
			debugFilePath("MashDebug.txt"){}
	};
}

#endif