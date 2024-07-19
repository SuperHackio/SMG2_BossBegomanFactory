#pragma once

#include "syati.h"
#include "Game/Boss/BossBegomanHead.h"

// Greetings!
// If you're reading this, you're probably browsing the source code of the repository
//
// The code inside BossBegoman.h & BossBegoman.cpp was originally written at some point in 2021 or 2022. Can't remember.
// BossBegomanHead.h and BossBegoman.cpp were also written at that point in time, however, with BossBegoman.cpp, I was unable to reverse engineer CalcJointEdge
// So at that point in time, I just used a custom GLE build to port the ASM directly from SMG1.
//
// It worked great, actually! Unfortunately, it would be irresponsible of me to publish something super jank like that, so I stashed it away for the time being.
// Some years later and Shibboleet decompiled the missing CalcJointEdge (though, at the time of writing this, I'm not even using the proper decompiled code since it doesn't work in SMG2)
// Shibboleet only decompiled BossBegomanHead as of the time of me writing this, so I took my old code for BossBegoman.h & .cpp and inserted it into the module. That's what you see here.
//
// Now, this port of Topmaniac was intended to be more flexable than the original vanilla one, therefore the Object Arguments have changed quite a bit.
// This is also why the code was not submitted to Petari, as it is not 1:1 (Though, I also didn't have acces to Petari back then).
//
// ObjArg0 - Power Star ID. If set to -1, no Power Star will be used. You can see if Topmaniac has died via SW_DEAD instead, in that case.
// ObjArg1 - Health. Defaults to 3. This represents the number of hits Topmaniac needs in order to be defeated.
// ObjArg2 - Baby Topmen Count. Defaults to 3. The number of babies that will be created.
// ObjArg3 - Spike Topmen Count. Defaults to 2. The number of spike topmen that will be created.
// ObjArg4 - Spring Topmen Count. Defaults to 0. The number of Spring topmen that will be created. This feature is exclusive to this port and is not found in SMG1.
// ObjArg5 - Baby Topmen Health trigger. Defaults to ObjArg1. The health that Topmaniac will switch to using Baby Topmen at.
// ObjArg6 - Spike Topmen Health trigger. Defaults to 1. The health that Topmaniac will switch to using Spike Topmen at.
// ObjArg7 - Spring Topmen Health trigger. Defaults to -1 (disabled). The health that Topmaniac will switch to using Spring Topmen at. This feature is exclusive to this port and is not found in SMG1.
//
// If the last 3 obj args are the same value, there's a special case where the game will spawn all 3 types at once for a topman tribe extravaganza!
// When Topmaniac's health reaches the value that is used for one of the last 3 args, he will switch to that type of topmen spawning.
// For example, if Topmaniac has 7 health, and you set ObjArg6 to 3, after you hit Topmaniac 4 times, he'll switch to using Spike Topmen.
// You can trigger the type switching in any order, but you can only trigger them once!
// If none of the Topmen types are used at the start of the fight, (or if you change to a topman type that has 0 topmen in it) you can fight Topmaniac on his own.
//
// One final note, the original Obj Args are no longer available.

class BossBegoman : public BegomanBase {
public:
	enum FollowerKind {
		FOLLOWERKIND_BABY = 0,
		FOLLOWERKIND_SPIKE = 1,
		FOLLOWERKIND_SPRING = 2, // NEW
		FOLLOWERKIND_ALL = 3,    // Moved from 2 to 3
		FOLLOWERKIND_NONE = 4    // NEW
	};

	BossBegoman(const char*);

	virtual void init(const JMapInfoIter& rIter);

	virtual void calcAnim();

	virtual void appear();

	virtual void kill();

	virtual void control();

	virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
	virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
	virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
	virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
	virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

	virtual u32 getKind() const;
	virtual bool onTouchElectric(const TVec3f&, const TVec3f&);
	virtual bool setNerveReturn();
	virtual Nerve* getNerveWait();
	virtual void addVelocityOnPushedFromElectricRail(const TVec3f&, const TVec3f&);
	virtual bool requestAttack();

	void setStepBackNerve();

	void exePreDemowait();
	void exeFirstContactDemo();
	void exeReady();
	void exeSignAttack();
	void exePursue();
	void exeTurn();
	void exeOnWeak();
	void exeStepBack();
	void exeTrampleReaction();
	void exeAware();
	void exeBlow();
	void exeElectricDeath();

	void edgeRecoverCore(); //right in the middle here lol

	void exeElectricReturn();
	void exeJumpToInitPos();

	void tryLaunchFollower();
	void killAllFollower(FollowerKind);
	void killAllFollowerCore(BegomanBase**, s32); //NEW
	bool isDeadAllFollower();
	bool isDeadAllFollowerCore(BegomanBase**, s32);
	s32 decideFollower(); // NEW. Returns the previous follower type

	bool receiveMsgTrample(HitSensor* pSender, HitSensor* pReceiver);

	void startRotationLevelSound();

	//These are in SMG1 terms
	BegomanBaby** mBabyStorage; //_100
	BegomanSpike** mSpikeStorage; //_104
	BegomanSpring** mSpringStorage; // NEW
	s32 mBabyCount; //_108
	s32 mSpikeCount; //_10C
	s32 mSpringCount; // NEW
	s32 mFollowerType; //_110
	s32 mLifeUseBaby; // NEW. When mLife reaches this value, switch to using this follower type
	s32 mLifeUseSpike; // NEW. ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	s32 mLifeUseSpring; // NEW. ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	ParabolicPath* mParabolicPath; //_114
	BossBegomanHead* mHead; //_118
	TPos3f mMatrix; //_11C
	s32 mLife; //_14C
	f32 _150; //_150
	ActorCameraInfo* mActorCameraInfo; //_154
	s32 mPowerStarId; // NEW
};

namespace NrvBossBegoman {
	ENDABLE_NERVE(HostTypeNrvPreDemoWait);
	NERVE(HostTypeNrvFirstContactDemo);
	NERVE(HostTypeNrvReady);
	ENDABLE_NERVE(HostTypeNrvNoCalcWait);
	NERVE(HostTypeNrvWait);
	NERVE(HostTypeNrvSignAttack);
	NERVE(HostTypeNrvPursue);
	ENDABLE_NERVE(HostTypeNrvTurn);
	ENDABLE_NERVE(HostTypeNrvOnWeak);
	NERVE(HostTypeNrvOnWeakTurn);
	NERVE(HostTypeNrvBrake);
	NERVE(HostTypeNrvStepBack);
	NERVE(HostTypeNrvStepBackOnWeak);
	NERVE(HostTypeNrvReturn);
	NERVE(HostTypeNrvProvoke);
	NERVE(HostTypeNrvTrampleReaction);
	NERVE(HostTypeNrvAware);
	NERVE(HostTypeNrvHitReaction);
	NERVE(HostTypeNrvBlow);
	NERVE(HostTypeNrvElectricDeath);
	NERVE(HostTypeNrvElectricReturn);
	NERVE(HostTypeNrvJumpToInitPos);
	NERVE(HostTypeNrvKeepDistance);
}