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

class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;

	MashSceneNode *m_root;
	uint32 m_playerControl;
	MashCamera *m_camera;
	MashGUIStaticText *m_fpsText;
public:
	MainLoop(MashDevice *device):m_device(device){}
	virtual ~MainLoop()
	{
	}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("NormalMapMaterials.mtl");
		MashMaterial *barrelMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BarrelMaterial");
		if (!barrelMaterial)
			return true;

		MashMaterial *wallMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("WallMaterial");
		if (!wallMaterial)
			return true;

		MashList<MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("NormalMapScene.nss", rootNodes, loadSettings);

		m_root = rootNodes.Front();

		MashLight *light = (MashLight*)m_device->GetSceneManager()->GetSceneNodeByName("Omni001");	
		light->SetDiffuse(sMashColour4(1.0f, 1.0f, 1.0f,0.0f));
		light->SetSpecular(sMashColour4(1.0f, 1.0f, 1.0f,0.0f));
		light->SetAmbient(sMashColour4(0.1f, 0.1f, 0.1f, 0.0f));
		light->SetAttenuation(0.0, 0.0001, 1.0);
		light->SetRange(1000);

		light->SetLightRendererType(aLIGHT_RENDERER_DEFERRED);
        light->SetShadowsEnabled(true);

		MashAnimationMixer *mixer = light->GetAnimationMixer();
		mixer->Transition("LightCycle");
		mixer->SetWrapMode("LightCycle", aWRAP_LOOP);

		/*
			Load shadow caster data from a xml file. Alternatively you can create shadow casters in
			code using:
			
			MashPointShadowCaster *shadowCaster = m_device->GetSceneManager()->CreatePointShadowCaster();
			m_device->GetSceneManager()->SetShadowCaster(aLIGHT_POINT, shadowCaster);
			shadowCaster->Drop();//the manager now owns it
		*/
		m_device->GetSceneManager()->LoadShadowCastersFromFile("ShadowCasterData.xml");

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		m_camera->SetZFar(1000);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 60, -200));
        
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 500.0f);
        m_camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
			MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 120.0f));
		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		MashStringc text= "\Controls :\nWASD to move the camera\n\
Hold mouse button 2 then move the mouse to look around";

		staticText->SetText(text);

		return false;
	}

	bool Update(f32 dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_root);

		return false;
	}

	void Render()
	{
		m_device->GetRenderer()->SetRenderTargetDefault();

		sMashViewPort originalViewport = m_device->GetRenderer()->GetViewport();
		int32 miniViewportWidth = originalViewport.width / 4;
		int32 miniViewportHeight = originalViewport.height / 3;

		sMashViewPort mainSceneViewport;

		//changes the size and position of the final composed image.
		mainSceneViewport.minZ = 0.0f;
		mainSceneViewport.maxZ = 1.0f;
		mainSceneViewport.x = 0;
		mainSceneViewport.y = 0;
		mainSceneViewport.width = originalViewport.width - miniViewportWidth;
		mainSceneViewport.height = originalViewport.height - miniViewportHeight;
		m_device->GetRenderer()->SetViewport(mainSceneViewport);
        
        /*
            Important, Changing the viewport can also change the aspect ratio of the
            camera. Culling must happen after the viewport change otherwise the scene
            will be culled using a different view frustum, and this may result in objects
            being culled before they are no longer visible.
        */
        m_device->GetSceneManager()->CullScene(m_root);

		m_device->GetSceneManager()->DrawScene();

		//set the viewport back to its orignal state.
		m_device->GetRenderer()->SetViewport(originalViewport);

		MashRectangle2 miniViewportArea;
		miniViewportArea.left = 0.0f;
		miniViewportArea.top = originalViewport.height - miniViewportHeight;
		miniViewportArea.right = miniViewportWidth;
		miniViewportArea.bottom = originalViewport.height;

		/*
			Renders each texture to its own viewport.

			Note here we are grabbing the deferred texture and setting it as the texture to render. 
			Another option is to create your own render material and use the built in effect autos for getting the
			deferred samplers.

			sampler2D autoGBufferDiffuseSampler
			sampler2D autoGBufferSpecularSampler
			sampler2D autoGBufferDepthSampler
			sampler2D autoGBufferNormalSampler
			sampler2D autoGBufferLightSampler
			sampler2D autoGBufferLightSpecularSampler
		*/
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredDiffuseMap(), miniViewportArea);

		miniViewportArea.left = miniViewportWidth; miniViewportArea.right += miniViewportWidth;
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredNormalMap(), miniViewportArea);

		miniViewportArea.left += miniViewportWidth; miniViewportArea.right += miniViewportWidth;
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredSpecularMap(), miniViewportArea);

		miniViewportArea.left += miniViewportWidth; miniViewportArea.right += miniViewportWidth;
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredDepthMap(), miniViewportArea);

		miniViewportArea.left = originalViewport.width - miniViewportWidth;
		miniViewportArea.right = originalViewport.width;
		miniViewportArea.top = 0;
		miniViewportArea.bottom = miniViewportHeight;
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredLightingMap(), miniViewportArea);

		miniViewportArea.top += miniViewportHeight; miniViewportArea.bottom += miniViewportHeight;
		m_device->GetRenderer()->DrawTexture(m_device->GetSceneManager()->GetDeferredLightingSpecularMap(), miniViewportArea);

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();

	}
};

int main()
{
	sMashDeviceSettings deviceSettings;
#ifdef USE_DIRECTX
	deviceSettings.rendererFunctPtr = CreateMashD3D10Device;
#else
    deviceSettings.rendererFunctPtr = CreateMashOpenGL3Device;
#endif
	deviceSettings.guiManagerFunctPtr = CreateMashGUI;
	deviceSettings.fullScreen = false;
	deviceSettings.screenWidth = 800;
	deviceSettings.screenHeight = 600;
	deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_DEFERRED;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_NONE;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/DeferredDataDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Deferred Data Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
