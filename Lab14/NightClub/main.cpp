#define CLIENT_DESCRIPTION "Light"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>


using namespace Ogre;

class InputController : public FrameListener, 
  public OIS::KeyListener, 
  public OIS::MouseListener
{

public:
  InputController(Root* root, OIS::Keyboard *keyboard, OIS::Mouse *mouse) : mRoot(root), mKeyboard(keyboard), mMouse(mouse)
  {
    mSceneMgr = root->getSceneManager("main");
    mLightD = mSceneMgr->getLight("LightD");
    mLightP = mSceneMgr->getLight("LightP");
    mLightS = mSceneMgr->getLight("LightS");

	mProfessorState[0] = mSceneMgr->getEntity("Professor")->getAnimationState("Idle");
	mProfessorState[1] = mSceneMgr->getEntity("Professor1")->getAnimationState("Walk");
	mProfessorState[2] = mSceneMgr->getEntity("Professor2")->getAnimationState("Run");
	mProfessorState[3] = mSceneMgr->getEntity("Professor3")->getAnimationState("Climb");
	mProfessorState[4] = mSceneMgr->getEntity("Professor4")->getAnimationState("Run");

	for (auto anim : mProfessorState) {
		anim->setLoop(true);
		anim->setEnabled(true);
	}

	mProfessorNodes[0] = mSceneMgr->getSceneNode("ProfessorYaw");
	mProfessorNodes[1] = mSceneMgr->getSceneNode("Professor1");
	mProfessorNodes[2] = mSceneMgr->getSceneNode("Professor2");
	mProfessorNodes[3] = mSceneMgr->getSceneNode("Professor3");
	mProfessorNodes[4] = mSceneMgr->getSceneNode("Professor4");


    mCameraYaw = mSceneMgr->getSceneNode("CameraYaw");
    mCameraPitch = mSceneMgr->getSceneNode("CameraPitch");
    mCameraHolder = mSceneMgr->getSceneNode("CameraHolder");

	mNightNode = mSceneMgr->getSceneNode("Night");

    mContinue = true;

    keyboard->setEventCallback(this);
    mouse->setEventCallback(this);
  }


  bool frameStarted(const FrameEvent &evt)
  {
    mKeyboard->capture();
    mMouse->capture();
	for (auto anim : mProfessorState) 
		anim->addTime(evt.timeSinceLastFrame);
	
	for (auto node : mProfessorNodes)
		node->rotate(Vector3::UNIT_Y, Degree(90 * evt.timeSinceLastFrame));
	
	mNightNode->rotate(Vector3::UNIT_Y, Degree(180 * evt.timeSinceLastFrame));

    return mContinue;
  }

  // Key Linstener Interface Implementation

  bool keyPressed( const OIS::KeyEvent &evt )
  {
    // Fill Here -----------------------------------------------
	  switch (evt.key)
	  {
	  case OIS::KC_A:
	  {
		  static float a = 0.f;
		  a = (a >= 1.f) ? 0.f : a + 0.1f;
		  mSceneMgr->setAmbientLight(ColourValue(a, a, a));
	  }
	  break;

	  case OIS::KC_D: mLightD->setVisible(!mLightD->getVisible()); break;
	  case OIS::KC_P: mLightP->setVisible(!mLightP->getVisible()); break;
	  case OIS::KC_S: mLightS->setVisible(!mLightS->getVisible()); break;
	  }
    // ---------------------------------------------------------

    return true;

  }

  bool keyReleased( const OIS::KeyEvent &evt )
  {

    switch(evt.key) 
    {
    case OIS::KC_ESCAPE: mContinue = false; break;
    }

    return true;
  }


  // Mouse Listener Interface Implementation

  bool mouseMoved( const OIS::MouseEvent &evt )
  {
    mCameraYaw->yaw(Degree(-evt.state.X.rel));
    mCameraPitch->pitch(Degree(-evt.state.Y.rel));

    mCameraHolder->translate(Ogre::Vector3(0, 0, -evt.state.Z.rel * 0.1f));

    return true;
  }

  bool mousePressed( const OIS::MouseEvent &evt, OIS::MouseButtonID id )
  {
    return true;
  }

  bool mouseReleased( const OIS::MouseEvent &evt, OIS::MouseButtonID id )
  {
    return true;
  }


private:
  bool mContinue;
  Ogre::Root* mRoot;
  Ogre::SceneManager* mSceneMgr;
  OIS::Keyboard* mKeyboard;
  OIS::Mouse* mMouse;

  Ogre::Light *mLightP, *mLightD, *mLightS;


  Ogre::AnimationState* mProfessorState[5];
  SceneNode* mProfessorNodes[5];
//  Ogre::AnimationState* mIdleState;

  SceneNode* mCharacterRoot;
  SceneNode* mCharacterYaw;

  SceneNode* mCameraHolder;
  SceneNode* mCameraYaw;
  SceneNode* mCameraPitch;

  SceneNode* mNightNode;

  Ogre::Vector3 mCharacterDirection;

  bool mRotating;  
  Quaternion mSrcQuat, mDestQuat;  
  float mRotatingTime;
};







