#include "HorrorSwitch.h"

HorrorSwitch::HorrorSwitch(const char* pName) : LiveActor(pName)
{
	mRotateQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
	mFrontVec.set(0.0f, 0.0f, 1.0f);
	mIsRotateFacePlayer = mIsInvertRotate = mIsStartActive = mIsIgnoreGravity = false;
}

void HorrorSwitch::init(const JMapInfoIter& rIter)
{
	MR::processInitFunction(this, rIter, false);

	if (!MR::isExistEffectKeeper(this))
		initEffectKeeper(4, NULL, false);
	MR::addEffectHitNormal(this, NULL);

	if (MR::isValidSwitchAppear(this))
		MR::syncStageSwitchAppear(this);
	else
		makeActorAppeared();

	MR::getJMapInfoArg0NoInit(rIter, &mIsStartActive);
	MR::getJMapInfoArg1NoInit(rIter, &mIsRotateFacePlayer);
	MR::getJMapInfoArg2NoInit(rIter, &mIsInvertRotate);
	MR::getJMapInfoArg3NoInit(rIter, &mIsIgnoreGravity);

	if (MR::isValidSwitchA(this))
		MR::listenStageSwitchOnOffA(this, MR::Functor(this, &autoOnSwitchA), MR::Functor(this, &autoOffSwitchA));

	if (MR::isValidSwitchB(this))
	{
		MR::listenStageSwitchOnOffB(this, MR::Functor(this, &onSwitchB), MR::Functor(this, &offSwitchB));
		MR::startAction(this, "Inactive");
	}
	else
	{
		MR::startAction(this, mIsStartActive ? "InitBlue":"InitRed");
	}
}
void HorrorSwitch::initAfterPlacement()
{
	if (mIsStartActive && MR::isValidSwitchA(this))
		MR::onSwitchA(this);


	if (MR::isZeroGravity(this) || mIsIgnoreGravity)
	{
		MR::calcUpVec(&mUpVec, this);
		mGravity.set(-mUpVec);
	}
	else
	{
		MR::calcGravity(this);
		MR::calcGravityVector(this, &mUpVec, NULL, 0);
	}

	mUpVec.negate();
	MR::makeQuatAndFrontFromRotate(&mRotateQuat, &mFrontVec, this);
}

void HorrorSwitch::appear()
{
	if (MR::isExistEffect(this, "Appear"))
		MR::emitEffect(this, "Appear");

	LiveActor::appear();
}

void HorrorSwitch::kill()
{
	if (MR::isExistEffect(this, "Kill"))
		MR::emitEffect(this, "Kill");

	LiveActor::kill();
}

void HorrorSwitch::control()
{
	if (mIsRotateFacePlayer)
	{
		MR::turnDirectionToPlayerDegreeHorizon(this, &mFrontVec, 3.0f);
		TVec3f temp = TVec3f(mFrontVec);
		if (mIsInvertRotate)
			temp.negate();
		MR::blendQuatUpFront(&mRotateQuat, mUpVec, temp, 0.2f, 0.3f);
	}

	if (MR::isBckOneTimeAndStopped(this))
	{
		if (MR::isValidSwitchB(this) && !MR::isOnSwitchB(this))
			MR::startAction(this, "Inactive");
		else
			MR::startAction(this, "Active");
		MR::validateClipping(this);
	}
}

void HorrorSwitch::calcAndSetBaseMtx()
{
	LiveActor::calcAndSetBaseMtx();
	if (mIsRotateFacePlayer)
		MR::setBaseTRMtx(this, mRotateQuat);
}

void HorrorSwitch::attackSensor(HitSensor* pSender, HitSensor* pReceiver)
{
	if (MR::isEqualString(pReceiver->mActor->mName, "MarioHorrorBoneInstance"))
		return;
	MR::sendMsgPush(pReceiver, pSender);
}

bool HorrorSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (msg != 0x0B || !MR::isEqualString(pSender->mActor->mName, "MarioHorrorBoneInstance") || pReceiver != getSensor("Gem"))
		return false;

	// Switch Logic
	if (MR::isValidSwitchB(this) ^ !MR::isOnSwitchB(this))
	{
		MR::emitEffectHit(this, pSender->mPosition, NULL);
		MR::startActionSound(this, "Hit", -1, -1, -1);
		if (MR::isOnSwitchA(this))
			offSwitchA();
		else
			onSwitchA();
	}

	return true;
}
bool HorrorSwitch::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (MR::isMsgExplosionAttack(msg))
	{
		MR::startBck(this, "Hit", NULL);
		return true;
	}
	return false;
}

void HorrorSwitch::onSwitchA()
{
	if (MR::isValidSwitchA(this))
		MR::onSwitchA(this);
}
void HorrorSwitch::offSwitchA()
{
	if (MR::isValidSwitchA(this))
		MR::offSwitchA(this);
}

void HorrorSwitch::autoOnSwitchA()
{
	MR::startAction(this, "HitToBlue");
	MR::invalidateClipping(this);
}
void HorrorSwitch::autoOffSwitchA()
{
	MR::startAction(this, "HitToRed");
	MR::invalidateClipping(this);
}

void HorrorSwitch::onSwitchB()
{
	MR::startAction(this, MR::isOnSwitchA(this) ? "InactiveToBlue" : "InactiveToRed");
	MR::invalidateClipping(this);
}
void HorrorSwitch::offSwitchB()
{
	MR::startAction(this, MR::isOnSwitchA(this) ? "BlueToInactive" : "RedToInactive");
	MR::invalidateClipping(this);
}