#include "HorrorItem.h"
#include "HorrorBone.h"
#include "Horror.h"

HorrorItem::HorrorItem(const char* pName) : LiveActor(pName) {
	mRotQuat.set(0.f, 0.f, 0.f, 1.f);
	mRotVec.set(0.f, 0.f, 1.f);
}

void HorrorItem::init(const JMapInfoIter& rIter) {
	MR::processInitFunction(this, rIter, "PowerUpHorror", false);

	TVec3f UpVec;
	MR::calcUpVec(&UpVec, this);
	JGeometry::negateInternal(&UpVec.x, &mGravity.x);

	initNerve(&NrvHorrorItem::NrvWait::sInstance, 0);

	if (MR::useStageSwitchSyncAppear(this, rIter)) {
		MR::invalidateClipping(this);
		setNerve(&NrvHorrorItem::NrvAppear::sInstance);
	}
	else
		makeActorAppeared();

	MR::createHorrorBoneGroup();
}

void HorrorItem::control() {
	if (MR::isGalaxyHorrorCometAppearInCurrentStage() && isNerve(&NrvHorrorItem::NrvWait::sInstance))
	{
		setNerve(&NrvHorrorItem::NrvHide::sInstance);
	}
	else if (!MR::isGalaxyHorrorCometAppearInCurrentStage() && isNerve(&NrvHorrorItem::NrvHide::sInstance))
		setNerve(&NrvHorrorItem::NrvAppear::sInstance);

	if (!isNerve(&NrvHorrorItem::NrvWait::sInstance))
		return; //No rotating unless we're waiting

	MR::turnDirectionToPlayerDegreeHorizon(this, &mRotVec, 3.0f);
	TVec3f tmp;
	JGeometry::negateInternal(&mGravity.x, &tmp.x);
	MR::blendQuatUpFront(&mRotQuat, tmp, mRotVec, 0.2f, 0.2f);
}

void HorrorItem::calcAndSetBaseMtx()
{
	LiveActor::calcAndSetBaseMtx();
	MR::setBaseTRMtx(this, mRotQuat);
}

bool HorrorItem::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
	if (MR::isMsgItemGet(msg))
		return updatePlayerPowerUp();
	return false;
}

bool HorrorItem::updatePlayerPowerUp() {
	if (MR::isHiddenModel(this))
		return false;

	MR::hideModelAndOnCalcAnim(this);
	MR::invalidateClipping(this);
	MR::MarioAccess_changePlayerModeHorror();
	MR::startActionSound(this, "Morph", -1, -1, -1);
	setNerve(&NrvHorrorItem::NrvCollect::sInstance);
	return true;
}

namespace NrvHorrorItem {
	void NrvAppear::execute(Spine* pSpine) const {
		HorrorItem* pHorror = (HorrorItem*)pSpine->mExecutor;
		if (MR::isFirstStep(pHorror))
		{
			MR::startAction(pHorror, "Appear");
			MR::showModel(pHorror);
		}
		if (MR::isActionEnd(pHorror))
			pHorror->setNerve(&NrvHorrorItem::NrvWait::sInstance);
	}
	NrvAppear(NrvAppear::sInstance);

	void NrvWait::execute(Spine* pSpine) const {
		HorrorItem* pHorror = (HorrorItem*)pSpine->mExecutor;
		if (MR::isFirstStep(pHorror))
		{
			MR::showModel(pHorror);
			MR::validateClipping(pHorror);

			MR::startAction(pHorror, "Wait");
		}
	}
	NrvWait(NrvWait::sInstance);

	void NrvHide::execute(Spine* pSpine) const {
		HorrorItem* pHorror = (HorrorItem*)pSpine->mExecutor;
		if (MR::isFirstStep(pHorror))
		{
			MR::startAction(pHorror, "Hide");
			MR::hideModel(pHorror);
		}
	}
	NrvHide(NrvHide::sInstance);

	void NrvCollect::execute(Spine* pSpine) const {
		HorrorItem* pHorror = (HorrorItem*)pSpine->mExecutor;
		if (MR::isFirstStep(pHorror))
		{
			MR::startAction(pHorror, "Hide");
		}

		if (MR::isGreaterStep(pHorror, 0xB4))
			pHorror->setNerve(&NrvHorrorItem::NrvHide::sInstance);
	}
	NrvCollect(NrvCollect::sInstance);
}