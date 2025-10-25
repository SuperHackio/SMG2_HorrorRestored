#pragma once

#include "syati.h"

#define SCENE_OBJ_HORROR_BONE_HOLDER 123
#define BONE_MODEL_NAME "MarioHorrorBone"
#define BONE_MAX 15
#define BONE_SPEED 30.f
#define MARIO_JOINT_HANDR "HandR"
#define MARIO_ANIMNAME_FIRE_SPIN_JP "ファイアスピン"
#define MARIO_ANIMNAME_FIRE_THROW_JP "ファイアボール投げ"
// This is used to break enemies immune to fire that accept a fire hitsensor
#define BONE_FORCE_ATTACK_STRONG "メラメラ"

class HorrorBone : public ModelObj {
public:
	HorrorBone(const char*);

	virtual void init(const JMapInfoIter& rIter);
	virtual void appear();
	virtual void kill();
	virtual void control();
	virtual void calcAndSetBaseMtx();
	virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
	virtual void doBehavior();
	virtual void startBreakSound();
	virtual void startThrowLevelSound();

	void exeThrow();
	void exeFall();
	void emit(bool calcGravity, const TVec3f& rTranslation, const TVec3f& rFrontVec, f32 speed);
	bool isTaken();
	bool tryReflectOnBind();
	bool tryKillOnBind();

	TQuat4f mRotateQuat;
	TQuat4f mUpQuat;
	TVec3f mFrontVec;
	f32 mSpeed;
	bool mCalcGravity;
	u8 mBounceCount;
};

class HorrorBoneGroup : public LiveActorGroup {
public:
	HorrorBoneGroup();

	virtual void movement();

	void tryAppearBone();
};

namespace NrvHorrorBone {
	NERVE(NrvTaken);
	NERVE(NrvThrow);
	NERVE(NrvFall);
};

namespace MR {
	HorrorBoneGroup* createHorrorBoneGroup();
	HorrorBoneGroup* getHorrorBoneGroup();

	void killAllHorrorBone();
}

NameObj* SceneObjHolder_createHorrorBoneGroup();