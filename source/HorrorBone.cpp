#include "HorrorBone.h"
#include "Horror.h"

HorrorBone::HorrorBone(const char* pName) : ModelObj(pName, BONE_MODEL_NAME, NULL, MR::CategoryList_Auto, MR::CategoryList_Auto, MR::CategoryList_Auto, false) {
	mRotateQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
	mUpQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
	mFrontVec.set(0.0f, 0.0f, 1.0f);
	mSpeed = 0.0f;
	mCalcGravity = true;
	mBounceCount = 0;
}

void HorrorBone::init(const JMapInfoIter& rIter) {
	ModelObj::init(rIter);

	MR::invalidateClipping(this);
	MR::onCalcGravity(this);
	MR::initShadowVolumeSphere(this, 50.0f);

	initNerve(&NrvHorrorBone::NrvTaken::sInstance, 0);
	makeActorDead();

	if (MR::isExistEffect(this, "Fire"))
		MR::setEffectColor(this, "Fire", 170, 0, 200, 0, 150, 230);
	if (MR::isExistEffect(this, "Break"))
		MR::setEffectColor(this, "Break", 170, 0, 200, 0, 150, 230);
}

void HorrorBone::appear() {
	ModelObj::appear();
	MR::startAction(this, "Roll");
	MR::onBind(this);
	setNerve(&NrvHorrorBone::NrvTaken::sInstance);
}

void HorrorBone::kill() {
	ModelObj::kill();
	MR::tryEmitEffect(this, "Break");
	mTranslation.set(*MR::getPlayerPos());
	MR::offBind(this);
	mBounceCount = 0;
	mVelocity.zero();
}

extern "C"
{
	void __kAutoMap_800145C0(LiveActor*);
}

void HorrorBone::control() {
	__kAutoMap_800145C0(this);
}

void HorrorBone::calcAndSetBaseMtx() {
	TMtx34f mtxTR;
	((TPos3f*)&mtxTR)->setTrans(mTranslation);
	((TRot3f*)&mtxTR)->setQuat(mRotateQuat);
	MR::setBaseTRMtx(this, *(TPos3f*)&mtxTR);
}

void HorrorBone::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
	if (MR::isSensorPlayer(pReceiver) || isNerve(&NrvHorrorBone::NrvTaken::sInstance))
		return;

	HitSensor* pBody = getSensor("Body");
	bool IsForceString = MR::isEqualString(pReceiver->mActor->mName, BONE_FORCE_ATTACK_STRONG);

	if (IsForceString)
	{
		if (MR::sendMsgEnemyAttackExplosion(pReceiver, pBody))
		{
			MR::tryRumblePadMiddle(this, 0);
			MR::startActionSound(this, "BmIcemerakingStoneBreak", -1, -1, -1);
			kill();
		}
	}
	else if (MR::sendArbitraryMsg(0xB, pReceiver, pBody) || MR::sendMsgEnemyAttackStrong(pReceiver, pBody))
	{
		MR::tryRumblePadMiddle(this, 0);
		MR::startActionSound(this, "BmIcemerakingStoneBreak", -1, -1, -1);
		kill();
	}
}

void HorrorBone::doBehavior() {
	if (MR::isFirstStep(this)) {
		mUpQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
	}

	if (mCalcGravity) {
		MR::vecKillElement(mFrontVec, mGravity, &mFrontVec);
		MR::normalizeOrZero(&mFrontVec);
	}

	mVelocity.scale(mSpeed, mFrontVec);
	MR::makeQuatUpFront(&mRotateQuat, -mGravity, mFrontVec);
}

void HorrorBone::startBreakSound() {
	MR::startActionSound(this, "EmMoguSpanaBreak", -1, -1, -1);
}

void HorrorBone::startThrowLevelSound() {
	MR::startActionSound(this, "EmLvMoguSpanaFly", -1, -1, -1);
}