class LectureApp {





public:

  LectureApp() {}

  ~LectureApp() {}

  void go(void)
  {
    // OGRE의 메인 루트 오브젝트 생성
#if !defined(_DEBUG)
    mRoot = new Root("plugins.cfg", "ogre.cfg", "ogre.log");
#else
    mRoot = new Root("plugins_d.cfg", "ogre.cfg", "ogre.log");
#endif


    // 초기 시작의 컨피규레이션 설정 - ogre.cfg 이용
    if (!mRoot->restoreConfig()) {
      if (!mRoot->showConfigDialog()) return;
    }

    mWindow = mRoot->initialise(true, CLIENT_DESCRIPTION " : Copyleft by Dae-Hyun Lee 2010");

    mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "main");

    mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

    mCamera = mSceneMgr->createCamera("main");

    mViewport = mWindow->addViewport(mCamera);
    mViewport->setBackgroundColour(ColourValue(0.0f,0.0f,0.0f));
    mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


    ResourceGroupManager::getSingleton().addResourceLocation("resource.zip", "Zip");
    ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem");
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


    _drawGridPlane();

    _setLights();

    _drawGroundPlane();



    SceneNode* professorRoot = mSceneMgr->getRootSceneNode()->createChildSceneNode("ProfessorRoot");
    SceneNode* professorYaw = professorRoot->createChildSceneNode("ProfessorYaw");


    SceneNode* cameraYaw = professorRoot->createChildSceneNode("CameraYaw", Vector3(0.0f, 120.0f, 0.0f));
    SceneNode* cameraPitch = cameraYaw->createChildSceneNode("CameraPitch");
    SceneNode* cameraHolder = cameraPitch->createChildSceneNode("CameraHolder", Vector3(0.0f, 80.0f, 500.0f));
    
    MeshPtr pMesh = MeshManager::getSingleton().load("DustinBody.mesh",
        ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,    
        HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
        HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
        true, true); //so we can still read it
    unsigned short src, dest;
    if (!pMesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
    {
        pMesh->buildTangentVectors(VES_TANGENT, src, dest);
    }

    Entity* entity = mSceneMgr->createEntity("Professor", "DustinBody.mesh");
	professorYaw->attachObject(entity);
	entity->setCastShadows(true);

	entity = mSceneMgr->createEntity("Professor1", "DustinBody.mesh");
	entity->setCastShadows(true);
	SceneNode* professorCopyNode = professorRoot->createChildSceneNode("Professor1");
	professorCopyNode->attachObject(entity);
	professorCopyNode->setPosition(Vector3(100, 0, -100));

	entity = mSceneMgr->createEntity("Professor2", "DustinBody.mesh");
	entity->setCastShadows(true);
	professorCopyNode = professorRoot->createChildSceneNode("Professor2");
	professorCopyNode->attachObject(entity);
	professorCopyNode->setPosition(Vector3(100, 0, 100));

	entity = mSceneMgr->createEntity("Professor3", "DustinBody.mesh");
	entity->setCastShadows(true);
	professorCopyNode = professorRoot->createChildSceneNode("Professor3");
	professorCopyNode->attachObject(entity);
	professorCopyNode->setPosition(Vector3(-100, 0, 100));

	entity = mSceneMgr->createEntity("Professor4", "DustinBody.mesh");
	entity->setCastShadows(true);
	professorCopyNode = professorRoot->createChildSceneNode("Professor4");
	professorCopyNode->attachObject(entity);
	professorCopyNode->setPosition(Vector3(-100, 0, -100));    // Fill Here ----------------------------------

    // --------------------------------------------

    cameraHolder->attachObject(mCamera);
    mCamera->lookAt(cameraYaw->getPosition());

    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
    OIS::ParamList pl;
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
    mInputManager = OIS::InputManager::createInputSystem(pl);

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mMouse = static_cast<OIS::Mouse*>( mInputManager->createInputObject(OIS::OISMouse, true));

    InputController* inputController = new InputController(mRoot, mKeyboard, mMouse);
    mRoot->addFrameListener(inputController);

    mRoot->startRendering();

    mInputManager->destroyInputObject(mKeyboard);
    mInputManager->destroyInputObject(mMouse);
    OIS::InputManager::destroyInputSystem(mInputManager);

    delete inputController;

    delete mRoot;
  }

private:

