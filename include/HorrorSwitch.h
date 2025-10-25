#pragma once

#include "syati.h"

class HorrorSwitch : public LiveActor {
public:
	HorrorSwitch(const char*);

	virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();

    virtual void appear();
    virtual void kill();

    virtual void control();
    virtual void calcAndSetBaseMtx();

    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    

    void onSwitchA();
    void offSwitchA();
    void autoOnSwitchA();
    void autoOffSwitchA();
    void onSwitchB();
    void offSwitchB();


    TQuat4f mRotateQuat;
    TVec3f mFrontVec;
    TVec3f mUpVec;
    bool mIsRotateFacePlayer;
    bool mIsInvertRotate;
    bool mIsStartActive;
    bool mIsIgnoreGravity;
};