void HorrorBone::exeThrow() {
	startThrowLevelSound();
	doBehavior();

	PSQUATMultiply(mUpQuat, mRotateQuat, mRotateQuat);
	mRotateQuat.normalize();

	if (tryReflectOnBind())
		return;
	
	if (!tryKillOnBind() && MR::isGreaterStep(this, 100)) {
		setNerve(&NrvHorrorBone::NrvFall::sInstance);
	}
}

void HorrorBone::exeFall() {
	PSQUATMultiply(mUpQuat, mRotateQuat, mRotateQuat);
	mRotateQuat.normalize();

	MR::applyVelocityDampAndGravity(this, 0.2f, 0.8f, 0.98f, 0.98f, 1.0f);

	if (!tryKillOnBind() && MR::isGreaterStep(this, 100)) {
		startBreakSound();
		kill();
	}
}

void HorrorBone::emit(bool calcGravity, const TVec3f& rTranslation, const TVec3f& rFrontVec, f32 speed) {
	mSpeed = speed;
	mTranslation.set(rTranslation);
	TVec3f off = TVec3f(-mGravity);
	off.scale(MarioAccess::getPlayerActor()->mMario->isCurrentFloorSink() ? 50 : 30);
	mTranslation.add(off);
	mVelocity.zero();
	MR::calcGravity(this);

	if (calcGravity) {
		MR::onCalcGravity(this);
		MR::vecKillElement(rFrontVec, mGravity, &mFrontVec);
		mCalcGravity = true;
	}
	else {
		MR::offCalcGravity(this);
		mFrontVec.set(rFrontVec);
		mCalcGravity = false;
	}

	MR::normalizeOrZero(&mFrontVec);

	setNerve(&NrvHorrorBone::NrvThrow::sInstance);
}

bool HorrorBone::isTaken() {
	return isNerve(&NrvHorrorBone::NrvTaken::sInstance);
}

// Returns true if rebound
bool HorrorBone::tryReflectOnBind()
{
	if (mBounceCount > 3)
		return false;

	// should that even be a cast? or are the headers wrong...
	if ((MR::isBindedWall(this) || MR::isBindedRoof(this)) && MR::isWallCodeRebound((const Triangle*)&this->mBinder->_CC.mParentTriangle.mIndex))
	{
		//TVec3f Grav;
		//MR::calcGravityVector(this, &Grav, NULL, 0);

		if (MR::isBindedWall(this))
		{
			TVec3f VecElement;
			f32 VecVal = MR::vecKillElement(mFrontVec, MR::getWallNormal(this), &VecElement);
			if (VecVal < 0)
			{
				TVec3f WallNormal = TVec3f(MR::getWallNormal(this));
				WallNormal.scale(VecVal);
				TVec3f TEMP = TVec3f(WallNormal);
				TEMP.scale(0.9f);
				mFrontVec = VecElement - TEMP;
			}
		}














		this->mSpine->mStep = 1;
		mBounceCount++;
		return true;
	}
	return false;
}

bool HorrorBone::tryKillOnBind()
{
	if (!MR::isBinded(this))
		return false;
	startBreakSound();
	kill();
	return true;
}

namespace NrvHorrorBone {
	void NrvTaken::execute(Spine* pSpine) const {
		//We need to manually update the bone here
		HorrorBone* pActor = (HorrorBone*)pSpine->mExecutor;
		if (MR::isLessStep(pActor, 1)) {
			MarioActor* pMarioActor = MR::getMarioHolder()->getMarioActor();
			TMtx34f mat;
			TVec3f pos;
			pMarioActor->getRealMtx((MtxPtr)&mat, MARIO_JOINT_HANDR);
			MR::extractMtxTrans((MtxPtr)&mat, &pos);
			pActor->mTranslation.set(pos);
		}
		else
		{
			TVec3f dir;
			MR::getPlayerFrontVec(&dir);
			pActor->emit(true, pActor->mTranslation, dir, BONE_SPEED);
		}
		MR::tryEmitEffect(pActor, "Fire");
	}

	void NrvThrow::execute(Spine* pSpine) const {
		HorrorBone* pActor = (HorrorBone*)pSpine->mExecutor;
		pActor->exeThrow();
	}

