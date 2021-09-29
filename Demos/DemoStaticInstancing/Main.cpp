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

class MainLoop : public MashGameLoop, public MashRenderable
{
private:
	MashDevice *m_device;

	MashSceneNode *m_root;
	uint32 m_playerControl;
	MashCamera *m_camera;

	int32 m_maxBarrelCount;
	int32 m_currentBarrelCount;
	int32 m_barrelIncrementAmount;

	MashMaterial *m_barrelMaterial;
	MashMaterial *m_barrelHwInstancedMaterial;
	MashMaterial *m_barrelBatchMaterial;

	MashMeshBuffer *m_barrelHwInstanceMeshBuffer;
	MashMeshBuffer *m_barrelBatchMeshBuffer;
	MashMesh *m_originalBarrelMesh;

	MashArray<MashMatrix4> m_barrelInstanceData;

	MashGenericArray m_barrelVertexOnlyData;


	MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;
	uint32 m_hwInstanceUpdateTime;
	uint32 m_batchUpdateTime;
	
	enum eRENDER_BUFFER_TYPE
	{
		RB_NORMAL,
		RB_HW_INSTANCED,
		RB_BATCH
	};

	MashAABB m_nullBounds;

	eRENDER_BUFFER_TYPE m_bufferToRender;
public:
	MainLoop(MashDevice *device):m_device(device), m_bufferToRender(RB_NORMAL), m_maxBarrelCount(1000), m_currentBarrelCount(1), m_barrelIncrementAmount(20),
	m_batchUpdateTime(0.0f), m_hwInstanceUpdateTime(0.0f), m_barrelBatchMeshBuffer(0), m_barrelHwInstanceMeshBuffer(0){}
	virtual ~MainLoop()
	{
		if (m_barrelBatchMeshBuffer)
		{
			m_barrelBatchMeshBuffer->Drop();
			m_barrelBatchMeshBuffer = 0;
		}

		if (m_barrelHwInstanceMeshBuffer)
		{
			m_barrelHwInstanceMeshBuffer->Drop();
			m_barrelHwInstanceMeshBuffer = 0;
		}
	}

	void UpdateText()
	{
		MashStringc text;
		switch(m_bufferToRender)
		{
		case RB_NORMAL:
			text = "Mode : 1 draw per instance (standard)\n";
			break;
		case RB_HW_INSTANCED:
			text = "Mode : Hardware instanced\n";
			break;
		case RB_BATCH:
			text = "Mode : Batched\n";
			break;
		};

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "\Instance count : %d\n", m_currentBarrelCount);
		text += buffer;

		text += "\nTime taken to build batches (ms):\n";
        helpers::PrintToBuffer(buffer, sizeof(buffer), "\t- HW instance : %d\n", m_hwInstanceUpdateTime);
		text += buffer;
        helpers::PrintToBuffer(buffer, sizeof(buffer), "\t- Batch : %d\n", m_batchUpdateTime);
		text += buffer;

		text += "\nPress Z to decrease instance count\n\
Press X to increase instance count\n\
Press C to change rendering type";

