//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"
#include <time.h>

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

#define USE_DEFERRED_PARTICLES

enum PARTICLE_TYPE
{
	PT_CPU,
	PT_GPU,
	PT_MESH,
#ifdef USE_DEFERRED_PARTICLES
	PT_SOFT_CPU,
	PT_SOFT_GPU,
#endif
	PT_COUNT
};

class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;

	MashSceneNode *m_root;
	uint32 m_playerControl;

	MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;

	MashParticleSystem *m_particleSystems[PT_COUNT];
public:
	MainLoop(MashDevice *device):m_device(device){}
	virtual ~MainLoop(){}

	void UpdateText(PARTICLE_TYPE particleType)
	{
		MashStringc text;
		switch(particleType)
		{
		case PT_CPU:
			text = "Particle type : CPU\n";
			break;
		case PT_GPU:
			text = "Particle type : GPU\n";
			break;
		case PT_MESH:
			text = "Particle type : HW Instanced Mesh\n";
			break;
#ifdef USE_DEFERRED_PARTICLES
		case PT_SOFT_CPU:
			text = "Particle type : Soft CPU\n";
			break;
		case PT_SOFT_GPU:
			text = "Particle type : Soft GPU\n";
			break;
#endif
		};

		text += "\nControls :\nW,S move forward/backward\n\
				A,D move left/right\n\
				Z change particle type\n\
				Hold MB 2 then move the mouse to look around";

		m_staticText->SetText(text);
	}

	bool Initialise()
	{
		//seed the random number generator for more random particles
		srand(time(NULL));

		//create controller data
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_Z);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::ChangeParticleType, this));

		/*
			Load material the landscape will use.
			This must be loaded before loading the scene files so that the
			material is found when the mesh is created (the mesh was initialised in
			the scene viewer with this material).
		*/
		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("StaticMeshMaterial.mtl");

		//create a root node to attach everything to
		m_root = m_device->GetSceneManager()->AddDummy(0, "ParticleSceneRoot");

		//create a camera
		MashCamera *camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		camera->SetZFar(10000);
		camera->SetZNear(1.0f);
		camera->SetPosition(MashVector3(0, 100, -500));
        
		//attach an FPS controller to the camera
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 500.0f);
        camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		//create a light
		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		MashVector3 lightDir(-1.0f,-0.5f,0.0f);
		lightDir.Normalize();
		light->SetDefaultDirectionalLightSettings(lightDir);
		light->SetRange(1000);
		light->SetShadowsEnabled(false);

		//load the landscape
		MashList<MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("Landscape.nss", rootNodes, loadSettings);

		MashEntity *landscapeEntity = (MashEntity*)rootNodes.Front()->GetChildByName("Landscape");	

		m_root->AddChild(landscapeEntity);

		sParticleSettings particleSettings;
#ifdef USE_DEFERRED_PARTICLES
		particleSettings.maxParticleCount = 10000;
		particleSettings.particlesPerSecond = 500;
		particleSettings.minStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		particleSettings.maxStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		particleSettings.minEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		particleSettings.maxEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		particleSettings.minStartSize = 40.5f;
		particleSettings.maxStartSize = 70.5f;
		particleSettings.minEndSize = 50.0f;
		particleSettings.maxEndSize = 10.0f;
		particleSettings.minRotateSpeed = -10.0f;
		particleSettings.maxRotateSpeed = 10.0f;
		particleSettings.minVelocity = MashVector3(-20.0f, 20.0f, -20.0f);
		particleSettings.maxVelocity = MashVector3(20.0f, 50.0f, 20.0f);
		particleSettings.gravity = MashVector3(0.0f,-2.0f,0.0f);
		particleSettings.minDuration = 5.0f;
		particleSettings.maxDuration = 5.0f;
		particleSettings.startTime = 5;
		particleSettings.emitterVelocityWeight = 0.1f;
		particleSettings.softParticleScale = 100.0f;
#else
		particleSettings.maxParticleCount = 1000;
		particleSettings.particlesPerSecond = 100;
		particleSettings.minStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		particleSettings.maxStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		particleSettings.minEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		particleSettings.maxEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		particleSettings.minStartSize = 40.5f;
		particleSettings.maxStartSize = 70.5f;
		particleSettings.minEndSize = 50.0f;
		particleSettings.maxEndSize = 10.0f;
		particleSettings.minRotateSpeed = -10.0f;
		particleSettings.maxRotateSpeed = 10.0f;
		particleSettings.minVelocity = MashVector3(-5.0f, 20.0f, -5.0f);
		particleSettings.maxVelocity = MashVector3(5.0f, 50.0f, 5.0f);
		particleSettings.gravity = MashVector3(0.0f,-2.0f,0.0f);
		particleSettings.minDuration = 5.0f;
		particleSettings.maxDuration = 5.0f;
		particleSettings.startTime = 5;
		particleSettings.emitterVelocityWeight = 0.1f;
		particleSettings.softParticleScale = 100.0f;
