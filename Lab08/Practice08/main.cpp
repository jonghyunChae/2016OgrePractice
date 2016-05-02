#define CLIENT_DESCRIPTION "Walking Around Professor Smooth Rotation"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#include <Ogre.h>
#include <OIS/OIS.h>
#include <iostream>
#include <map>

using namespace std;
using namespace Ogre;

class AnimationObject
{
public:
	AnimationObject()
	{
		mNode           = nullptr;
		mEntity         = nullptr;
		mAnimationState = nullptr;
		mState          = eNONE;
		mRotatingTime   = 0.f;

		mDirVector = mVelocity = Vector3::ZERO;
		mTargetPos = Vector3::ZERO;
		mSpeed          = 0.f;
		mTargetDistance = 0.f;

		mSrcQuat        = Quaternion::ZERO;
		mDestQuat       = Quaternion::ZERO;

		mBasicLookVector = Vector3::UNIT_Z;
	}

	~AnimationObject()
	{
	}

	Vector3 getPosition() { return mNode->getPosition(); }

	void basicRotate(Vector3 & toLook)
	{
		Quaternion rot = Vector3::UNIT_Z.getRotationTo(toLook);
		mNode->rotate(rot);
		mBasicLookVector = rot.zAxis();
	}

	bool isMovingToPoint() { return mTargetDistance > 0.f; }
	void setIdleAnim(const char * name) { mAnimList[eIDLE] = string(name); }
	void setWalkAnim(const char * name) { mAnimList[eWALKING] = string(name); }
	void setSpeed(float speed) { mSpeed = speed; }
	void setData(Root * root, const char * objName, const char * initAnimState, char * initWalkState)
	{
		mNode   = root->getSceneManager("main")->getSceneNode(objName);
		mEntity = root->getSceneManager("main")->getEntity(objName);
		setIdleAnim(initAnimState);
		setWalkAnim(initWalkState);

		mAnimationState = mEntity->getAnimationState(initAnimState);
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
	}

	void setAnimation(string name)
	{
		mAnimationState->setEnabled(false);
		mAnimationState = mEntity->getAnimationState(name);//("Walk");
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
	}

	void move(const Vector3 & addVelocity)
	{
		Vector3 before = mVelocity;
		mVelocity += addVelocity;
		
		Vector3 after = mVelocity;
		after.normalise();
		mDirVector = after;
		changeState(before, mDirVector);

		mTargetDistance = 0.f;
	}

	void moveToPoint(const Vector3 & pos)
	{
		mTargetPos = pos;
		Vector3 mypos = mNode->getPosition();
		Vector3 Direction = mTargetPos - mypos;
		mTargetDistance = Direction.normalise();

		changeState(mDirVector, Direction);
		mDirVector = Direction;
		mVelocity = Vector3::ZERO;
	}

	void update(float frameTime)
	{
		mAnimationState->addTime(frameTime);

		if (mState == eROTATING)
		{
			static const float ROTATION_TIME = 0.3f;
			mRotatingTime = (mRotatingTime > ROTATION_TIME) ? ROTATION_TIME : mRotatingTime;
			mRotatingTime += frameTime;
			const Quaternion delta = Quaternion::Slerp(mRotatingTime / ROTATION_TIME, mSrcQuat, mDestQuat, true);

			mNode->setOrientation(delta);
			if (mRotatingTime >= ROTATION_TIME)
			{
				mRotatingTime = 0.f;
				mState = eWALKING;
				mNode->setOrientation(mDestQuat);
			}
		}
		else if (mState == eWALKING)
		{
			if (isMovingToPoint())
			{
				mTargetDistance -= mSpeed * frameTime;
				if (mTargetDistance < 0.1f)
				{
					mNode->setPosition(mTargetPos);
					mTargetDistance = 0.f;
					return;
				}
			}
			mNode->translate(mDirVector * mSpeed * frameTime);

			Quaternion rot = mBasicLookVector.getRotationTo(mDirVector);
			mNode->setOrientation(rot);
		}
	}

	bool changeState(Vector3 & before, Vector3 & afterVelocity)
	{
		if (afterVelocity == Vector3::ZERO)
		{
			mState = eIDLE;
			setAnimation(mAnimList[eIDLE]);
			return false;
		}
		else
		{
			mState = eWALKING;
			setAnimation(mAnimList[eWALKING]);
		}

		Vector3 MoveDir = afterVelocity;
		MoveDir.normalise();

		Vector3 Before = before;
		Before.normalise();

		if (Before == MoveDir) 
			return false;

		mSrcQuat = mNode->getOrientation();
		mDestQuat = mBasicLookVector.getRotationTo(MoveDir);

		mState = eROTATING;
		mRotatingTime = 0.f;
		return true;
	}

private:
	SceneNode * mNode;
	Entity * mEntity;
	AnimationState* mAnimationState;

