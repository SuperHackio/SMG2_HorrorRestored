#pragma once

#include "syati.h"

#define SUBMETER_HORROR "SubMeterHorror"
#define SUBMETER__PANENAME_RING "Ring"
#define SUBMETER_ANIMNAME_DRAIN "Drain"
#define SUBMETER_ANIMNAME_CHUNK "Chunk"
#define SUBMETER_ANIMNAME_REFILL "Refill"
#define SUBMETER_AIR "êÖíÜÉÅÅ[É^Å["

class SubMeterLayout;

namespace MR {
	bool isHorrorModeActive();
	s32 getHorrorFlagData();
	bool isHorrorDamageWait();
	bool isHorrorInLightAny();
	void addHorror(int);

	void MarioAccess_changePlayerModeHorror();

	void MarioActor_initHorrorMario(MarioActor* pMarioActor);

	void initHorrorMeter(SubMeterLayout*);
	void startHorrorRingAnim(const char* pAnimName);
}