#endif

		//create CPU particle system
		MashParticleSystem *particleSystem = (MashParticleSystem*)m_device->GetSceneManager()->AddParticleSystem(m_root, "ParticleSystem", particleSettings, aPARTICLE_CPU, aLIGHT_TYPE_VERTEX);
		particleSystem->SetDiffuseTexture(m_device->GetRenderer()->GetTexture("smoke.PNG"));
		MashParticleEmitter *particleEmitter = particleSystem->CreatePointEmitter();
		particleSystem->PlayEmitter();
		m_particleSystems[PT_CPU] = particleSystem;

		//create GPU particle system
		particleSystem = (MashParticleSystem*)m_device->GetSceneManager()->AddParticleSystem(0, "ParticleSystem", particleSettings, aPARTICLE_GPU, aLIGHT_TYPE_VERTEX);
		particleSystem->SetDiffuseTexture(m_device->GetRenderer()->GetTexture("smoke.PNG"));
		particleEmitter = particleSystem->CreatePointEmitter();
		particleSystem->PlayEmitter();
		m_particleSystems[PT_GPU] = particleSystem;

		//create Mesh particle system
		rootNodes.Clear();
		m_device->GetSceneManager()->LoadSceneFile("MeshParticle.nss", rootNodes, loadSettings);

		MashEntity *particleEntity = (MashEntity*)rootNodes.Front();	

		sParticleSettings meshParticleSettings;
		meshParticleSettings.maxParticleCount = 1000;
		meshParticleSettings.particlesPerSecond = 50;
		meshParticleSettings.minStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		meshParticleSettings.maxStartColour = sMashColour4(1.0f,1.0f,1.0f,1.0f);
		meshParticleSettings.minEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		meshParticleSettings.maxEndColour = sMashColour4(1.0f,0.0f,0.0f,1.0f);
		meshParticleSettings.minStartSize = 1.0f;
		meshParticleSettings.maxStartSize = 1.0f;
		meshParticleSettings.minEndSize = 1.0f;
		meshParticleSettings.maxEndSize = 1.0f;
		meshParticleSettings.minRotateSpeed = -10.0f;
		meshParticleSettings.maxRotateSpeed = 10.0f;
		meshParticleSettings.minVelocity = MashVector3(-10.0f, 20.0f, -10.0f);
		meshParticleSettings.maxVelocity = MashVector3(10.0f, 50.0f, 10.0f);
		meshParticleSettings.gravity = MashVector3(0.0f,-2.0f,0.0f);
		meshParticleSettings.minDuration = 5.0f;
		meshParticleSettings.maxDuration = 5.0f;
		meshParticleSettings.startTime = 5;

		particleSystem = (MashParticleSystem*)m_device->GetSceneManager()->AddParticleSystem(0, "ParticleSystem", meshParticleSettings, aPARTICLE_MESH, aLIGHT_TYPE_PIXEL, true, particleEntity->GetModel());
		particleSystem->SetDiffuseTexture(m_device->GetRenderer()->GetTexture("Dirt.PNG"));
		particleEmitter = particleSystem->CreatePointEmitter();

		particleSystem->PlayEmitter();
		m_particleSystems[PT_MESH] = particleSystem;
#ifdef USE_DEFERRED_PARTICLES
		//create soft CPU particle system
		particleSystem = (MashParticleSystem*)m_device->GetSceneManager()->AddParticleSystem(0, "ParticleSystem", particleSettings, aPARTICLE_CPU_SOFT_DEFERRED, aLIGHT_TYPE_VERTEX);
		particleSystem->SetDiffuseTexture(m_device->GetRenderer()->GetTexture("smoke.PNG"));
		particleEmitter = particleSystem->CreatePointEmitter();
		particleSystem->PlayEmitter();
		m_particleSystems[PT_SOFT_CPU] = particleSystem;

		//create soft GPU particle system
		particleSystem = (MashParticleSystem*)m_device->GetSceneManager()->AddParticleSystem(0, "ParticleSystem", particleSettings, aPARTICLE_GPU_SOFT_DEFERRED, aLIGHT_TYPE_VERTEX);
		particleSystem->SetDiffuseTexture(m_device->GetRenderer()->GetTexture("smoke.PNG"));
		particleEmitter = particleSystem->CreatePointEmitter();
		particleSystem->PlayEmitter();
		m_particleSystems[PT_SOFT_GPU] = particleSystem;
#endif

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 40.0f),
			MashGUIUnit(0.0f, 500.0f), MashGUIUnit(0.0f, 200.0f));
		m_staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		UpdateText(PT_CPU);

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
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();
	}

	void ChangeParticleType(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			static int32 nextParticleSystem = 0;
			m_particleSystems[nextParticleSystem]->Detach();

			nextParticleSystem = ++nextParticleSystem % PT_COUNT;
			m_root->AddChild(m_particleSystems[nextParticleSystem]);

			UpdateText((PARTICLE_TYPE)nextParticleSystem);
		}
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

#ifdef USE_DEFERRED_PARTICLES
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_DEFERRED;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_NONE;
#else
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_X2;
#endif

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/ParticleDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Particle Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
