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
	virtual ~MainLoop(){}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl, inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);//keyboard/mouse

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("NormalMapMaterials.mtl");
		MashMaterial *barrelMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BarrelMaterial");
		if (!barrelMaterial)
			return true;

		MashMaterial *wallMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("WallMaterial");
		if (!wallMaterial)
			return true;

		/*
			Load shadow caster data from a xml file. Alternatively you can create shadow casters in
			code using:
			
			MashPointShadowCaster *shadowCaster = m_device->GetSceneManager()->CreatePointShadowCaster();
			m_device->GetSceneManager()->SetShadowCaster(aLIGHT_POINT, shadowCaster);
			shadowCaster->Drop();//the manager now owns it
		*/
		m_device->GetSceneManager()->LoadShadowCastersFromFile("ShadowCasterData.xml");

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
		light->SetShadowsEnabled(true);
        light->SetLightRendererType(aLIGHT_RENDERER_FORWARD);

		MashAnimationMixer *mixer = light->GetAnimationMixer();
		mixer->Transition("LightCycle");
		mixer->SetWrapMode("LightCycle", aWRAP_LOOP);

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		m_camera->SetZFar(1000);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 60, -200));
        
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 300.0f);
        m_camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
			MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.0f, 120.0f));
		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		MashStringc text= "\Controls :\nW,S move forward/backward\n\
A,D move left/right\n\
Hold MB 2 then move the mouse to look around";

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

		m_device->GetSceneManager()->CullScene(m_root);
		m_device->GetSceneManager()->DrawScene();

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, 256, "FPS : %d", m_device->GetFPS());
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
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;
	deviceSettings.fixedTimeStep = 1.0/30.0;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/NormalMapDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Normal Map Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
