//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "../SupportLib/MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "UnitTest++.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

void TestNodes(MashDevice *device);
void TestLights(MashDevice *device);

class MainLoop : public mash::MashGameLoop
{
private:
	MashDevice *m_device;
	MashCamera *m_camera;
public:
	MainLoop(mash::MashDevice *device):m_device(device){}
	virtual ~MainLoop(){}

	bool Initialise()
	{
        TestNodes(m_device);
        TestLights(m_device);
        
		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(0, "Camera01");
		m_camera->SetZFar(1000);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 0, 0));
        
        
		return false;
	}

	bool Update(f32 dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_camera);
		
		return true;
	}

	void LateUpdate(f32 dt)
	{
	}

	void Render()
	{
		m_device->GetSceneManager()->CullScene(m_camera);

		m_device->GetSceneManager()->DrawScene();

		m_device->GetRenderer()->SetRenderTargetDefault();
	}
};



void TestNodes(MashDevice *device)
{
    MashSceneNode *n = device->GetSceneManager()->AddDummy(0, "node1");
    n->SetUserID(1);
    n = device->GetSceneManager()->AddDummy(0, "node2");
    n->SetUserID(2);
    n = device->GetSceneManager()->AddDummy(0, "node3");
    n->SetUserID(3);
    n = device->GetSceneManager()->AddDummy(0, "node4");
    n->SetUserID(4);
    n = device->GetSceneManager()->AddDummy(0, "node5");
    n->SetUserID(5);
    
    MashStringc uniqueName1, uniqueName2;
    device->GetSceneManager()->GenerateUniqueSceneNodeName(uniqueName1);
    device->GetSceneManager()->GenerateUniqueSceneNodeName(uniqueName2);
    
    n = device->GetSceneManager()->AddDummy(0, uniqueName1);
    n->SetUserID(6);
    n = device->GetSceneManager()->AddDummy(0, uniqueName2);
    n->SetUserID(7);
    
    CHECK(!uniqueName1.Empty());
    CHECK(!uniqueName2.Empty());
    CHECK(uniqueName1 != uniqueName2);
    
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(1));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(2));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(3));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(4));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(5));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(6));
    CHECK(device->GetSceneManager()->GetSceneNodeByUserID(7));
    CHECK(!device->GetSceneManager()->GetSceneNodeByUserID(327));
    
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node1"));
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node2"));
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node3"));
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node4"));
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node5"));
    CHECK(device->GetSceneManager()->GetSceneNodeByName(uniqueName1));
    CHECK(device->GetSceneManager()->GetSceneNodeByName(uniqueName2));
    CHECK(!device->GetSceneManager()->GetSceneNodeByName("nodasdsadade5"));
    
    CHECK(device->GetSceneManager()->GetSceneNodeCount() == 7);
    n = device->GetSceneManager()->GetSceneNodeByName("node2");
    n->Grab();
    CHECK(device->GetSceneManager()->GetSceneNodeByName("node2")->GetReferenceCount() == 2);
    device->GetSceneManager()->RemoveSceneNode(device->GetSceneManager()->GetSceneNodeByName("node2"));
    CHECK(n->GetReferenceCount() == 1);
    CHECK(!device->GetSceneManager()->GetSceneNodeByName("node2"));
    CHECK(!device->GetSceneManager()->GetSceneNodeByUserID(2));
    CHECK(device->GetSceneManager()->GetSceneNodeCount() == 6);
    CHECK(n->Drop());
    
    MashSceneNode *pnode = device->GetSceneManager()->GetSceneNodeByName(uniqueName1);
    MashSceneNode *cnode = device->GetSceneManager()->GetSceneNodeByName(uniqueName2);
    
    CHECK(pnode->GetChildCount() == 0);
    CHECK(cnode->GetChildCount() == 0);
    
    pnode->AddChild(cnode);
    
    CHECK(pnode->GetChildCount() == 1);
    CHECK(cnode->GetChildCount() == 0);
    
    cnode->Grab();
    CHECK(cnode->GetReferenceCount() == 3);
    device->GetSceneManager()->RemoveSceneNode(cnode);
    CHECK(!device->GetSceneManager()->GetSceneNodeByName(uniqueName2));
    CHECK(cnode->GetReferenceCount() == 1);
    CHECK(pnode->GetChildCount() == 0);
    CHECK(cnode->Drop());
    
    cnode = device->GetSceneManager()->GetSceneNodeByName("node3");
    pnode->AddChild(cnode);
    
    CHECK(pnode->GetChildCount() == 1);
    CHECK(cnode->GetChildCount() == 0);
    
    pnode->Grab();
    CHECK(pnode->GetReferenceCount() == 2);
    CHECK(cnode->GetReferenceCount() == 2);
    device->GetSceneManager()->RemoveSceneNode(pnode);
    CHECK(!device->GetSceneManager()->GetSceneNodeByName(uniqueName1));
    CHECK(pnode->GetReferenceCount() == 1);
    CHECK(pnode->GetChildCount() == 1);
    CHECK(pnode->Drop());
    
    CHECK(cnode->GetReferenceCount() == 1);
    
    device->GetSceneManager()->RemoveAllSceneNodes();
    CHECK(device->GetSceneManager()->GetSceneNodeCount() == 0);
}

