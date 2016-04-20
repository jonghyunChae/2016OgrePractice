#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>

using namespace Ogre;

class ESCListener : public FrameListener {
	OIS::Keyboard *mKeyboard;
  
public:
	ESCListener(OIS::Keyboard *keyboard) : mKeyboard(keyboard) {}
  bool frameStarted(const FrameEvent &evt)
  {
    mKeyboard->capture();
	return !mKeyboard->isKeyDown(OIS::KC_ESCAPE);
  }
};  

class LectureApp {

  Root* mRoot;
  RenderWindow* mWindow;
  SceneManager* mSceneMgr;
  Camera* mCamera;
  Viewport* mViewport;
  OIS::Keyboard* mKeyboard;
  OIS::InputManager *mInputManager;
  ESCListener* mESCListener;

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
		  if (!mRoot->showConfigDialog())
			  return;
	  }

	  // 렌더 윈도의 생성
	  mWindow = mRoot->initialise(true, "Make Render Window");

	  // ESC key를 눌렀을 경우, 오우거 메인 렌더링 루프의 탈출을 처리
	  size_t windowHnd = 0;
	  std::ostringstream windowHndStr;
	  OIS::ParamList pl;

	  mWindow->getCustomAttribute("WINDOW", &windowHnd);
	  windowHndStr << windowHnd;

	  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	  mInputManager = OIS::InputManager::createInputSystem(pl);
	  mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));

	  mESCListener = new ESCListener(mKeyboard);
	  mRoot->addFrameListener(mESCListener);

	  // 기본 씬 매니저의 생성
	  mSceneMgr = mRoot->createSceneManager(ST_GENERIC);

	  // 카메라의 생성 및 설정
	  mCamera = mSceneMgr->createCamera("camera");
	  mCamera->setPosition(500.0f, 100.0f, 0.0f);
	  mCamera->lookAt(0.0f, 100.0f, 0.0f);
	  mCamera->setNearClipDistance(5.0f);

	  // 뷰포트의 생성 및 설정
	  mViewport = mWindow->addViewport(mCamera);
	  mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.0f));
	  // 종횡비
	  mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));

	  // 렌더링 루프의 실행
	  mRoot->startRendering();

	  delete mRoot;
  }
};


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
    std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
  }

  return 0;
}