		m_staticText->SetText(text);
	}

	void CreateBatch()
	{
		/*
			This mesh has both vertex and index buffers. Which means for each barrel batched, we need to increment
			the values stored in the index buffer by:
				indexValue += (currentBarrel * numberOfIndices)

			Here we've choosen to create just a vertex buffer without the index buffer. We loop through all the indices
			and extract the triangle data.
		*/
		uint32 vertexStride = m_barrelMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);
		m_barrelVertexOnlyData.Reserve(m_originalBarrelMesh->GetIndexCount() * vertexStride);

		int8 *indexBufferStart = (int8*)m_originalBarrelMesh->GetRawIndices().Pointer();
		int8 *vertexBufferStart = (int8*)m_originalBarrelMesh->GetRawVertices().Pointer();
		for(uint32 idx = 0; idx < m_originalBarrelMesh->GetIndexCount(); ++idx)
		{
			if (m_originalBarrelMesh->GetIndexFormat() == aFORMAT_R32_UINT)
				m_barrelVertexOnlyData.Append(&vertexBufferStart[vertexStride * ((uint32*)indexBufferStart)[idx]], vertexStride);
			else
				m_barrelVertexOnlyData.Append(&vertexBufferStart[vertexStride * ((uint16*)indexBufferStart)[idx]], vertexStride);
		}

		/*
			this array is used to initialize the mesh buffer.
			We reserve enough memory for the greatest size we will need.
		*/
		MashGenericArray batchInitData;
		batchInitData.Reserve(m_barrelVertexOnlyData.GetCurrentSize() * m_maxBarrelCount);
		for(uint32 currentBarrel = 0; currentBarrel < m_currentBarrelCount; ++currentBarrel)
			batchInitData.Append(m_barrelVertexOnlyData.Pointer(), m_barrelVertexOnlyData.GetCurrentSize());

		/*
			If this buffer was never going to change then set the usage to STATIC. Rendering is optimized for Static rendering.
			Dynamic is needed in this case so we can update the vertices if requested.
			Performance of this buffer will be affected because its dynamic.
		*/
		sVertexStreamInit meshBufferInit;
		meshBufferInit.data = batchInitData.Pointer();
		meshBufferInit.dataSizeInBytes = m_barrelVertexOnlyData.GetCurrentSize() * m_maxBarrelCount;//batchInitData.GetCurrentSize();
		meshBufferInit.usage = aUSAGE_DYNAMIC; 

		//only using a vertex buffer
		m_barrelBatchMeshBuffer = m_device->GetRenderer()->CreateMeshBuffer(&meshBufferInit, 
			1, 
			m_barrelBatchMaterial->GetVertexDeclaration());
	}

	void CreateHWInstanceBatch()
	{
		/*
			Notice with HW instancing the first stream contains only 1 copy of the original vertex
			and index data.
			The second stream then contains 1 element per instance.
		*/

		sVertexStreamInit meshBufferInit[2];
		meshBufferInit[0].data = m_originalBarrelMesh->GetRawVertices().Pointer();
		meshBufferInit[0].dataSizeInBytes = m_originalBarrelMesh->GetRawVertices().GetCurrentSize();
		meshBufferInit[0].usage = aUSAGE_STATIC;
		meshBufferInit[1].data = &m_barrelInstanceData[0];
		meshBufferInit[1].dataSizeInBytes = sizeof(MashMatrix4) * m_maxBarrelCount;
		meshBufferInit[1].usage = aUSAGE_DYNAMIC; 

		m_barrelHwInstanceMeshBuffer = m_device->GetRenderer()->CreateMeshBuffer(meshBufferInit, 
			2, 
			m_barrelHwInstancedMaterial->GetVertexDeclaration(), 
			m_originalBarrelMesh->GetRawIndices().Pointer(), 
			m_originalBarrelMesh->GetIndexCount(),
			m_originalBarrelMesh->GetIndexFormat(), 
			aUSAGE_STATIC);
	}

	bool Initialise()
	{
		srand(time(NULL));

		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_C);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::ChangeRenderingType, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_Z);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+2, actions, MashInputEventFunctor(&MainLoop::DecreaseInstanceCount, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_X);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+3, actions, MashInputEventFunctor(&MainLoop::IncreaseInstanceCount, this));

		/*
			in this example we are not rendering or updating the barrel entity,
			so we dont add it to the scene.
		*/
		m_root = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");

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

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BarrelMaterials.mtl");

		m_barrelMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BarrelMaterial");
		if (!m_barrelMaterial)
			return true;

		m_barrelHwInstancedMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BarrelHWInstanceMaterial");
		if (!m_barrelHwInstancedMaterial)
			return true;

		m_barrelBatchMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BarrelBatchMaterial");
		if (!m_barrelBatchMaterial)
			return true;

		MashList<MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("Barrel.nss", rootNodes, loadSettings);	

		MashEntity *barrelEntity = (MashEntity*)rootNodes.Front();	

		/*
			In this example, all barrel materials have the exact same vertex declaration at stream 0 (the geometry stream).
			That is, position rgb32float, normal rgb32float, texcoords rg32float.

			Because of this, we can use the same stream to render all the instancing examples.

			If this information was different across the material then we would need to create a different mesh for each 
			using MashMeshBuilder::ChangeVertexFormat() or MashMesh::Clone()
		*/
		barrelEntity->SetMaterialToAllSubEntities(m_barrelMaterial);
		m_originalBarrelMesh = barrelEntity->GetModel()->GetMesh(0);

        //Holds all the barrel instance world matrices
        m_barrelInstanceData.Resize(m_maxBarrelCount, MashMatrix4());
        
		CreateBatch();
        
		CreateHWInstanceBatch();

		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
			MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 320.0f));
		m_staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		UpdateText();
		
		return false;
	}

	void UpdateInstanceCount()
	{
		int32 areaHalfWidth = 1000;
		int32 areaHalfLength = 1000;

		/*
			Update instance positions
		*/
		for(uint32 i = 0; i < m_currentBarrelCount; ++i)
		{
			m_barrelInstanceData[i].SetTranslation(MashVector3(math::RandomInt(-areaHalfWidth, areaHalfWidth), 
                                                              0, math::RandomInt(-areaHalfLength, areaHalfLength)));
		}

		/*
			We already know the location and stride of the position element in the vertex declaration because
			we created that within the barrel material.
			If this information was unknown then you could use MashVertex::GetVertexElements()
		*/
		uint32 positionElementLocation = 0;
		uint32 barrentIndexCount = m_originalBarrelMesh->GetIndexCount();
		uint32 barrelVertexStride = m_originalBarrelMesh->GetVertexDeclaration()->GetStreamSizeInBytes(0);
		uint32 barrelStride = barrentIndexCount * barrelVertexStride;

		uint32 timeStart = m_device->GetTimer()->GetTimeSinceProgramStart();

		/*
			Update standard batch
		*/
		int8 *batchVertexPtr;
		m_barrelBatchMeshBuffer->GetVertexBuffer()->Lock(aLOCK_WRITE_DISCARD, (void**)&batchVertexPtr);
		
		const int8 *originalVerticesPtr = (int8*)m_barrelVertexOnlyData.Pointer();
		for(uint32 currentBarrel = 0; currentBarrel < m_currentBarrelCount; ++currentBarrel)
		{
			for(uint32 vert = 0; vert < barrentIndexCount; ++vert)
			{
				const int8 *orignalMeshVertexStart = &originalVerticesPtr[barrelVertexStride * vert];
				int8 *batchVertexStart = &batchVertexPtr[(barrelVertexStride * vert) + (currentBarrel * barrelStride)];

				//copy over vertex data (pos, normal, texcoords)
				memcpy(batchVertexStart, orignalMeshVertexStart, barrelVertexStride);

				//now modify just the position element
				const MashVector3 *originalPosition = (MashVector3*)(orignalMeshVertexStart + positionElementLocation);
				MashVector3 *batchBarrelPosition = (MashVector3*)(batchVertexStart + positionElementLocation);

				batchBarrelPosition->x = originalPosition->x + m_barrelInstanceData[currentBarrel].m41;
				batchBarrelPosition->y = originalPosition->y;
				batchBarrelPosition->z = originalPosition->z + m_barrelInstanceData[currentBarrel].m43;
			}
		}

		m_barrelBatchMeshBuffer->GetVertexBuffer()->Unlock();

		uint32 timeEnd = m_device->GetTimer()->GetTimeSinceProgramStart();
		m_batchUpdateTime = timeEnd - timeStart;

		/*
			Update HW instance batch
		*/
		MashMatrix4 *hwInstanceData;

		timeStart = m_device->GetTimer()->GetTimeSinceProgramStart();

		m_barrelHwInstanceMeshBuffer->GetVertexBuffer(1)->Lock(aLOCK_WRITE_DISCARD, (void**)&hwInstanceData);
		for(uint32 currentBarrel = 0; currentBarrel < m_currentBarrelCount; ++currentBarrel)
		{
			hwInstanceData[currentBarrel] = m_barrelInstanceData[currentBarrel];
		}
		m_barrelHwInstanceMeshBuffer->GetVertexBuffer(1)->Unlock();

		timeEnd = m_device->GetTimer()->GetTimeSinceProgramStart();
		m_hwInstanceUpdateTime = timeEnd - timeStart;
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

		/*
			Here we add this Renderable class to the render queue so that our custom batches
			still receive lighting and shadow data.

			These objects are not culled in anyway by the engine. If culling is needed, materials
			allow you to add a custom render step. These are called after a scene node has passed the culling test
			and before a scene node is rendered.

			The high level pass param allows you to choose what group the objects will be rendered with. Most
			likely aHLPASS_SCENE will always be used.

			The render stage allows the objects to be considered for shadow casting. If that is desired, then
			call this function twice, once to add it to the shadow casting pass, and again to render
			into the normal scene. In this case we are only rendering into the scene pass.
		*/
        m_device->GetSceneManager()->AddRenderableToRenderQueue(this, aHLPASS_SCENE, aRENDER_STAGE_SCENE);

        m_device->GetSceneManager()->DrawScene();

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();
	}

	/*
		This will be called from within MashSceneManager::DrawScene()
	*/
	void Draw()
	{
		switch(m_bufferToRender)
		{
		case RB_NORMAL:
			{
				for(uint32 i = 0; i < m_currentBarrelCount; ++i)
				{
					//set the world matrix for the effect auto
					m_device->GetRenderer()->GetRenderInfo()->SetWorldTransform(m_barrelInstanceData[i]);
					/*
						The material needs to be set for each object otherwise effect autos will not be updated per object.
					*/
					if (m_barrelMaterial->OnSet())
					{
						m_device->GetRenderer()->DrawIndexedList(m_originalBarrelMesh->GetMeshBuffer(),
							m_originalBarrelMesh->GetVertexCount(),
							m_originalBarrelMesh->GetIndexCount(),
							m_originalBarrelMesh->GetPrimitiveCount(),
							m_originalBarrelMesh->GetPrimitiveType());
					}
				}

				break;
			}
		case RB_HW_INSTANCED:
			{
				if (m_barrelHwInstancedMaterial->OnSet())
				{
					m_device->GetRenderer()->DrawIndexedInstancedList(m_barrelHwInstanceMeshBuffer, 
						m_originalBarrelMesh->GetVertexCount(),
						m_originalBarrelMesh->GetIndexCount(),
						m_originalBarrelMesh->GetPrimitiveCount(),
						m_originalBarrelMesh->GetPrimitiveType(),
						m_currentBarrelCount);
				}

				break;
			}
		case RB_BATCH:
			{
				if (m_barrelBatchMaterial->OnSet())
				{
					m_device->GetRenderer()->DrawVertexList(m_barrelBatchMeshBuffer,
						m_originalBarrelMesh->GetIndexCount() * m_currentBarrelCount,
						m_originalBarrelMesh->GetPrimitiveCount() * m_currentBarrelCount,
						m_originalBarrelMesh->GetPrimitiveType());
				}

				break;
			}
		}
	}

	MashMaterial* GetMaterial()const
	{
		switch(m_bufferToRender)
		{
		case RB_NORMAL:
			{
				return m_barrelMaterial;
			}
		case RB_HW_INSTANCED:
			{
				return m_barrelHwInstancedMaterial;
			}
		case RB_BATCH:
			{
				return m_barrelBatchMaterial;
			}
		}

		return 0;
	}

	const MashAABB& GetWorldBoundingBox()const
	{
		/*
			Not needed in this case, but we need to return something.

			This is mainly used to determine depth for transparent objects.
		*/
		return m_nullBounds;
	}

	const mash::MashAABB& GetTotalWorldBoundingBox()const
	{
		return m_nullBounds;
	}

	void ChangeRenderingType(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			static int32 renderingTypeCounter = 1;

			m_bufferToRender = eRENDER_BUFFER_TYPE(renderingTypeCounter++ % 3);

			UpdateText();
		}
	}

	void DecreaseInstanceCount(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			m_currentBarrelCount = math::Max(m_currentBarrelCount-m_barrelIncrementAmount, 1);
			UpdateInstanceCount();
			UpdateText();
		}
	}

	void IncreaseInstanceCount(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			m_currentBarrelCount = math::Min(m_currentBarrelCount+m_barrelIncrementAmount, m_maxBarrelCount);
			UpdateInstanceCount();
			UpdateText();
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
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");

    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/StaticInstancingDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Static Instancing Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