void TestLights(MashDevice *device)
{
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 0);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    
    MashLight *l1 = device->GetSceneManager()->AddLight(0, "light1", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
    
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 1);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->IsDeferredRendererInitialised());
    
    l1->SetLightRendererType(aLIGHT_RENDERER_DEFERRED);
    
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 0);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->IsDeferredRendererInitialised());//should be false until after MashGameLoop::Init().
    
    l1->SetLightRendererType(aLIGHT_RENDERER_FORWARD);
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 1);
    
    l1->SetShadowsEnabled(true);
    CHECK(device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    /*CHECK(!device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredSpotShadowsEnabled());*/
    
    l1->SetLightRendererType(aLIGHT_RENDERER_DEFERRED);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredSpotShadowsEnabled());
    
    l1->SetLightType(aLIGHT_POINT);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredSpotShadowsEnabled());
    
    l1->SetLightType(aLIGHT_SPOT);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(device->GetSceneManager()->GetDeferredSpotShadowsEnabled());
    
    //check what happens when an active light is removed from the scene manager
    l1->SetLightRendererType(aLIGHT_RENDERER_FORWARD);
    device->GetSceneManager()->RemoveAllSceneNodes();
    CHECK(device->GetSceneManager()->GetSceneNodeCount() == 0);
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 0);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredSpotShadowsEnabled());
    
    l1 = device->GetSceneManager()->AddLight(0, "light1", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
    MashLight *l2 = device->GetSceneManager()->AddLight(0, "light1", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, false);
    CHECK(l1->IsMainForwardRenderedLight());
    CHECK(!l2->IsMainForwardRenderedLight());
    
    l2->SetLightRendererType(aLIGHT_RENDERER_FORWARD, true);
    CHECK(!l1->IsMainForwardRenderedLight());
    CHECK(l2->IsMainForwardRenderedLight());
    
    l1->SetShadowsEnabled(true);
    l2->SetShadowsEnabled(true);
    device->GetSceneManager()->RemoveAllSceneNodes();
    CHECK(device->GetSceneManager()->GetSceneNodeCount() == 0);
    CHECK(device->GetSceneManager()->GetForwardRenderedLightCount() == 0);
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetForwardRenderedShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredDirShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredPointShadowsEnabled());
    CHECK(!device->GetSceneManager()->GetDeferredSpotShadowsEnabled());
}

bool g_errorLogWasReceived = false;
struct sErrorHandler
{
	void LoggingCallback(const sLogEvent &e)
	{
		if (e.level == MashLog::aERROR_LEVEL_ERROR)
			g_errorLogWasReceived = true;
	}
};

MashDevice *g_device = 0;
MainLoop *g_mainLoop = 0;
sErrorHandler *g_errorHandler = 0;

