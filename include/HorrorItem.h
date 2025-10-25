#pragma once

#include "syati.h"

class HorrorItem : public LiveActor {
public:
	HorrorItem(const char* pName);

	virtual void init(const JMapInfoIter& rIter);
	virtual void control();
	virtual void calcAndSetBaseMtx();
	virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

	bool updatePlayerPowerUp();

	TQuat4f mRotQuat;
	TVec3f mRotVec;
};

namespace NrvHorrorItem {
	NERVE(NrvAppear);
	NERVE(NrvWait);
	NERVE(NrvHide);
	NERVE(NrvCollect);
}