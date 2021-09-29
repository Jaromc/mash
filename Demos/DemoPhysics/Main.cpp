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

	MashEntity *m_bulletEntity;
	MashCamera *m_camera;

	MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;

	/*
		Collision shapes can be reused between multiple rigidbodies.
		So we create only one and store it fo the next created.
	*/
	MashPhysicsCollisionShape *m_bulletCollisionShape;
public:
	MainLoop(MashDevice *device):m_device(device), m_bulletCollisionShape(0){}
	virtual ~MainLoop()
	{
		if (m_bulletCollisionShape)
			m_bulletCollisionShape->Drop();
	}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		m_device->GetInputManager()->SetPlayerActionCallback(m_playerControl, inputContext, aINPUTMAP_FIRE, 
			MashInputEventFunctor(&MainLoop::Fire, this));

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BaseMaterial.mtl");
		MashMaterial *baseMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BaseMaterial");
		if (!baseMtrl)
			return true;

		MashList<MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("PhysicsScene.nss", rootNodes, loadSettings);

		m_root = rootNodes.Front();	

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		m_camera->SetZFar(2000);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 60, -200));
        
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 500.0f);
        m_camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		MashVector3 lightDir(-1.0f,-0.5f,0.0f);
		lightDir.Normalize();
		light->SetDefaultDirectionalLightSettings(lightDir);
		light->SetRange(1000);
		light->SetShadowsEnabled(false);

		m_device->GetPhysicsManager()->SetGravity(MashVector3(0,-20.0f,0));

		/*
			Here we create the collision shapes and rigid bodies.

			Notice we only pass a model when creating collision shapes. Collision shapes can be, and should be
			reused among rigidbodies where possible.
		*/
		MashList<MashSceneNode*>::ConstIterator childIter = m_root->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator childIterEnd = m_root->GetChildren().End();
		for(; childIter != childIterEnd; ++childIter)
		{
			if ((*childIter)->GetNodeType() == aNODETYPE_ENTITY)
			{
				MashEntity *curEntity = (MashEntity*)(*childIter);
				
				if ("Landscape" == curEntity->GetNodeName())
				{
					/*
						We know the landscape is mountainous therefore a convex collision shape will not do.
						Creating a triangle collision mesh allows for concave meshes. These objects also share
						data from a meshes triangle buffer to reduce its memory footprint.
					*/
					MashPhysicsCollisionShape *newShape = m_device->GetPhysicsManager()->CreateStaticTriangleCollisionShape(curEntity->GetModel());

					sRigisBodyConstruction rigidBodyConstructor;
					rigidBodyConstructor.mass = 0.0f;
					rigidBodyConstructor.friction = 1.0f;
                    rigidBodyConstructor.restitution = 0.0f;
					m_device->GetPhysicsManager()->AddRigidBody(curEntity, rigidBodyConstructor, newShape);

					newShape->Drop();
				}
				else
				{
					/*
						This creates a convex hull around the entity.

						If you know the size and shape of an entity then it would be better
						to create the collision shape from a primitive. Primtives provide faster
						collision detection and use less memory. See the bullet creation below for an example.
					*/
					MashPhysicsCollisionShape *newShape = m_device->GetPhysicsManager()->CreateCollisionShape(curEntity->GetModel());

					sRigisBodyConstruction rigidBodyConstructor;
					rigidBodyConstructor.mass = 1.0f;
					rigidBodyConstructor.friction = 1.0f;
                    rigidBodyConstructor.restitution = 0.0f;
					m_device->GetPhysicsManager()->AddRigidBody(curEntity, rigidBodyConstructor, newShape);

					newShape->Drop();
				}
			}
		}

		/*
			Create the cube bullets.
			This is simply used to clone new bullets into existance. This
			bullet is not added to the scene.
		*/
		MashMesh *bulletMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreateCube(bulletMesh, 5, 5, 5, baseMtrl->GetVertexDeclaration());
		MashModel *bulletModel = m_device->GetSceneManager()->CreateModel();
		bulletModel->Append(&bulletMesh);
		bulletMesh->Drop();
		m_bulletEntity = m_device->GetSceneManager()->AddEntity(0, bulletModel, "BulletEntity");
		bulletModel->Drop();

		m_bulletEntity->SetMaterialToAllSubEntities(baseMtrl);

		/*
			Important optional note, if the collision mesh were to be created from this meshes vertices
			then we would need to create a triangle buffer and assign it to the mesh. This buffer
			holds sorted triangle data used by the physics system and for some standard engine collision
			methods.
			But, in this case we are using simple primitives for collision so the triangle buffer
			isn't necessary.
			Note for imported meshes, the triangle buffer is usually created from the modelling package
			it came from.

			bulletMesh->SetTriangleBuffer(m_device->GetSceneManager()->CreateTriangleBuffer(bulletMesh));
		*/


		/*
			Create a collision shape for the bullet rigid bodies. This will
			be shared among all bullets. This is more efficient than creating
			a new shape for each bullet.
		*/
		sCollisionObjectConstruction collisionObj;
		collisionObj.type = aPHYSICS_SHAPE_CUBE;
		//my bullets are 5 in size so the half width is 2.5
		collisionObj.cubeHalfExt.Set(2.5f, 2.5f, 2.5f);
		m_bulletCollisionShape = m_device->GetPhysicsManager()->CreateCollisionShape(collisionObj);

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
			MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.0f, 120.0f));
		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);
MashStringc text= "\Controls :\nW,S move forward/backward\n\
A,D move left/right\n\
MB 1 shoot\n\
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

		m_device->GetPhysicsManager()->DrawDebug();

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();

	}

	void Fire(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			static uint32 bulletCounter = 0;
			int8 nameBuffer[256];
			sprintf(nameBuffer, "BulletEntity_%d", bulletCounter++);

			MashEntity *newBullet = (MashEntity*)m_device->GetSceneManager()->AddInstance(m_bulletEntity, m_root, nameBuffer);

			//bullet direction
			MashVector3 bulletDirection(0.0f, 0.0f, 1.0f);
			MashMatrix4 invView = m_camera->GetView();
			bulletDirection = invView.Invert().TransformRotation(bulletDirection);
			bulletDirection.Normalize();

			//start at the camera position
			newBullet->SetPosition(m_camera->GetWorldTransformState().translation + (bulletDirection * 10.0f), true);
			
			//create the physics object
			sRigisBodyConstruction rigidBodyConstructor;
			rigidBodyConstructor.mass = 1.0f;
			rigidBodyConstructor.friction = 2.0f;
			rigidBodyConstructor.restitution = 0.5f;

			MashPhysicsRigidBody *newPhysicsBullet = m_device->GetPhysicsManager()->AddRigidBody(newBullet, rigidBodyConstructor, m_bulletCollisionShape);

			//shoot the bullet
			newPhysicsBullet->ApplyImpulse(bulletDirection * 100.0f);
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
	deviceSettings.physicsManagerFunctPtr = CreateMashPhysics;
	deviceSettings.fullScreen = false;
	deviceSettings.screenWidth = 800;
	deviceSettings.screenHeight = 600;
	deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/PhysicsDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Physics Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