struct sEngineStartup
{
    sEngineStartup()
    {
        if (!g_device)
        {
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
            
            deviceSettings.rootPaths.PushBack("../../../../../../Media/Materials");
            
            ////Only needed if GUI is used.
            //deviceSettings.rootPaths.push_back("../../Media/GUI");
            
            ////You can set up paths that debug material information will be written to.
            //deviceSettings.compiledShaderOutputDirectory = "../MaterialDebug";
            //deviceSettings.intermediateShaderOutputDirectory = "../MaterialDebug";

			g_errorHandler = new sErrorHandler;
			mash::MashLog::Instance()->AddReceiver(mash::MashLogEventFunctor(&sErrorHandler::LoggingCallback, g_errorHandler));
            
            //Creates the device
            g_device = CreateDevice(deviceSettings);
            
            if (g_device)
            {        
                g_device->SetWindowCaption("Unit Tests");
                
                g_mainLoop = MASH_NEW_COMMON MainLoop(g_device);
                g_device->SetGameLoop(g_mainLoop);
            }
        }
        else
        {
            g_device->Grab();
            g_mainLoop->Grab();
        }
    }
    
    ~sEngineStartup()
    {
		if (g_mainLoop)
			g_mainLoop->Drop();

		if (g_device)
			g_device->Drop();

		if (g_errorHandler)
			delete g_errorHandler;

		CHECK(g_errorLogWasReceived == false);
    }
};

SUITE(ArrayTest)
{
TEST(ArrayPushBack)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const uint32 arrayAmount = 1000;
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushBack(i);
    
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushFront(i);
}

TEST(ArrayPushBackPopback)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const uint32 arrayAmount = 100000;
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushBack(i);
    
    CHECK(testArray.Size() == arrayAmount);
    
    testArray.ShrinkToFit();
    CHECK(testArray.ReservedSize() == arrayAmount);
    
    for(uint32 i = 0; i < arrayAmount / 2; ++i)
        testArray.PopBack();
    
    while(!testArray.Empty())
        testArray.PopFront();
    
    CHECK(testArray.Size() == 0);
}

TEST(ArrayErase)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const uint32 arrayAmount = 100000;
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushBack(i);
    
    CHECK(testArray.Size() == arrayAmount);
    
    testArray.ShrinkToFit();
    CHECK(testArray.ReservedSize() == arrayAmount);
    
    testArray.Erase(testArray.End() - 1);
    CHECK(testArray[testArray.Size()-1] == arrayAmount-2);
    
    testArray.Erase(1);
    CHECK(testArray[1] == 2);
    
    MashArray<int32>::Iterator iter = testArray.Begin();
    testArray.Erase(iter);
    CHECK(*(testArray.Begin()) == 2);
    
    CHECK(testArray.Size() == arrayAmount-3);
}

TEST(ArraySearchErase)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const uint32 arrayAmount = 100000;
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushBack(i);
    
    CHECK(testArray.Size() == arrayAmount);
    
    testArray.Erase(testArray.Search(1));
    CHECK(testArray[1] == 2);
}

TEST(ArrayInsertAppend)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const int32 data[5] = {0, 2, 4, 7, 3};
    testArray.Insert(0, data, 2);
    CHECK(testArray.Size() == 2);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    
    testArray.Insert(2, data, 5);
    CHECK(testArray.Size() == 7);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    CHECK(testArray[2] == 0);
    CHECK(testArray[3] == 2);
    CHECK(testArray[4] == 4);
    CHECK(testArray[5] == 7);
    CHECK(testArray[6] == 3);
    
    testArray.Clear();
    testArray.Insert(0, data, 2);
    CHECK(testArray.Size() == 2);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    
    MashArray<int32>::Iterator iter = testArray.Insert(testArray.Begin() + 2, data, 5);
    CHECK(testArray.Size() == 7);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    CHECK(testArray[2] == 0);
    CHECK(testArray[3] == 2);
    CHECK(testArray[4] == 4);
    CHECK(testArray[5] == 7);
    CHECK(testArray[6] == 3);
    
    testArray.Insert(testArray.Begin() + 2, 99);
    CHECK(testArray[2] == 99);
    
    testArray.Clear();
    testArray.Append(data, 2);
    CHECK(testArray.Size() == 2);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    
    testArray.Append(data, 5);
    CHECK(testArray.Size() == 7);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    CHECK(testArray[2] == 0);
    CHECK(testArray[3] == 2);
    CHECK(testArray[4] == 4);
    CHECK(testArray[5] == 7);
    CHECK(testArray[6] == 3);
    
    testArray.Clear();
    testArray.Insert(0, data, 2);
    CHECK(testArray.Size() == 2);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    
    testArray.Insert(1, data, 5);
    CHECK(testArray.Size() == 7);
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 0);
    CHECK(testArray[2] == 2);
    CHECK(testArray[3] == 4);
    CHECK(testArray[4] == 7);
    CHECK(testArray[5] == 3);
    CHECK(testArray[6] == 2);
}