  void _setLights(void)
  {
    // Fill Here ----------------------------------------------------------------------------------------------
	  mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));
	  mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);

	  mLightD = mSceneMgr->createLight("LightD");
	  mLightD->setType(Light::LT_DIRECTIONAL);
	  mLightD->setDirection(Vector3(1, -2.f, -1));
	  mLightD->setVisible(false);

	  mLightP = mSceneMgr->createLight("LightP");
	  mLightP->setType(Light::LT_POINT);
	  mLightP->setPosition(Vector3(-250, 50, 250));
	  mLightP->setVisible(false);

	  mLightS = mSceneMgr->createLight("LightS");
	  mLightS->setType(Light::LT_SPOTLIGHT);
	  mLightS->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);
	  mLightS->setPosition(Vector3(250, 900, 250));
	  mLightS->setSpotlightRange(Degree(10), Degree(80));
	  mLightS->setVisible(false);

	  SceneNode * NightNode = 
		  mSceneMgr->getRootSceneNode()->createChildSceneNode("Night", Vector3(0, 50, 0));

	  Light * mLightRed = mSceneMgr->createLight("LightRed");
	  mLightRed->setType(Light::LT_SPOTLIGHT);
	  mLightRed->setDirection((Vector3(-1, 0, -0.5f)).normalisedCopy());
	  mLightRed->setPosition(Vector3(0, 0, 0));
	  mLightRed->setSpotlightRange(Degree(20), Degree(80));
	  mLightRed->setDiffuseColour(1.f, 0.f, 0.f);
	  mLightRed->setVisible(true);
	  NightNode->attachObject(mLightRed);

	  Light * mLightGreen = mSceneMgr->createLight("LightGreen");
	  mLightGreen->setType(Light::LT_SPOTLIGHT);
	  mLightGreen->setDirection((Vector3(1, 0, -0.5)).normalisedCopy());
	  mLightGreen->setPosition(Vector3(0, 0, 0));
	  mLightGreen->setSpotlightRange(Degree(20), Degree(80));
	  mLightGreen->setDiffuseColour(0.f, 1.f, 0.f);
	  mLightGreen->setVisible(true);
	  NightNode->attachObject(mLightGreen);

	  Light * mLightBlue = mSceneMgr->createLight("LightBlue");
	  mLightBlue->setType(Light::LT_SPOTLIGHT);
	  mLightBlue->setDirection((Vector3(0, 0, 0.5)).normalisedCopy());
	  mLightBlue->setPosition(Vector3(0, 0, 0));
	  mLightBlue->setSpotlightRange(Degree(20), Degree(80));
	  mLightBlue->setDiffuseColour(0.f, 0.f, 1.f);
	  mLightBlue->setVisible(true);
	  NightNode->attachObject(mLightBlue);
    // --------------------------------------------------------------------------------------------------------
  }

  void _drawGroundPlane(void)
  {
	  // Fill Here ----------------------------------------------------------------------------------------------

	  Plane plane(Vector3::UNIT_Y, 0);
	  MeshManager::getSingleton().createPlane(
		  "Ground",
		  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		  plane,
		  500, 500,
		  1, 1,
		  true, 1, 10, 10,
		  Vector3::NEGATIVE_UNIT_Z
		  );

	  Entity* groundEntity = mSceneMgr->createEntity("GroundPlane", "Ground");
	  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(groundEntity);
	  groundEntity->setMaterialName("KPU_LOGO");
	  groundEntity->setCastShadows(false);
	  // --------------------------------------------------------------------------------------------------------
  }

  void _drawGridPlane(void)
  {
    // 좌표축 표시
    Ogre::Entity* mAxesEntity = mSceneMgr->createEntity("Axes", "axes.mesh");
    mSceneMgr->getRootSceneNode()->createChildSceneNode("AxesNode",Ogre::Vector3(0,0,0))->attachObject(mAxesEntity);
    mSceneMgr->getSceneNode("AxesNode")->setScale(5, 5, 5);

    Ogre::ManualObject* gridPlane =  mSceneMgr->createManualObject("GridPlane"); 
    Ogre::SceneNode* gridPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GridPlaneNode"); 

    Ogre::MaterialPtr gridPlaneMaterial = Ogre::MaterialManager::getSingleton().create("GridPlanMaterial","General"); 
    gridPlaneMaterial->setReceiveShadows(false); 
    gridPlaneMaterial->getTechnique(0)->setLightingEnabled(true); 
    gridPlaneMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,1,1,0); 
    gridPlaneMaterial->getTechnique(0)->getPass(0)->setAmbient(1,1,1); 
    gridPlaneMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,1,1); 

    gridPlane->begin("GridPlaneMaterial", Ogre::RenderOperation::OT_LINE_LIST); 
    for(int i=0; i<21; i++)
    {
      gridPlane->position(-500.0f, 0.0f, 500.0f-i*50);
      gridPlane->position(500.0f, 0.0f, 500.0f-i*50);

      gridPlane->position(-500.f+i*50, 0.f, 500.0f);
      gridPlane->position(-500.f+i*50, 0.f, -500.f);
    }

    gridPlane->end(); 

    gridPlaneNode->attachObject(gridPlane);
  }


  Root* mRoot;
  RenderWindow* mWindow;
  SceneManager* mSceneMgr;
  Camera* mCamera;
  Viewport* mViewport;

  Ogre::Light *mLightP, *mLightD, *mLightS;

  OIS::Keyboard* mKeyboard;
  OIS::Mouse* mMouse;
  OIS::InputManager *mInputManager;
};


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
  int main(int argc, char *argv[])
#endif
  {
    LectureApp app;

    try {

      app.go();

    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
      std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
  }

#ifdef __cplusplus
}
#endif