	void NrvFall::execute(Spine* pSpine) const {
		HorrorBone* pActor = (HorrorBone*)pSpine->mExecutor;
		pActor->exeFall();
	}

	NrvTaken(NrvTaken::sInstance);
	NrvThrow(NrvThrow::sInstance);
	NrvFall(NrvFall::sInstance);
}







HorrorBoneGroup::HorrorBoneGroup() : LiveActorGroup("HorrorBoneGroup", BONE_MAX) {
	MR::connectToSceneMapObjMovement(this);

	for (s32 i = 0; i < BONE_MAX; i++)
	{
		HorrorBone* pBone = new HorrorBone("MarioHorrorBoneInstance");
		pBone->initWithoutIter();
		registerActor(pBone);
	}
}

void HorrorBoneGroup::movement()
{
	for (s32 i = 0; i < BONE_MAX; i++)
	{
		HorrorBone* pBone = (HorrorBone*)mObjs[i];
		if (MR::isDead(pBone))
			pBone->mTranslation.set(*MR::getPlayerPos());
	}
}

void HorrorBoneGroup::tryAppearBone()
{
	HorrorBone* pBone = (HorrorBone*)getDeadActor();
	if (pBone == NULL)
		return; //No bones atm
	pBone->appear();
}


namespace MR {
	HorrorBoneGroup* createHorrorBoneGroup() {
		return (HorrorBoneGroup*)MR::createSceneObj(SCENE_OBJ_HORROR_BONE_HOLDER);
	}
	HorrorBoneGroup* getHorrorBoneGroup() {
		return (HorrorBoneGroup*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_HORROR_BONE_HOLDER);
	}

	void killAllHorrorBone()
	{
		if (HorrorBoneGroup* pGroup = getHorrorBoneGroup())
			for (s32 i = 0; i < pGroup->mMaxObjs; i++)
				if (LiveActor* pActor = pGroup->getActor(i))
					if (!MR::isDead(pActor))
						pActor->kill();
	}
}

NameObj* SceneObjHolder_createHorrorBoneGroup() {
	return new HorrorBoneGroup();
}

namespace {
	void MarioActor_shootFireBallOrBone(MarioActor* pMarioActor) {
		if (!MR::isGalaxyHorrorCometAppearInCurrentStage() && !MR::isHorrorInLightAny())
		{
			MR::setEffectColor(pMarioActor, MARIO_ANIMNAME_FIRE_SPIN_JP, 255, 255, 255, 255, 255, 255);
			MR::setEffectColor(pMarioActor, MARIO_ANIMNAME_FIRE_THROW_JP, 255, 255, 255, 255, 255, 255);
			if (MR::isCreatePlayerResourceFire())
				pMarioActor->shootFireBall();
			return;
		}

		HorrorBoneGroup* boneGroup = MR::getHorrorBoneGroup();
		if (boneGroup == NULL)
			return;

		MR::setEffectColor(pMarioActor, MARIO_ANIMNAME_FIRE_SPIN_JP, 170, 0, 200, 0, 150, 230);
		MR::setEffectColor(pMarioActor, MARIO_ANIMNAME_FIRE_THROW_JP, 170, 0, 200, 0, 150, 230);


		if (pMarioActor->_F94 != 0 && (pMarioActor->_F94 -= 1) <= 0)
			boneGroup->tryAppearBone();

	}

	kmCall(0x803C2D30, MarioActor_shootFireBallOrBone);


	s32 MarioActor_selectActionSpinHorror(MarioActor* pMarioActor, const char* pName)
	{
		if (MR::isGalaxyHorrorCometAppearInCurrentStage() && !MR::isHorrorInLightAny() && MR::isSubPadSwing(0) && !MR::isPlayerInAreaObj("HorrorForbidThrow"))
			return 2; //Force Fire spinning
		return pMarioActor->selectAction(pName);
	}
	kmCall(0x803C2EF0, MarioActor_selectActionSpinHorror);



}