TEST(ArraySearch)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const uint32 arrayAmount = 100000;
    for(uint32 i = 0; i < arrayAmount; ++i)
        testArray.PushBack(i);
    
    CHECK(testArray.Size() == arrayAmount);
    
    CHECK(testArray.Contains(2));
    CHECK(testArray.Contains(232));
    CHECK(testArray.Contains(252));
    CHECK(testArray.Contains(23457));
    CHECK(testArray.Contains(32432));
    CHECK(testArray.Contains(99999));
    CHECK(!testArray.Contains(993242999));
    
    CHECK(testArray.Search(0) == testArray.Begin());
    CHECK(testArray.Search(1) == (testArray.Begin() + 1));
}

TEST(ArraySort)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    const int32 data[5] = {0, 2, 7, 7, 3};
    testArray.Insert(0, data, 5);
    CHECK(testArray.Size() == 5);
    
    testArray.Sort();
    CHECK(testArray[0] == 0);
    CHECK(testArray[1] == 2);
    CHECK(testArray[2] == 3);
    CHECK(testArray[3] == 7);
    CHECK(testArray[4] == 7);
}

TEST(ArrayReserve)
{
    MashArray<int32> testArray;
    UNITTEST_TIME_CONSTRAINT(60000);//1min
    
    testArray.Resize(2, 1);
    CHECK(testArray.Size() == 2);
    CHECK(testArray.ReservedSize() == 2);
    CHECK(testArray[0] == 1);
    CHECK(testArray[1] == 1);
    
    testArray.Clear();
    CHECK(testArray.Size() == 0);
    CHECK(testArray.ReservedSize() == 2);
    
    testArray.Reserve(1);
    CHECK(testArray.Size() == 0);
    CHECK(testArray.ReservedSize() == 2);//shouldnt decrease
    
    testArray.Resize(1);
    CHECK(testArray.Size() == 1);
    CHECK(testArray.ReservedSize() == 2);//shouldnt decrease
    
    testArray.Clear();
    testArray.Reserve(5);
    CHECK(testArray.Size() == 0);
    CHECK(testArray.Empty());
    CHECK(testArray.ReservedSize() == 5);
    
    const int32 data[5] = {0, 2, 7, 7, 3};
    testArray.Insert(0, data, 5);
    CHECK(testArray.Size() == 5);
    CHECK(testArray.ReservedSize() == 5);
    
    testArray.Resize(1);
    CHECK(testArray.Size() == 1);
    CHECK(testArray.ReservedSize() == 5);
    
    testArray.PopBack();
    CHECK(testArray.Size() == 0);
    CHECK(testArray.ReservedSize() == 5);
}
}

