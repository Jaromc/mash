//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

class MainLoop : public mash::MashGameLoop
{
private:
	MashDevice *m_device;
	MashCamera *m_camera;
public:
	MainLoop(mash::MashDevice *device):m_device(device){}
	virtual ~MainLoop(){}

	/*
		All scene setup should be done in this function. This includes:
		- material loading
		- applying materials to a mesh
		- mesh construction
		- loading scene nodes from a file
		- adding lights and light/shadow settings

		After this function exits the engine compiles all materials and shaders loaded, and also
		compiles some internal API specific objects.

		Lighting values can be changed after this function but some lighting values will force
		some materials to be recompiled to reflect those changes. This will affect runtime 
        performance. These values include:
		- enable/disable shadows
		- adding/removing lights
		- changing light types
		- changing what light is considered as the main forward rendering light

		Simply changing light colours, direction, attenuation, position at runtime is fine, and won't
		affect performance.
	*/
	bool Initialise()
	{
		/*
			There always needs to be one active camera in the current scene.
			The first camera created is set as the active camera by default. The active
			camera can be changed by calling SceneManager::SetActiveCamera()
		*/
		m_camera = m_device->GetSceneManager()->AddCamera(0, "Camera01");
		m_camera->SetZFar(1000);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 0, 0));

        //return true to abort the application loading further.
		return false;
	}

	/*
		This function is called in fixed time steps, normally 0.016ms (60fps)
        but this step can be changed if needed when the device is created.
		From here you would do all scene updating from this function.
     
        This function may be called mutiple times per frame or not at all depending
        on the speed of the application in relation to the fixed time step.
	*/
	bool Update(f32 dt)
    {
		/*
			Updates the given scenes animations and positions.

			SceneManager::UpdateScene() can be called multiple times to update
			different scene graphs.
		*/
		m_device->GetSceneManager()->UpdateScene(dt, m_camera);
		
        //return true to quit the application.
		return false;
	}

	/*
		Called once every frame (not at fixed time steps).
	*/
	void LateUpdate(f32 dt)
	{
	}

	/*
		This is where all rendering occurs.
     
        Call MashSceneManager::CullScene() once per scene graph you want rendered, this fills 
        the internal render buckets. Than call MashSceneManager::DrawScene() to render 
        everything to the current render target and empty the buckets.
	*/
	void Render()
    {
		/*
			Culls the given scene for rendering. SceneManager::CullScene() can be called
			multiple times for different scene graphs. This can be handy if you wanted
			to cull different graphs using different culling techniques. Nodes that pass
            culling will be added to internal render buckets and drawn to the screen
            when MashSceneManager::DrawScene() is called.

			Some scene node data is delayed till a node passes culling. This is to save on
            unncessary processing. After this function has been called for a scene graph, 
            all nodes that passed culling will be completly updated.
         
            Nodes that pass culling have their render/interpolated position updated forward towards
			MashSceneNode::GetWorldTransformState().
            This interpolation reduces any jitter that may be noticable in a nodes movement due to 
			changing frame rates. A nodes render position can be accessed by calling 
			MashSceneNode::GetRenderTransformState().
		*/
		m_device->GetSceneManager()->CullScene(m_camera);

		/*
			Draws the culled objects to the screen. This function will choose forward or deferred
			rendering based on material and lighting settings, and generate shadow maps if needed.
            Finally the scene will be rendered to the render target set.
		*/
		m_device->GetSceneManager()->DrawScene();
        
        /*
            The default render target is your main backbuffer. If you were to render
            to another render target at some point then you will need to call this
            before this function exits to render the final scene to the backbuffer.
            In this case it's not necessary to call it because we haven't changed render
            targets, it's just here for your information.
         */
		m_device->GetRenderer()->SetRenderTargetDefault();
	}
};

int main()
{
	/*
		Loads the engine with these settings. Optionally you could load
		these in from a file for easy settings changes.

		The function pointers create the main and optional components to the engine.
		The gui, physics and script managers are all optional and can be null.
		This saves on .exe size and runtime memory if these are things you
		are not going to use.
	*/
	sMashDeviceSettings deviceSettings;
#ifdef USE_DIRECTX
	deviceSettings.rendererFunctPtr = CreateMashD3D10Device;
#else
    deviceSettings.rendererFunctPtr = CreateMashOpenGL3Device;
#endif
	//deviceSettings.guiManagerFunctPtr = CreateMashGUI;
	//deviceSettings.physicsManagerFunctPtr = CreateMashPhysics;
	//deviceSettings.scriptManagerFunctPtr = CreateMashScriptManager;

	deviceSettings.fullScreen = false;
	deviceSettings.screenWidth = 800;
	deviceSettings.screenHeight = 600;
	deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;

	/*
		Root paths tell the engine where to look for files. You should include at least
		the paths stated below to the engine data. Then add paths to any of your 
        application files (texture, models, sounds, etc...).
		Eg, Your sound path may be "../GameMedia/Sounds". Then throught your code you can
		load these sounds by simply calling "ShootSound.mp3". The root paths will be searched
		for your sound.
		This behavior can be handy for easy path changes. But be aware of files that share the
		same name in different paths as the wrong file could be loaded. Smart name choices will
		avoid any issues here.
	*/
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    
    ////Only needed if GUI is used.
    //deviceSettings.rootPaths.push_back("../../../../../Media/GUI");
    
    ////You can set up paths that debug material information will be written to.
    //deviceSettings.compiledShaderOutputDirectory = "../MaterialDebug";
    //deviceSettings.intermediateShaderOutputDirectory = "../MaterialDebug";

	//Creates the device
	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Hello World Demo");

	/*
		Sets the game loop.
		This can be called multiple times throught your application life
		for different loops. Maybe a different loop per game level?
	*/
	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