	enum OBJ_STATE{ eNONE = -1, eIDLE, eWALKING, eROTATING};
	OBJ_STATE mState;
	float mRotatingTime;
	
	Vector3 mVelocity, mDirVector;
	float mSpeed;
	Quaternion mSrcQuat, mDestQuat;

	float mTargetDistance;
	Vector3 mTargetPos;

	Vector3 mBasicLookVector;

	std::map<OBJ_STATE, string> mAnimList;
};


class InputController : public FrameListener,
	public OIS::KeyListener,
	public OIS::MouseListener
{

public:
	InputController(Root* root, OIS::Keyboard *keyboard, OIS::Mouse *mouse) : mRoot(root), mKeyboard(keyboard), mMouse(mouse)
	{
		mProfessor = new AnimationObject();
		mProfessor->setData(root, "Professor", "Idle", "Walk");
		mProfessor->setSpeed(100.f);

		mCameraMoveVector = Vector3::ZERO;

		mCamera = mRoot->getSceneManager("main")->getCamera("main");

		keyboard->setEventCallback(this);
		mouse->setEventCallback(this);
	}
	~InputController()
	{
		if (mProfessor) delete mProfessor;
	}

	bool frameStarted(const FrameEvent &evt)
	{
		mKeyboard->capture();
		mMouse->capture();

		mCamera->moveRelative(mCameraMoveVector);

		mProfessor->update(evt.timeSinceLastFrame);

		return mContinue;
	}

	// Key Linstener Interface Implementation

	bool keyPressed(const OIS::KeyEvent &evt)
	{
		switch (evt.key)
		{
		case OIS::KC_W: mCameraMoveVector.y += 1; break;
		case OIS::KC_S: mCameraMoveVector.y -= 1; break;
		case OIS::KC_A: mCameraMoveVector.x -= 1; break;
		case OIS::KC_D: mCameraMoveVector.x += 1; break;
		
		case OIS::KC_LEFT:  mProfessor->move(-Vector3::UNIT_X); break;
		case OIS::KC_RIGHT: mProfessor->move(Vector3::UNIT_X);  break;
		case OIS::KC_UP:    mProfessor->move(-Vector3::UNIT_Z); break;
		case OIS::KC_DOWN:  mProfessor->move(Vector3::UNIT_Z);  break;
		
		case OIS::KC_ESCAPE: mContinue = false; break;
		}

		return true;
	}

	bool keyReleased(const OIS::KeyEvent &evt)
	{
		switch (evt.key)
		{
		case OIS::KC_W: mCameraMoveVector.y -= 1; break;
		case OIS::KC_S: mCameraMoveVector.y += 1; break;
		case OIS::KC_A: mCameraMoveVector.x += 1; break;
		case OIS::KC_D: mCameraMoveVector.x -= 1; break;

		case OIS::KC_LEFT:  mProfessor->move(Vector3::UNIT_X);   break;
		case OIS::KC_RIGHT: mProfessor->move(-Vector3::UNIT_X);  break;
		case OIS::KC_UP:    mProfessor->move(Vector3::UNIT_Z);   break;
		case OIS::KC_DOWN:  mProfessor->move(-Vector3::UNIT_Z);  break;

		case OIS::KC_ESCAPE: mContinue = false; break;
		}

		return true;
	}

	// Mouse Listener Interface Implementation

	bool mouseMoved(const OIS::MouseEvent &evt)
	{
		if (evt.state.buttonDown(OIS::MB_Right)) {
			mCamera->yaw(Degree(-evt.state.X.rel));
			mCamera->pitch(Degree(-evt.state.Y.rel));
		}

		mCamera->moveRelative(Ogre::Vector3(0, 0, -evt.state.Z.rel * 0.1f));

		return true;
	}

	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		return true;
	}

private:
	AnimationObject * mProfessor;

	bool mContinue;
	Ogre::Root* mRoot;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;
	Camera* mCamera;

	Ogre::Vector3 mCameraMoveVector;
};

class NinjaController : public FrameListener
{

public:
	NinjaController(Root* root)
	{
		mProfessorNode = root->getSceneManager("main")->getSceneNode("Professor");

		mNinja = new AnimationObject();
		mNinja->setData(root, "Ninja", "Walk", "Walk");
		mNinja->basicRotate(-Vector3::UNIT_Z);
		mNinja->setSpeed(80.f);

		mWalkList.push_back(randomVector());

		nextLocation();
	}