SUITE(ListTest)
{
    TEST(PushBack)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 100000;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushBack(i);
        
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushFront(i);
    }
    
    TEST(PushBackPopback)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 100000;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushBack(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        for(uint32 i = 0; i < arrayAmount / 2; ++i)
            testArray.PopBack();
        
        while(!testArray.Empty())
            testArray.PopFront();
        
        CHECK(testArray.Size() == 0);
    }
    
    TEST(Erase)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 100000;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushBack(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        testArray.Erase(--(testArray.End()));
        CHECK(testArray.Back() == arrayAmount-2);
        
        testArray.Erase(1);
        CHECK(*(testArray.Begin() + 1)== 2);
        
        MashList<int32>::Iterator iter = testArray.Begin();
        testArray.Erase(iter);
        CHECK(*(testArray.Begin()) == 2);
        
        CHECK(testArray.Size() == arrayAmount-3);
    }
    
    TEST(SearchErase)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 100000;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushBack(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        MashList<int32>::Iterator iter = testArray.Search(1);
        testArray.Erase(iter);
        CHECK(*(testArray.Begin() + 1) == 2);
    }
    
    TEST(InsertAppend)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const int32 data[5] = {0, 2, 4, 7, 3};
        testArray.Insert(testArray.Begin(), data[0]);
        testArray.Insert(testArray.Begin() + 1, data[1]);
        testArray.Insert(testArray.Begin() + 2, data[2]);
        testArray.Insert(testArray.Begin() + 3, data[3]);
        testArray.Insert(testArray.Begin() + 4, data[4]);
        
        CHECK(testArray.Size() == 5);

        MashList<int32>::Iterator iter = testArray.Insert(testArray.Begin() + 2, 99);
        CHECK(testArray.Size() == 6);
        CHECK(*(testArray.Begin() + 0) == 0);
        CHECK(*(testArray.Begin() + 1) == 2);
        CHECK(*(testArray.Begin() + 2) == 99);
        CHECK(*(testArray.Begin() + 3) == 4);
        CHECK(*(testArray.Begin() + 4) == 7);
        CHECK(*(testArray.Begin() + 5) == 3);
    }
    
    TEST(Search)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 100000;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.PushBack(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        CHECK(testArray.Contains(2));
        CHECK(testArray.Contains(232));
        CHECK(testArray.Contains(252));
        CHECK(testArray.Contains(23457));
        CHECK(testArray.Contains(32432));
        CHECK(testArray.Contains(99999));
        CHECK(!testArray.Contains(993242999));
        
        CHECK(testArray.Search(0) == testArray.Begin());
        CHECK(testArray.Search(1) == (testArray.Begin() + 1));
    }
    
    TEST(Sort)
    {
        MashList<int32> testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const int32 data[5] = {0, 2, 7, 7, 3};
        for(int32 i = 0; i < sizeof(data) / sizeof(int32); ++i)
            testArray.PushBack(data[i]);

        CHECK(testArray.Size() == 5);
        
        testArray.Sort();
        CHECK(*(testArray.Begin()) == 0);
        CHECK(*(testArray.Begin() + 1) == 2);
        CHECK(*(testArray.Begin() + 2) == 3);
        CHECK(*(testArray.Begin() + 3) == 7);
        CHECK(*(testArray.Begin() + 4) == 7);
        
        testArray.DeleteData();
        CHECK(testArray.Size() == 0);
        CHECK(testArray.Empty());
    }
}

