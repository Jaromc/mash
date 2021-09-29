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

MashMatrix4 g_lastViewProjection;
class LastViewProjectionEffectAuto : public MashAutoEffectParameter
{
public:
	LastViewProjectionEffectAuto():MashAutoEffectParameter(){}
	~LastViewProjectionEffectAuto(){}

	void OnSet(const MashRenderInfo *renderInfo, 
		MashEffect *effect, 
		MashEffectParamHandle *parameter,
		uint32 index)
	{
		effect->SetMatrix(parameter, &g_lastViewProjection);
	}

	const int8* GetParameterName()const{return "lastViewProjection";}
};

class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;
	MashSceneNode *m_root;
	MashCamera *m_camera;
	uint32 m_playerControl;
	MashMaterial *m_motionBlurMaterial;
	MashRenderSurface *m_renderSurface;
	MashGUIStaticText *m_fpsText;
public:
	MainLoop(MashDevice *device):m_device(device), m_renderSurface(0){}
	virtual ~MainLoop()
	{
		if (m_renderSurface)
		{
			m_renderSurface->Drop();
			m_renderSurface = 0;
		}
	}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BaseMaterial.mtl");
		MashMaterial *baseMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BaseMaterial");
		if (!baseMaterial)
			return true;

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("MotionBlurMaterial.mtl");
		m_motionBlurMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("MotionBlurMaterial");
		if (!m_motionBlurMaterial)
			return true;

		eFORMAT renderTargetFormat = aFORMAT_RGBA32_FLOAT;
		m_renderSurface = m_device->GetRenderer()->CreateRenderSurface(-1, -1, &renderTargetFormat, 1, 1, aDEPTH_OPTION_SHARE_MAIN_DEPTH);
		if (!m_renderSurface)
			return true;

		m_root = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera01");
		m_camera->SetZFar(1000);
		m_camera->SetZNear(0.001f);
		m_camera->SetPosition(MashVector3(0, 0, -1));
        
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 200.0f);
        m_camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		light->SetDefaultDirectionalLightSettings(MashVector3(-1.0f,-0.0f,0.0f));
		light->SetAmbient(sMashColour4(0.5f, 0.5f, 0.5f, 1.0f));
		light->SetRange(1000);
		light->SetShadowsEnabled(false);

		//Create some scene objects
		MashMesh *cubeMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreateCube(cubeMesh, 5, 5, 5, baseMaterial->GetVertexDeclaration());
		MashModel *cubeModel = m_device->GetSceneManager()->CreateModel();
		cubeModel->Append(&cubeMesh);
		cubeMesh->Drop();
		MashEntity *mainCube = m_device->GetSceneManager()->AddEntity(m_root, cubeModel, "CubeEntity");
		cubeModel->Drop();
		mainCube->SetMaterialToAllSubEntities(baseMaterial);

		for(uint32 i = 0; i < 20; ++i)
		{
			//create some random scene objects
			MashEntity *newInstance = (MashEntity*)m_device->GetSceneManager()->AddInstance(mainCube, m_root, "Instance");
			int32 areaHalfWidth = 200;
			int32 areaHalfLength = 200;
			newInstance->SetPosition(MashVector3(math::RandomInt(-areaHalfWidth, areaHalfWidth), 
				0, math::RandomInt(-areaHalfLength, areaHalfLength)));
		}

		//force update the camera values so we have valid a viewProj matrix
		m_device->GetSceneManager()->UpdateScene(0.1f, m_camera);
		g_lastViewProjection = m_camera->GetView() * m_camera->GetProjection();

		/*
			Here we add an effect auto for our post processing effect.
			This will be called each time the effect is set and the data will
			be sent to the gpu.
		*/
		MashAutoEffectParameter *effectParam = MASH_NEW_COMMON LastViewProjectionEffectAuto();
		m_device->GetRenderer()->GetMaterialManager()->RegisterAutoParameterHandler(effectParam);
		effectParam->Drop();

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
        /*
            Note, SetRenderTargetDefault() is the last target we set in this function. That buffer
            will have its depth and colour surfaces cleared automatically before entering this function.
        */
        
		m_device->GetSceneManager()->CullScene(m_root);

		m_device->GetRenderer()->SetRenderTarget(m_renderSurface);
        
        /*
            We are sharing the default depth buffer and it has already been cleared, so only clear 
            the colour target.
        */
		m_device->GetRenderer()->ClearTarget(aCLEAR_TARGET, sMashColour4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);

		m_device->GetSceneManager()->DrawScene();
		
		m_device->GetRenderer()->SetRenderTargetDefault();

		m_motionBlurMaterial->SetTexture(0, m_renderSurface->GetTexture(0));
		m_motionBlurMaterial->SetTexture(1, m_device->GetSceneManager()->GetDeferredDepthMap());

		if (m_motionBlurMaterial->OnSet() == aMASH_OK)
			m_device->GetRenderer()->DrawFullScreenQuad();

		g_lastViewProjection = m_camera->GetView() * m_camera->GetProjection();

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
	deviceSettings.enableVSync = true;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_DEFERRED;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/PostProcessingDemo");

	//Creates the device
	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Post Processing Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