	bool frameStarted(const FrameEvent &evt)
	{
		mNinja->update(evt.timeSinceLastFrame);
		Vector3 professorPos = mProfessorNode->getPosition();
		if (mNinja->getPosition().distance(professorPos) < 100.f)
		{
			mNinja->moveToPoint(professorPos);
		}
		else if (false == mNinja->isMovingToPoint())
		{
			nextLocation();
		}
		return true;
	}

	bool nextLocation(void)
	{
		if (mWalkList.empty())  // 더 이상 목표 지점이 없으면 false 리턴
			return false;

		Vector3 mDest = mWalkList.front();
		mWalkList.pop_front();
		mNinja->moveToPoint(mDest);
		mWalkList.push_back(randomVector());
		return true;
	}

	Vector3 randomVector()
	{
		return Vector3(rand() % 500 - 250, 0.f, rand() % 500 - 250);
	}

private:
	std::deque<Vector3> mWalkList;
	AnimationObject * mNinja;
	SceneNode * mProfessorNode;
};



class LectureApp {

	Root* mRoot;
	RenderWindow* mWindow;
	SceneManager* mSceneMgr;
	Camera* mCamera;
	Viewport* mViewport;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;

	OIS::InputManager *mInputManager;



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

		mWindow = mRoot->initialise(true, CLIENT_DESCRIPTION " : Copyleft by Dae-Hyun Lee");

		mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "main");
		mCamera = mSceneMgr->createCamera("main");


		mCamera->setPosition(0.0f, 150.0f, 600.0f);
		mCamera->lookAt(0.0f, 100.0f, 0.0f);

		mViewport = mWindow->addViewport(mCamera);
		mViewport->setBackgroundColour(ColourValue(0.0f, 0.0f, 0.5f));
		mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


		ResourceGroupManager::getSingleton().addResourceLocation("resource.zip", "Zip");
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

		// 좌표축 표시
		Ogre::Entity* mAxesEntity = mSceneMgr->createEntity("Axes", "axes.mesh");
		mSceneMgr->getRootSceneNode()->createChildSceneNode("AxesNode", Ogre::Vector3(0, 0, 0))->attachObject(mAxesEntity);
		mSceneMgr->getSceneNode("AxesNode")->setScale(5, 5, 5);

		_drawGridPlane();


		Entity* entity1 = mSceneMgr->createEntity("Professor", "DustinBody.mesh");
		SceneNode* node1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Professor", Vector3(0.0f, 0.0f, 0.0f));
		node1->attachObject(entity1);

		Entity* entity2 = mSceneMgr->createEntity("Ninja", "ninja.mesh");
		SceneNode* node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("Ninja", Vector3(0.0f, 0.0f, 0.0f));
		node2->attachObject(entity2);
		node2->translate(Vector3(300, 0, 0));

		node2->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Y));


		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		OIS::ParamList pl;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		mInputManager = OIS::InputManager::createInputSystem(pl);


		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));

		InputController* inputController = new InputController(mRoot, mKeyboard, mMouse);
		mRoot->addFrameListener(inputController);

		NinjaController* professorController = new NinjaController(mRoot);
		mRoot->addFrameListener(professorController);

		mRoot->startRendering();

		mInputManager->destroyInputObject(mKeyboard);
		mInputManager->destroyInputObject(mMouse);
		OIS::InputManager::destroyInputSystem(mInputManager);

		delete professorController;
		delete inputController;

		delete mRoot;
	}

private:
	void _drawGridPlane(void)
	{
		Ogre::ManualObject* gridPlane = mSceneMgr->createManualObject("GridPlane");
		Ogre::SceneNode* gridPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GridPlaneNode");

		Ogre::MaterialPtr gridPlaneMaterial = Ogre::MaterialManager::getSingleton().create("GridPlanMaterial", "General");
		gridPlaneMaterial->setReceiveShadows(false);
		gridPlaneMaterial->getTechnique(0)->setLightingEnabled(true);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setDiffuse(1, 1, 1, 0);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setAmbient(1, 1, 1);
		gridPlaneMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1, 1, 1);

		gridPlane->begin("GridPlaneMaterial", Ogre::RenderOperation::OT_LINE_LIST);
		for (int i = 0; i < 21; i++)
		{
			gridPlane->position(-500.0f, 0.0f, 500.0f - i * 50);
			gridPlane->position(500.0f, 0.0f, 500.0f - i * 50);

			gridPlane->position(-500.f + i * 50, 0.f, 500.0f);
			gridPlane->position(-500.f + i * 50, 0.f, -500.f);
		}

		gridPlane->end();

		gridPlaneNode->attachObject(gridPlane);
	}
};


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		LectureApp app;

		try {

			app.go();

		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
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