SUITE(StringTest)
{
    TEST(PushBack)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 256;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Append(i);
        
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Insert(0, i);
    }
    
    TEST(PushBackPopback)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 256;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Append(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        testArray.ShrinkToFit();
        CHECK(testArray.ReservedSize() == arrayAmount+1/*null terminator*/);
        
        for(uint32 i = 0; i < arrayAmount / 2; ++i)
            testArray.PopBack();
        
        while(!testArray.Empty())
            testArray.PopFront();
        
        CHECK(testArray.Size() == 0);
    }
    
    TEST(Erase)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 256;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Append(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        testArray.ShrinkToFit();
        CHECK(testArray.ReservedSize() == arrayAmount+1/*null terminator*/);
        
        testArray.Erase(testArray.End() - 1);
        CHECK(testArray[testArray.Size()-1] == ((int8)arrayAmount-2));
        
        testArray.Erase(1);
        CHECK(testArray[1] == 2);
        
        CHECK(testArray.Size() == arrayAmount-2);
    }
    
    TEST(SearchErase)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 256;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Append(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        testArray.Erase(testArray.Find(1));
        CHECK(testArray[1] == 2);
    }
    
    TEST(InsertAppend)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const int8 data[5] = {0, 2, 4, 7, 3};
        testArray.Insert(0, data, 2);
        CHECK(testArray.Size() == 2);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        
        testArray.Insert(2, data, 5);
        CHECK(testArray.Size() == 7);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        CHECK(testArray[2] == 0);
        CHECK(testArray[3] == 2);
        CHECK(testArray[4] == 4);
        CHECK(testArray[5] == 7);
        CHECK(testArray[6] == 3);
        
        testArray.Clear();
        testArray.Insert(0, data, 2);
        CHECK(testArray.Size() == 2);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        
        MashStringc::Iterator iter = testArray.Insert(2, data, 5);
        CHECK(testArray.Size() == 7);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        CHECK(testArray[2] == 0);
        CHECK(testArray[3] == 2);
        CHECK(testArray[4] == 4);
        CHECK(testArray[5] == 7);
        CHECK(testArray[6] == 3);
        
        testArray.Insert(testArray.Begin() + 2, 99);
        CHECK(testArray[2] == 99);
        
        testArray.Clear();
        testArray.Append(data, 2);
        CHECK(testArray.Size() == 2);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        
        testArray.Append(data, 5);
        CHECK(testArray.Size() == 7);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        CHECK(testArray[2] == 0);
        CHECK(testArray[3] == 2);
        CHECK(testArray[4] == 4);
        CHECK(testArray[5] == 7);
        CHECK(testArray[6] == 3);
        
        testArray.Clear();
        testArray.Insert(0, data, 2);
        CHECK(testArray.Size() == 2);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 2);
        
        testArray.Insert(1, data, 5);
        CHECK(testArray.Size() == 7);
        CHECK(testArray[0] == 0);
        CHECK(testArray[1] == 0);
        CHECK(testArray[2] == 2);
        CHECK(testArray[3] == 4);
        CHECK(testArray[4] == 7);
        CHECK(testArray[5] == 3);
        CHECK(testArray[6] == 2);
    }
    
    TEST(Search)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        const uint32 arrayAmount = 200;
        for(uint32 i = 0; i < arrayAmount; ++i)
            testArray.Append(i);
        
        CHECK(testArray.Size() == arrayAmount);
        
        CHECK(testArray.Contains(2));
        CHECK(testArray.Contains(133));
        CHECK(testArray.Contains(45));
        CHECK(testArray.Contains(65));
        CHECK(testArray.Contains(2));
        CHECK(testArray.Contains(56));
        CHECK(!testArray.Contains(234));
    }
    
    TEST(Reserve)
    {
        MashStringc testArray;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        
        testArray.Resize(2, 1);
        CHECK(testArray.Size() == 2);
        CHECK(testArray[0] == 1);
        CHECK(testArray[1] == 1);
        
        testArray.Clear();
        CHECK(testArray.Size() == 0);
        
        testArray.Reserve(1);
        CHECK(testArray.Size() == 0);
        
        testArray.Resize(1);
        CHECK(testArray.Size() == 1);
        
        testArray.Clear();
        testArray.Reserve(5);
        CHECK(testArray.Size() == 0);
        CHECK(testArray.Empty());
        
        const int8 data[5] = {0, 2, 7, 7, 3};
        testArray.Insert(0, data, 5);
        CHECK(testArray.Size() == 5);
        
        testArray.Resize(1);
        CHECK(testArray.Size() == 1);
        
        testArray.PopBack();
        CHECK(testArray.Size() == 0);
    }
    
    TEST(Concat)
    {
        const int8 *whole = "Hello World!";
        
        MashStringc t1, t2, f;
        UNITTEST_TIME_CONSTRAINT(60000);//1min
        t1.Append(whole, 5);
        t2.Append(&whole[5]);
        
        f = t1 + t2;
        CHECK(f == whole);
    }
}

TEST_FIXTURE(sEngineStartup, FailSpectacularly)
{
	CHECK(g_device != 0);
    CHECK(true);
}

int main()
{        
    return UnitTest::RunAllTests();
}
