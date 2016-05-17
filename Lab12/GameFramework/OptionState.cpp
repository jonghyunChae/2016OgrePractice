#include "OptionState.h"
#include "TitleState.h"
#include "PlayState.h"

using namespace Ogre;

OptionState OptionState::mOptionState;

void OptionState::enter(void)
{
	mRoot = Root::getSingletonPtr();
	//mRoot->getAutoCreatedWindow()->resetStatistics();

	mSceneMgr = mRoot->getSceneManager("main");
	mCamera = mSceneMgr->getCamera("main");
	mCamera->setPosition(Ogre::Vector3::ZERO);

	mCharacterEntity = mSceneMgr->getEntity("Professor");

	mAnimationState = mCharacterEntity->getAnimationState("Run");
	mAnimationState->setLoop(true);
	mAnimationState->setEnabled(true);
}

void OptionState::exit(void)
{
	PlayState::getInstance()->setAnimState(mAnimationState);
}

void OptionState::pause(void)
{
}

void OptionState::resume(void)
{
}

bool OptionState::frameStarted(GameManager* game, const FrameEvent& evt)
{
	mAnimationState->addTime(evt.timeSinceLastFrame);

	return true;
}

bool OptionState::frameEnded(GameManager* game, const FrameEvent& evt)
{
	return true;
}


bool OptionState::keyReleased(GameManager* game, const OIS::KeyEvent &e)
{
	return true;
}

bool OptionState::keyPressed(GameManager* game, const OIS::KeyEvent &e)
{
	// Fill Here -------------------------------------------
	switch (e.key)
	{
	case OIS::KC_W:
		mAnimationState->setEnabled(false);
		mAnimationState = mCharacterEntity->getAnimationState("Walk");
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
		break;

	case OIS::KC_R:
		mAnimationState->setEnabled(false);
		mAnimationState = mCharacterEntity->getAnimationState("Run");
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
		break;

	case OIS::KC_ESCAPE:
		game->popState();
		break;
	}
	// -----------------------------------------------------
	return true;
}

bool OptionState::mousePressed(GameManager* game, const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
	return true;
}

bool OptionState::mouseReleased(GameManager* game, const OIS::MouseEvent &e, OIS::MouseButtonID id)
{
	return true;
}


bool OptionState::mouseMoved(GameManager* game, const OIS::MouseEvent &e)
{
	return true;
}



