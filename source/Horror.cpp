#include "Horror.h"
#include "ExtMarioSubMeter.h"

extern u8 sPlayerType__10MarioActor;

namespace MR {
	static s32 gHorrorMax = 3600;
	static s32 gHorrorProgress = 100;

	bool isHorrorModeActive()
	{
		return EventFunction::isStartCometEvent("Horror") || MarioAccess::getPlayerActor()->mPlayerMode == PLAYER_MODE_HORROR;
	}
	kmBranch(0x80024F00, isHorrorModeActive);

	// Returns the flag data that indicates if you are inside a light area... or Boo Mario... for some reason it tracks that too..
	s32 getHorrorFlagData()
	{
		register Mario* pMario = MarioAccess::getPlayerActor()->mMario;
		register s32 HorrorFlagData;
		__asm {
			lwz HorrorFlagData, 0x5F8(pMario)
		}
		return HorrorFlagData;
	}

	// Returns true if the player is in the "broken bones on the floor" state. The state where you can spam A to recover faster.
	bool isHorrorDamageWait()
	{
		register Mario* pMario = MarioAccess::getPlayerActor()->mMario;
		register HorrorDamageExecutor* reg;
		__asm {
			lwz reg, 0x7C4(pMario)
			lwz reg, 0x40(reg)
		}
		return reg->isNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakWait::sInstance);
	}

	bool isMarioInFreeze()
	{
		return MR::getMarioHolder()->mMarioActor->mMario->isStatusActive(MARIO_FREEZE);
	}

	void resetHorrorDamageExecutor()
	{
		register Mario* pMario = MarioAccess::getPlayerActor()->mMario;
		register HorrorDamageExecutor* reg;
		__asm {
			lwz reg, 0x7C4(pMario)
			lwz reg, 0x40(reg)
		}
		reg->setNerve(&NrvHorrorDamageExecutor::HorrorDamageExecutorNrvBoneBreakStart::sInstance);
	}

	bool isHorrorInLightAny()
	{
		s32 horror = getHorrorFlagData();
		return (horror & PAUSE_HORROR_FLAG_GLARING_LIGHT) || (horror & PAUSE_HORROR_FLAG_SUN_LIGHT);
	}

	void addHorror(int v) {
		gHorrorProgress += v;
		startHorrorRingAnim(SUBMETER_ANIMNAME_REFILL);
	}

	void subHorror(int v) {
		gHorrorProgress -= v;
		if (v > 15)
			startHorrorRingAnim(SUBMETER_ANIMNAME_CHUNK);
		else if (v > 1)
			startHorrorRingAnim(SUBMETER_ANIMNAME_DRAIN);
	}

	void MarioAccess_changePlayerModeHorror()
	{
		gHorrorProgress = gHorrorMax;
		MR::getMarioHolder()->getMarioActor()->setPlayerMode(PLAYER_MODE_HORROR, true, true);
	}

	void MarioActor_initHorrorMario(MarioActor* pMarioActor)
	{
		//Currently this doesn't check the MarioHolder flag, so that's just another mario model that's always loaded...

		const char* pModelName = sPlayerType__10MarioActor == 1 ? "HorrorLuigi" : "HorrorMario";
		const char* pArchiveName = sPlayerType__10MarioActor == 1 ? "HorrorLuigi.arc" : "HorrorMario.arc";
		MR::createAndAddResourceHolder(pArchiveName);
		pMarioActor->mHorrorModel = new ModelHolder(pModelName, true);
		pMarioActor->mMarioAllModel->registerModel(MARIO_ALL_MODEL_HORROR, pMarioActor->mHorrorModel, false, false);
		pMarioActor->mHorrorModel->kill();
		MR::hideModel(pMarioActor->mHorrorModel);
		pMarioActor->mModelFlags |= PLAYER_MODEL_FLAG_HORROR;
		gHorrorProgress = gHorrorMax;
	}


	kmWrite32(0x803C0128, 0x7FC3F378); //mr r3, r30
	kmCall(0x803C012C, MarioActor_initHorrorMario);
	kmWrite32(0x803C0130, 0x7FC3F378); //mr r3, r30
	kmWrite32(0x803C0134, 0x60000000); //nop

	bool newJ3DModelExt(const char* pTarget, const char* pRockMarioStr)
	{
		return MR::isEqualString(pTarget, pRockMarioStr) || MR::isEqualString(pTarget, "HorrorMario") || MR::isEqualString(pTarget, "HorrorLuigi");
	}
	kmCall(0x800429E0, newJ3DModelExt);



	void Mario_updateHorrorMode(Mario* pMario)
	{
		pMario->updateCometCode();

		//Update the meter
		SubMeterLayout* pHorrorMeter = getMarioSubMeter(SUBMETER_HORROR);

		if (!MR::isGalaxyHorrorCometAppearInCurrentStage())
		{
			pHorrorMeter->requestDeactivate();
			return;
		}

		if (isMarioInFreeze())
			resetHorrorDamageExecutor();

		s32 HorrorMode = getHorrorFlagData();

		if (HorrorMode == 0) //No special effects. Just drain the meter;
		{
			int v = 1;
			if (isHorrorDamageWait() || isMarioInFreeze())
				v = 5;
			else if (MarioAccess::getPlayerActor()->mMario->isCurrentFloorSink())
				v = 2; //you drain twice as fast in sinking floor
			if (!MR::isOffPlayerControl())
				subHorror(v); // Ensure the meter cannot go down if the player has no control Lol
		}
		else if (HorrorMode & PAUSE_HORROR_FLAG_TERESA_MARIO) {
			//uhhhh what...
		}
		else if (HorrorMode & PAUSE_HORROR_FLAG_SUN_LIGHT) {
			// When inside a Sunlight Area, the meter will begin to refill
			addHorror(10);
		}
		else if (HorrorMode & PAUSE_HORROR_FLAG_GLARING_LIGHT) {
			// When inside a Glaring Light Area, the meter will be paused from draining
		}

		s32 min = 0;
		s32 max = gHorrorMax;

		if (AreaObj* pArea = MR::getAreaObjAtPlayerPos("HorrorLimit"))
		{
			if (pArea->mObjArg0 >= 0)
				min = pArea->mObjArg0;
			if (pArea->mObjArg1 >= 0)
				max = pArea->mObjArg1;
		}

		gHorrorProgress = MR::clamp(gHorrorProgress, min, max);

		if (gHorrorProgress <= 0) //TIME TO DIE
		{
			pHorrorMeter->requestDeactivate();
			MR::forceKillPlayerByWaterOrGroundRace();
			return;
		}
		if (gHorrorProgress <= gHorrorMax)
		{
			if (MR::isHorrorInLightAny() && MR::isPlayerInWaterMode() && !MR::isPlayerOnWaterSurface())
			{
				deactivateSubMeterExt(SUBMETER_HORROR);
				activeSubMeterExt(SUBMETER_AIR); //Yeah, you can use the Module to activate the Water and Fly meters as well lol.
			}
			else
			{
				if (MR::isActiveDefaultGameLayout())
				{
					activeSubMeterExt(SUBMETER_HORROR);
					if (!isActiveSubMeterExt(SUBMETER_HORROR))
						requestFrameInSubMeterExt(SUBMETER_HORROR);
				}
			}

		}


		f32 scale = (f32)(1.f / gHorrorMax);
		f32 ratio = ((gHorrorProgress) * scale);
		pHorrorMeter->setLifeRatio(ratio);
	}

	kmCall(0x803889D0, Mario_updateHorrorMode);


	// This prevents the game from adding time to the comet timer infinitely
	kmWrite32(0x80389020, 0x60000000);


	bool isGalaxyHorrorAndNoLightAreaAny()
	{
		return MR::isGalaxyHorrorCometAppearInCurrentStage() && !MR::isHorrorInLightAny();
	}
	kmCall(0x803B90DC, isGalaxyHorrorAndNoLightAreaAny);
	kmCall(0x803E4B58, isGalaxyHorrorAndNoLightAreaAny);

	void Mario_updateOnSand(Mario* pMario)
	{
		if (MR::isGalaxyHorrorAndNoLightAreaAny())
			return; //Basically don't do any of this if you're Cursed.

		pMario->updateOnSand();
	}

	kmCall(0x803893C4, Mario_updateOnSand);

	void Mario_updateOnPoison(Mario* pMario)
	{
		if (MR::isGalaxyHorrorAndNoLightAreaAny())
			return; //Basically don't do any of this if you're Cursed.

		pMario->updateOnPoison();
	}

	kmCall(0x803893D4, Mario_updateOnPoison);
	

	Mario* MarioModule_decHorrorOnFireImpact(MarioModule* pModule)
	{
		register Mario* pMario = pModule->getPlayer();
		register u16 FloorCode;
		__asm
		{
			lwz FloorCode, 0x964(pMario)
		}
		if (MR::isGalaxyHorrorAndNoLightAreaAny())
		{
			if (pMario->checkCurrentFloorCodeSevere(0x0A) || pMario->checkCurrentFloorCodeSevere(0x81))
				subHorror(60 * 8);
			else if (pMario->checkCurrentFloorCodeSevere(0x18))
				subHorror(60 * 4);

			if (gHorrorProgress <= 0)
				gHorrorProgress = 2; // needed to make the UI update properly
		}
		return pMario;
	}
	kmCall(0x803E5E28, MarioModule_decHorrorOnFireImpact);


	bool addHorrorAndStarPiece()
	{
		register int StarPieceNum;
		__asm
		{
			mr StarPieceNum, r30
		}
		if (MR::isGalaxyHorrorCometAppearInCurrentStage())
			MR::addHorror(30*StarPieceNum);
		return GameSequenceFunction::isPadModeDreamer();
	}
	kmCall(0x800239E4, addHorrorAndStarPiece);



	void MarioActor_playEffectOrHorror(MarioActor* pMarioActor, const char* pEffectName)
	{
		if (MR::isGalaxyHorrorCometAppearInCurrentStage())
			MR::setEffectColor(pMarioActor, pEffectName, 48, 0, 32, 150, 0, 230);
		else
			MR::setEffectColor(pMarioActor, pEffectName, 255, 255, 255, 255, 255, 255);
		pMarioActor->playEffect(pEffectName);
	}
	kmCall(0x803C1E10, MarioActor_playEffectOrHorror);





	void initHorrorMeter(SubMeterLayout* pMeter)
	{
		MR::createAndAddPaneCtrl(pMeter, SUBMETER__PANENAME_RING, 1);
	}

	void startHorrorRingAnim(const char* pAnimName)
	{
		SubMeterLayout* pMeter = getMarioSubMeter(SUBMETER_HORROR);
		if (pMeter == NULL || pMeter != getCurrentMarioSubMeter())
			return;
		if (MR::isPaneAnimStopped(pMeter, SUBMETER__PANENAME_RING, 0))
			MR::startPaneAnim(pMeter, SUBMETER__PANENAME_RING, pAnimName, 0);
	}

	// I have no choice but to do this in ASM
	asm void Mario_getTargetWalkSpeedA()
	{
		bl isGalaxyHorrorAndNoLightAreaAny;
		mr r5, r3; // Extremely Dangerous
		mr r3, r31;
	}
	kmCall(0x803ACB3C, Mario_getTargetWalkSpeedA);

	asm void Mario_getTargetWalkSpeedB()
	{
		cmpwi r5, 0;
		beq Loc_A;
		li r0, 0x12;
		b Loc_B;
		Loc_A:
		lhz r0, 0x964(r31);
	    Loc_B:
	}
	kmCall(0x803ACB8C, Mario_getTargetWalkSpeedB);

	asm void Mario_getTargetWalkSpeedC()
	{
		lwz r0, 0xC(r3);
		extrwi. r0, r0, 1, 10; // Extremely Clever, but Dangerous
	}
	kmCall(0x803ACB40, Mario_getTargetWalkSpeedC);

	asm void Mario_jumpCheckFuncA()
	{
		bl isGalaxyHorrorAndNoLightAreaAny;
		mr r5, r3;
		mr r3, r30;
		lbz r4, 0x72D(r3);
	}
	kmCall(0x80399D50, Mario_jumpCheckFuncA);

	asm void Mario_jumpCheckFuncB()
	{
		cmpwi r5, 0;
		beq Loc_A;
		li r0, 0x12;
		b Loc_B;
		Loc_A:
		lhz r0, 0x964(r3);
		Loc_B:
	}
	kmCall(0x80399D64, Mario_jumpCheckFuncB);

	asm void MarioSwim_decOxygenA()
	{
		mr r30, r3;
		bl isGalaxyHorrorAndNoLightAreaAny;
	}
	kmCall(0x804002B8, MarioSwim_decOxygenA);
	// This is separate due to the fact that the function closer would overwrite r0
	asm void MarioSwim_decOxygenB()
	{
		cmpwi r3, 1;
		mr r3, r30;
		li r0, 0;
		beq Loc_A;
		lhz r0, 0xA0(r3);
	Loc_A:
	}
	kmCall(0x804002BC, MarioSwim_decOxygenB);


	// Fix the Comet Timer from not killing you if you are in Horror Mode and walking in a sink floor code
	bool isAllowCometTimerDeath()
	{
		return !(isHorrorModeActive() && !isHorrorInLightAny());
	}
	kmCall(0x803B5BC8, isAllowCometTimerDeath);



	// Found by IcoFriendly, if you were to lose the Horror powerup when in a state when the meter doesn't update, it doesn't go away
	void removeMeterIfNotHorror() {
		SubMeterLayout* pMeter = getMarioSubMeter(SUBMETER_HORROR);
		if (pMeter == NULL || pMeter != getCurrentMarioSubMeter())
			return;
		if (!isHorrorModeActive())
			pMeter->requestDeactivate();
	}
	kmBranch(0x803B94D0, removeMeterIfNotHorror);
}