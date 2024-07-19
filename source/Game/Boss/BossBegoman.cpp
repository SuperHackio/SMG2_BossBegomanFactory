#include "Game/Boss/BossBegoman.h"

#define SENSORNAME_BODY "Body"
#define SENSORNAME_TRAMPLE "Trample"
#define JOINTNAME_CENTER "Center"

MR::ActorMoveParam hWaitParam = {
	0.0f,
	3.0f,
	0.95f,
	1.0f
};

MR::ActorMoveParam hSignAttackParam = {
	0.0f,
	3.0f,
	0.8f,
	3.0f
};

MR::ActorMoveParam hPursueParam = {
	1.3f,
	3.f,
	0.97f,
	3.0f
};

MR::ActorMoveParam hTurnParam = {
	0.4f,
	3.0f,
	0.95f,
	0.0f
};

MR::ActorMoveParam hHitReactionParam = {
	0.0f,
	3.0f,
	0.95,
	0.0f
};

MR::ActorMoveParam hOnWeakParam = {
	-0.5f, //The speed when he stays away from the player
	3.0f,
	0.95f,
	3.0f
};

MR::ActorMoveParam hOnWeakNoMoveParam = {
	0.0f,
	3.0f,
	0.095f,
	3.0f
};

BegomanSound soundBoss = {
	"SE_BM_LV_BBEGO_DASH"
};

BossBegoman::BossBegoman(const char* pName) : BegomanBase(pName)
{
	mBabyStorage = NULL;
	mSpikeStorage = NULL;
	mBabyCount = NULL;
	mSpikeCount = NULL;
	mFollowerType = FOLLOWERKIND_NONE;
	mParabolicPath = NULL;
	mActorCameraInfo = NULL;
	mHead = NULL;
	mLife = 3;
	_150 = 0.2f;
	mMatrix.identity();
	mPowerStarId = -1;
}

void BossBegoman::init(const JMapInfoIter& rIter)
{
	initCore(rIter, "BossBegoman", true);
	MR::calcGravity(this);
	TVec3f vVec = TVec3f(this->mGravity);
	vVec.scale(10.f);
	_F4.sub(vVec);

	mHead = new BossBegomanHead(this, (MtxPtr)&mMatrix);
	mHead->initWithoutIter();
	MR::startBrk(mHead, "OffWait");

	initBinder(160.0f, 160.0f, 0);
	initNerve(&NrvBossBegoman::HostTypeNrvPreDemoWait::sInstance, 0);
	initSensor(1, 190.f, 250.f, JOINTNAME_CENTER);
	MR::addHitSensorAtJoint(this, SENSORNAME_TRAMPLE, JOINTNAME_CENTER, ATYPE_BEGOMAN, 8, 180.f * mScale.y, TVec3f(0.f, 0.f, 0.f));
	initEffect(0);
	MR::setEffectHostSRT(this, "EdgeSpark", NULL, NULL, NULL);
	initSound(6, "BossBegoman", NULL, TVec3f(0.f, 0.f, 0.f));
	initShadow(200.f, JOINTNAME_CENTER);
	MR::setShadowDropLength(this, NULL, 4000.f);

	MR::getJMapInfoArg0NoInit(rIter, &mPowerStarId);
	if (mPowerStarId != -1)
		MR::declarePowerStar(this, mPowerStarId);

	mParabolicPath = new ParabolicPath();


	// Completely new phase system via Object Arguments
	// A wee bit limited, but it will work

	// Life is defaulted in the Ctor
	MR::getJMapInfoArg1NoInit(rIter, &mLife);
	if (mLife < 1)
		mLife = 1; //Must at least take one hit...
	MR::declareStarPiece(this, (mLife * 8) + 0x10);

	// We will default to Battlerock Galaxy's phase setup. Don't go crazy with the enemy types though, too many and the game crashes due to collision overload...
	mBabyCount = 3;
	mSpikeCount = 2;
	mSpringCount = 0; // Spring is new, we won't allocate anything by default

	MR::getJMapInfoArg2NoInit(rIter, &mBabyCount);
	MR::getJMapInfoArg3NoInit(rIter, &mSpikeCount);
	MR::getJMapInfoArg4NoInit(rIter, &mSpringCount);

	mLifeUseBaby = mLife;
	mLifeUseSpike = 1;
	mLifeUseSpring = -1; //Do not enable springs by default
	MR::getJMapInfoArg5NoInit(rIter, &mLifeUseBaby);
	MR::getJMapInfoArg6NoInit(rIter, &mLifeUseSpike);
	MR::getJMapInfoArg7NoInit(rIter, &mLifeUseSpring);

	decideFollower();

	mBabyStorage = new BegomanBaby * [mBabyCount];
	for (s32 i = 0; i < mBabyCount; i++)
	{
		mBabyStorage[i] = new BegomanBaby("子分ベビー");
		mBabyStorage[i]->mTranslation.set(this->mTranslation);
		mBabyStorage[i]->initWithoutIter();
		mBabyStorage[i]->makeActorDead();
	}
	
	mSpikeStorage = new BegomanSpike * [mSpikeCount];
	for (s32 i = 0; i < mSpikeCount; i++)
	{
		mSpikeStorage[i] = new BegomanSpike("子分トゲ");
		mSpikeStorage[i]->mTranslation.set(this->mTranslation);
		mSpikeStorage[i]->initWithoutIter();
		mSpikeStorage[i]->makeActorDead();
	}
	
	mSpringStorage = new BegomanSpring * [mSpringCount];
	for (s32 i = 0; i < mSpringCount; i++)
	{
		mSpringStorage[i] = new BegomanSpring("子分トゲ"); //TODO: Fix this name.
		mSpringStorage[i]->mTranslation.set(this->mTranslation);
		mSpringStorage[i]->initWithoutIter();
		mSpringStorage[i]->makeActorDead();
	}

	if (MR::tryRegisterDemoCast(this, rIter))
	{
		MR::registerDemoActionNerve(this, &NrvBossBegoman::HostTypeNrvFirstContactDemo::sInstance, NULL);
		for (s32 i = 0; i < mBabyCount; i++)
			MR::tryRegisterDemoCast(mBabyStorage[i], rIter);
		for (s32 i = 0; i < mSpikeCount; i++)
			MR::tryRegisterDemoCast(mSpikeStorage[i], rIter);
		for (s32 i = 0; i < mSpringCount; i++)
			MR::tryRegisterDemoCast(mSpringStorage[i], rIter);
		MR::tryRegisterDemoCast(mHead, rIter);
	}

	mActorCameraInfo = new ActorCameraInfo(rIter);
	MR::initAnimCamera(this, mActorCameraInfo, "OpeningDemo");
	makeActorAppeared();
}

void BossBegoman::appear()
{
	BegomanBase::appear();
	MR::emitEffect(this, "Death");
	setNerve(&NrvBossBegoman::HostTypeNrvWait::sInstance);
}

void BossBegoman::kill()
{
	BegomanBase::kill();
	killAllFollower(FOLLOWERKIND_ALL);
	//Start after boss BGM?
	if (mPowerStarId != -1)
		MR::requestAppearPowerStar(this, mPowerStarId, this->mTranslation);
}

void BossBegoman::control()
{
	TVec3f vVec;
	MR::copyJointScale(this->mHead, "Edge", &vVec);

	//f32 fr3 = 1.f;
	//f32 fr1 = 0.34999999f;
	//f32 fr0 = 200.f;
	//f32 fr2 = fr3 - fr0;
	//fr1 = fr1 * fr2;
	//fr1 = fr3 - fr1;
	//fr1 = fr0 * fr1;

	//MR::setShadowVolumeSphereRadius(this, NULL, fr1);

	if (isNerve(&NrvBossBegoman::HostTypeNrvOnWeak::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvOnWeakTurn::sInstance))
	{
		getSensor(SENSORNAME_BODY)->mRadius = 160.f;
	}
	else
		getSensor(SENSORNAME_BODY)->mRadius = 190.f;

	checkTouchElectricRail(
		!(isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
			isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
			isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
			isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance) ||
			isNerve(&NrvBossBegoman::HostTypeNrvOnWeakTurn::sInstance))
	);

	if (isNerve(&NrvBossBegoman::HostTypeNrvPursue::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvTurn::sInstance))
	{
		register f32 low = 0.001f;
		register f32 val = _150;
		register f32 hi = 1.0f;

		__asm {
			fadds     val, low, val
			fcmpo     cr0, val, hi
			cror      eq, gt, eq
			bne locA
			b locB
			locA :
			fmr       hi, val
				locB :
		}
		_150 = hi;
	}
	else {
		_150 = 0.2f;
	}

	if (MR::isStep(this, 1))
	{
		if (isNerve(&NrvBossBegoman::HostTypeNrvWait::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvNoCalcWait::sInstance))
			MR::validateClipping(this);
		else
			MR::invalidateClipping(this);
	}

	BegomanBase::control();

	if (!isNerve(&NrvBossBegoman::HostTypeNrvPreDemoWait::sInstance) && !isNerve(&NrvBossBegoman::HostTypeNrvFirstContactDemo::sInstance))
		startRotationLevelSound();
}

void BossBegoman::setStepBackNerve()
{
	if (mHead->isEdgeOut())
		setNerve(&NrvBossBegoman::HostTypeNrvStepBack::sInstance);
	else
		setNerve(&NrvBossBegoman::HostTypeNrvStepBackOnWeak::sInstance);
}

bool BossBegoman::onTouchElectric(const TVec3f& rVecA, const TVec3f& rVecB)
{
	if (isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
		return false;

	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvOnWeakTurn::sInstance))
	{
		setNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance);
		return false;
	}

	if (isNerve(&NrvBossBegoman::HostTypeNrvStepBack::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvStepBackOnWeak::sInstance))
	{
		if (!MR::isOnGround(this))
			return false;
	}

	if (BegomanBase::onTouchElectric(rVecA, rVecB))
	{
		setStepBackNerve();
		return true;
	}
	return false;
}

bool BossBegoman::setNerveReturn()
{
	setNerve(&NrvBossBegoman::HostTypeNrvReturn::sInstance);
	return true;
}

Nerve* BossBegoman::getNerveWait()
{
	return &NrvBossBegoman::HostTypeNrvWait::sInstance;
}

void BossBegoman::addVelocityOnPushedFromElectricRail(const TVec3f& rVecA, const TVec3f& rVecB)
{
	TVec3f vVec = TVec3f(_F4);
	vVec.sub(getSensor(SENSORNAME_BODY)->mPosition);

	MR::vecKillElement(vVec, mGravity, &vVec);

	f32 RailPushVelHBoss;
	//Manually implement BegomanBase::getRailPushVelHBoss
	{
		RailPushVelHBoss = 30.f;
	}
	vVec.setLength(RailPushVelHBoss);
	mVelocity.add(vVec);

	f32 RailPushJumpBoss;
	//Manually implement BegomanBase::getRailPushJumpBoss
	{
		RailPushJumpBoss = 30.f;
	}

	TVec3f vGrav = mGravity;
	vGrav.scale(RailPushJumpBoss);
	mVelocity.add(vGrav);
}

bool BossBegoman::requestAttack()
{
	if (!isDeadAllFollower())
		return false;
	return BegomanBase::requestAttack();
}


void BossBegoman::exePreDemowait()
{
	if (MR::isFirstStep(this))
		MR::startAction(this, "PreDemoWait");

	MR::startActionSound(this, "BmLvBBegoPreDemoFly", -1, -1, -1);
	exeNoCalcWaitCore(0.0049999999f, NULL);
}
void BossBegoman::exeFirstContactDemo()
{
	if (MR::isFirstStep(this))
	{
		MR::startAction(this, "OpeningDemo");
		MR::startAnimCameraTargetSelf(this, this->mActorCameraInfo, "OpeningDemo", 0, false, 1.f);
		MR::showModel(this);
		MR::showModel(this->mHead);
		mHead->setOpeningDemo();
		mVelocity.zero();
		MR::stopStageBGM(0x3C); //Is this really needed?
		MR::overlayWithPreviousScreen(2);
	}

	if (MR::isActionEnd(this))
		setNerve(&NrvBossBegoman::HostTypeNrvReady::sInstance);
}
void BossBegoman::exeReady()
{
	if (MR::isFirstStep(this))
	{
		MR::startBossBGM(7); //Is this really needed?
		mVelocity.zero();
	}

	edgeRecoverCore();

	if (MR::isActionEnd(this) && MR::isAnimCameraEnd(this, this->mActorCameraInfo, "OpeningDemo"))
	{
		MR::endAnimCamera(this, this->mActorCameraInfo, "OpeningDemo", -1, false);
		tryLaunchFollower();
		setNerve(&NrvBossBegoman::HostTypeNrvWait::sInstance);
	}
}
void BossBegoman::exeSignAttack()
{
	if (MR::isFirstStep(this))
	{
		MR::startAction(this, "Attack");
		MR::startActionSound(this, "BmBBegoPrePursue", -1, -1, -1);
	}
	updateRotateY(0.5f, 0.005f);
	exeSignAttackCore(hSignAttackParam, &NrvBossBegoman::HostTypeNrvPursue::sInstance);
}
void BossBegoman::exePursue()
{
	if (MR::isFirstStep(this))
	{
		MR::startActionSound(this, "BmBBegoPursueStart", -1, -1, -1);
	}

	updateRotateY(0.5f, 0.005f);
	exePursueCore(hPursueParam, &NrvBossBegoman::HostTypeNrvBrake::sInstance, &NrvBossBegoman::HostTypeNrvTurn::sInstance, soundBoss, _150);
}
void BossBegoman::exeTurn()
{
	if (MR::isFirstStep(this))
		mHead->tryTurn();

	if (MR::isBckPlaying(this, "Turn"))
	{
		TVec3f vVec = TVec3f(_9C);
		vVec.scale(180.f);
		TVec3f vVec2 = TVec3f(mTranslation);
		vVec2.add(vVec);
		MR::emitEffectHit(this, vVec2, "EdgeSpark");
	}
	updateRotateY(0.4f, 0.005f);
	MR::startActionSound(this, "BmLvBBegoTurn", -1, -1, -1);
	exeTurnCore(hTurnParam, &NrvBossBegoman::HostTypeNrvBrake::sInstance, &NrvBossBegoman::HostTypeNrvPursue::sInstance, false);
}
void BossBegoman::exeOnWeak()
{
	if (MR::isFirstStep(this))
	{
		MR::startAction(this, "Shake");
		MR::startBrk(this->mHead, "OnWait");
	}
	updateRotateY(0.4f, 0.005f);

	if (MR::calcDistanceToPlayer(mTranslation) < 600.f)
	{
		MR::moveAndTurnToPlayer(this, &_9C, hOnWeakParam._0, hOnWeakParam._4, hOnWeakParam._8, hOnWeakParam._C);
		addVelocityEscapeToSide(.35f);
	}
	else
		MR::moveAndTurnToPlayer(this, &_9C, hOnWeakNoMoveParam._0, hOnWeakNoMoveParam._4, hOnWeakNoMoveParam._8, hOnWeakNoMoveParam._C);

	if (isNerve(&NrvBossBegoman::HostTypeNrvOnWeakTurn::sInstance))
	{
		if (!MR::isGreaterStep(this, 0x1E))
			return;
		setNerve(&NrvBossBegoman::HostTypeNrvOnWeak::sInstance);
	}

	if (MR::isGreaterStep(this, 0x1A4))
	{
		setNerve(&NrvBossBegoman::HostTypeNrvAware::sInstance);
		return;
	}

	if (MR::isStep(this, 0xF0))
	{
		MR::startBrk(this->mHead, "SignWait");
		MR::setBrkRate(this->mHead, 0.5f);
	}
	else if (MR::isStep(this, 0x168))
	{
		MR::startBrk(this->mHead, "SignWait");
		MR::setBrkRate(this->mHead, 2.0f);
	}

	if (MR::isGreaterEqualStep(this, 0x168))
	{
		MR::startActionSound(this, "BmLvBBegoAlarmFast", -1, -1, -1);
	}
	else if (MR::isGreaterEqualStep(this, 0xF0))
	{
		MR::startActionSound(this, "BmLvBBegoAlarm", -1, -1, -1);
	}
}
void BossBegoman::exeStepBack()
{
	if (MR::isFirstStep(this))
	{
		//what
	}
	updateRotateY(0.2f, 0.005f);

	exeStepBackCore(hWaitParam,
		isNerve(&NrvBossBegoman::HostTypeNrvStepBackOnWeak::sInstance) ? (Nerve*)&NrvBossBegoman::HostTypeNrvOnWeak::sInstance : (Nerve*)&NrvBossBegoman::HostTypeNrvWait::sInstance);
}
void BossBegoman::exeTrampleReaction()
{
	if (MR::isFirstStep(this))
	{
		mHead->trySwitchPushTrample();
		if (!mHead->isSwitchOn())
		{
			MR::startAction(this, "TrampleReaction");
			MR::startActionSound(this, "BmBBegoNeedleOn", -1, -1, -1);
		}
		else
		{
			MR::startAction(this, "HopEnd");
			MR::startActionSound(this, "BmBBegoStomped", -1, -1, -1);
			MR::startActionSound(this, "BmBBegoNeedleOff", -1, -1, -1);
		}
	}
	updateRotateY(0.25f, 0.005f);

	MR::moveAndTurnToPlayer(this, &_9C, 0.0f, 3.0f, 0.95f, 3.0f);//I guess they didn't want an ActorMoveParam for this

	if (mHead->isSwitchOn())
	{
		if (MR::isGreaterStep(this, 0x50))
			setNerve(&NrvBossBegoman::HostTypeNrvOnWeak::sInstance);
		return;
	}

	if (MR::isGreaterStep(this, 0x50))
	{
		setNerve(&NrvBossBegoman::HostTypeNrvSignAttack::sInstance);
	}
}
void BossBegoman::exeAware()
{
	if (MR::isFirstStep(this))
	{
		mVelocity.zero();
	}

	updateRotateY(0.1f, 0.005f);

	//MR::moveAndTurnToPlayer(this, &_9C, 0.f, 3.f, 0.95f, 0.f);
	edgeRecoverCore();

	if (MR::isGreaterStep(this, 0x3C))
	{
		tryLaunchFollower();
		setNerve(&NrvBossBegoman::HostTypeNrvSignAttack::sInstance);
	}
}
void BossBegoman::exeBlow()
{
	if (MR::isFirstStep(this))
	{
		MR::startActionSound(this, "EmBegomanRotStop", -1, -1, -1);
		MR::startAction(this, "Damage");
		MR::stopScene(2);
	}

	MR::startActionSound(this, "EmLvBegomanSpark", -1, -1, -1);

	MR::moveAndTurnToDirection(this, &_9C, _A8, hHitReactionParam._0, hHitReactionParam._4, hHitReactionParam._8, hHitReactionParam._C);
	reboundWallAndGround(&_9C, false);

	if (MR::isGreaterStep(this, 0x14) && MR::isOnGround(this))
	{
		MR::startAction(this, "Turn");

		if (mHead->isSwitchOn())
			setNerve(&NrvBossBegoman::HostTypeNrvOnWeakTurn::sInstance);
		else
			setNerve(&NrvBossBegoman::HostTypeNrvTurn::sInstance);
	}
}
void BossBegoman::exeElectricDeath()
{
	if (MR::isFirstStep(this))
	{
		MR::startAction(this, "electricshock");
		MR::startBrk(mHead, "Damage");

		mVelocity.zero();
		getSensor(SENSORNAME_BODY)->invalidate();
		MR::startActionSound(this, "EmBegomanElecDamage", -1, -1, -1);
		MR::invalidateClipping(this);
		mLife--;
	}

	if (MR::isStep(this, 0x1E) && mLife > 0)
	{
		TVec3f vStarPieceAppearVec = TVec3f(_F4);
		vStarPieceAppearVec.sub(mTranslation);
		MR::vecKillElement(vStarPieceAppearVec, mGravity, &vStarPieceAppearVec);
		MR::normalizeOrZero(&vStarPieceAppearVec);
		vStarPieceAppearVec.sub(mGravity);
		MR::normalizeOrZero(&vStarPieceAppearVec);

		TVec3f vVecA = TVec3f(mGravity);
		vVecA.scale(200.f);
		TVec3f vVecB = TVec3f(mTranslation);
		vVecB.sub(vVecA);
		//I'm doing it like this to fix a bug where the noise would still play
		//even if starbits were not spawned.
		//Admittedly just my paranoia
		s32 num = mLife > 1 ? 0x08 : 0x10; //Final phase spawns 16 starbits yipee
		if (MR::appearStarPieceToDirection(this, vVecB, vStarPieceAppearVec, num, 20.f, 40.f, false))
			MR::startActionSound(this, "OjStarPieceBurst", -1, -1, -1);
	}

	if (!MR::isGreaterStep(this, 0x3C))
		return;

	MR::stopScene(5);
	MR::shakeCameraWeak();

	if (mLife == 0)
	{
		MR::startActionSound(this, "BmBBegoDead", -1, -1, -1);
		kill();
		MR::emitEffect(this, "Death");
	}
	else
	{
		s32 previous = decideFollower();
		if (previous != mFollowerType)
			killAllFollower((BossBegoman::FollowerKind)previous);
		setNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance);
	}

	getSensor(SENSORNAME_BODY)->validate();
	MR::validateClipping(this);
}
void BossBegoman::edgeRecoverCore()
{
	if (MR::isFirstStep(this))
		MR::startAction(this, "Recover");

	if (MR::isStep(this, 0x37))
	{
		MR::startActionSound(this, "BmBBegoNeedleOn", -1, -1, -1);
		mHead->tryForceRecover();
		MR::tryRumblePadAndCameraDistanceMiddle(this, 800.f, 1200.f, 2000.f);
	}
}
void BossBegoman::exeElectricReturn()
{
	TVec3f vVec2C, vVec38;
	if (MR::isFirstStep(this) && ElectricRailFunction::isTouchRail(getSensor("check"), &vVec38, &vVec2C))
	{
		vVec38.sub(mTranslation);
		TVec3f vVec20 = TVec3f(vVec38);
		PSVECCrossProduct((const Vec*)&vVec2C, (const Vec*)&vVec20, (Vec*)&vVec20);
		TVec3f vVec14 = TVec3f(vVec20);
		PSVECCrossProduct((const Vec*)&vVec2C, (const Vec*)&vVec14, (Vec*)&vVec14);
		if (vVec38.dot(vVec14) < 0.f)
		{
			vVec14 = -vVec14;
		}
		vVec14.setLength(25.f);
		mVelocity.set(vVec14);
	}
	reboundWallAndGround(&_9C, false);

	MR::moveAndTurnToPlayer(this, &_9C, 0.f, 3.f, 0.8f, 0.f);

	if (MR::isGreaterStep(this, 0x3C))
	{
		_9C.set(_A8);
		setNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance);
	}
}
void BossBegoman::exeJumpToInitPos()
{
	if (MR::isFirstStep(this))
	{
		MR::startAction(this, "Shake");
		MR::startActionSound(this, "BmBBegoBigJump", -1, -1, -1);
		TVec3f v38 = TVec3f(_F4);
		v38.sub(mTranslation);
		v38.scale(0.5f);
		TVec3f v2C = TVec3f(mGravity);
		v2C = -v2C;
		TVec3f v8 = TVec3f(mGravity);
		v8.scale(10.f);
		TVec3f v14 = TVec3f(_F4);
		v14.sub(v8);

		mParabolicPath->initFromUpVector(mTranslation, v14, v2C, 700.f);
		mVelocity.zero();
	}

	updateRotateY(0.1f, 0.005f);

	mParabolicPath->calcPosition(&mTranslation, MR::calcNerveRate(this, 0x3C));

	if (MR::isGreaterStep(this, 0x3C))
	{
		setNerve(&NrvBossBegoman::HostTypeNrvSignAttack::sInstance);
		tryLaunchFollower();
		MR::startActionSound(this, "BmBBegoNeedleOn", -1, -1, -1);
		mHead->tryForceRecover();
		MR::tryRumblePadAndCameraDistanceMiddle(this, 800.f, 1200.f, 2000.f);
	}
}

void BossBegoman::tryLaunchFollower()
{
	if (!isDeadAllFollower())
		return;

	TVec3f v8 = TVec3f(*MR::getPlayerCenterPos());
	v8.sub(mTranslation);
	MR::vecKillElement(v8, mGravity, &v8);
	MR::normalizeOrZero(&v8);

	if (MR::isNearZero(v8, 0.001f))
	{
		v8.set(_9C);
	}

	if (mFollowerType == FOLLOWERKIND_BABY || mFollowerType == FOLLOWERKIND_ALL)
	{
		//Manually implementing BegomanBase::launchBegomanBabyFromGuarder((LiveActor *, BegomanBaby **, long, float, float, float, JGeometry::TVec3_f_ const *))
		BegomanBase::launchBegomanCore(this, (BegomanBase**)mBabyStorage, mBabyCount, 100.f, 10.f, 15.f, &v8);
		for (s32 i = 0; i < mBabyCount; i++)
		{
			mBabyStorage[i]->BegomanBase::appear();
			mBabyStorage[i]->setNerve(&NrvBegomanBaby::HostTypeNrvLaunchFromGuarder::sInstance);
		}
	}
	if (mFollowerType == FOLLOWERKIND_SPIKE || mFollowerType == FOLLOWERKIND_ALL)
	{
		//Manually implementing BegomanBase::launchBegoman((LiveActor *, BegomanBase **, long, float, float, float, JGeometry::TVec3_f_ const *))
		BegomanBase::launchBegomanCore(this, (BegomanBase**)mSpikeStorage, mSpikeCount, 100.f, 10.f, 20.f, &v8);
		for (s32 i = 0; i < mSpikeCount; i++)
		{
			mSpikeStorage[i]->appear();
			mSpikeStorage[i]->setNerveLaunch();
		}
	}
	if (mFollowerType == FOLLOWERKIND_SPRING || mFollowerType == FOLLOWERKIND_ALL)
	{
		//Manually implementing BegomanBase::launchBegoman((LiveActor *, BegomanBase **, long, float, float, float, JGeometry::TVec3_f_ const *))
		BegomanBase::launchBegomanCore(this, (BegomanBase**)mSpringStorage, mSpringCount, 100.f, 10.f, 20.f, &v8);
		for (s32 i = 0; i < mSpringCount; i++)
		{
			mSpringStorage[i]->appear();
			mSpringStorage[i]->setNerveLaunch();
		}
	}
}

void BossBegoman::killAllFollower(FollowerKind kind)
{
	if (kind == FOLLOWERKIND_BABY || kind == FOLLOWERKIND_ALL)
		killAllFollowerCore((BegomanBase**)mBabyStorage, mBabyCount);
	if (kind == FOLLOWERKIND_SPIKE || kind == FOLLOWERKIND_ALL)
		killAllFollowerCore((BegomanBase**)mSpikeStorage, mSpikeCount);
	if (kind == FOLLOWERKIND_SPRING || kind == FOLLOWERKIND_ALL)
		killAllFollowerCore((BegomanBase**)mSpringStorage, mSpringCount);
}
//Why didn't Nintendo do this? It saves on code space!
void BossBegoman::killAllFollowerCore(BegomanBase** pStorage, s32 Count)
{
	for (s32 i = 0; i < Count; i++)
	{
		if (MR::isDead(pStorage[i]))
			continue;
		MR::emitEffect(pStorage[i], "Death");
		pStorage[i]->kill();
	}
}

bool BossBegoman::isDeadAllFollower()
{
	if (mFollowerType == FOLLOWERKIND_BABY)
		return isDeadAllFollowerCore((BegomanBase**)mBabyStorage, mBabyCount);
	if (mFollowerType == FOLLOWERKIND_SPIKE)
		return isDeadAllFollowerCore((BegomanBase**)mSpikeStorage, mSpikeCount);
	if (mFollowerType == FOLLOWERKIND_SPRING)
		return isDeadAllFollowerCore((BegomanBase**)mSpringStorage, mSpringCount);

	return isDeadAllFollowerCore((BegomanBase**)mBabyStorage, mBabyCount) &&
		   isDeadAllFollowerCore((BegomanBase**)mSpikeStorage, mSpikeCount) &&
		   isDeadAllFollowerCore((BegomanBase**)mSpringStorage, mSpringCount);
}

bool BossBegoman::isDeadAllFollowerCore(BegomanBase** pStorage, s32 Count)
{
	for (s32 i = 0; i < Count; i++)
	{
		if (!MR::isDead(pStorage[i]))
			return false;
	}

	return true;
}

s32 BossBegoman::decideFollower()
{
	s32 previous = mFollowerType;
	// Triple threat mode!
	if (mLife == mLifeUseBaby && mLife == mLifeUseSpike && mLife == mLifeUseSpring)
	{
		mFollowerType = FOLLOWERKIND_ALL;
		return previous;
	}

	if (mLife == mLifeUseBaby)
		mFollowerType = FOLLOWERKIND_BABY;
	else if (mLife == mLifeUseSpike)
		mFollowerType = FOLLOWERKIND_SPIKE;
	else if (mLife == mLifeUseSpring)
		mFollowerType = FOLLOWERKIND_SPRING;
	return previous;
}

void BossBegoman::attackSensor(HitSensor* pSender, HitSensor* pReceiver)
{
	if (getSensor(SENSORNAME_BODY) != pSender)
		return;

	if (MR::isSensorEnemy(pReceiver))
	{
		MR::sendMsgEnemyAttack(pReceiver, pSender);
		return;
	}

	if (!MR::isSensorPlayer(pReceiver))
		return;

	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
	{
		MR::sendMsgPush(pReceiver, pSender);
		return;
	}

	if (isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
	{
		if (!MR::sendMsgEnemyAttackFlipRot(pReceiver, pSender))
			MR::sendMsgPush(pReceiver, pSender);
		return;
	}

	if (MR::isPlayerSwingAction())
		return;

	if (!MR::isOnGroundPlayer())
		return;

	if (!mHead->isEdgeOut() || !MR::isPlayerExistSide(this, 80.f, 0.25f))
	{
		MR::sendMsgEnemyAttackFlipRot(pReceiver, pSender);
		return;
	}

	if (!MR::sendMsgEnemyAttackFire(pReceiver, pSender))
	{
		MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
		return;
	}

	TVec3f v14 = TVec3f(pSender->mPosition);
	v14.sub(pReceiver->mPosition);
	MR::normalizeOrZero(&v14);

	if (!MR::isNearZero(v14, 0.001f))
	{
		bool x = reboundPlaneWithEffect(v14, 0.0f, 0.0f, "Spark");
		TVec3f v8 = TVec3f(v14);
		v8.scale(2.0f);
		mVelocity.add(v8);
		if (x)
			MR::startActionSound(this, "EmBegomanColli", -1, -1, -1);
	}

	setNerve(&NrvBossBegoman::HostTypeNrvHitReaction::sInstance);
}
bool BossBegoman::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver)
{
	getSensor(SENSORNAME_BODY); //what
	return false;
}
bool BossBegoman::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (pReceiver != getSensor(SENSORNAME_BODY))
		return false;

	if (MR::isMsgEnemyAttackElectric(msg))
	{
		TVec3f v2C = TVec3f(pSender->mActor->mVelocity);
		MR::vecKillElement(v2C, mGravity, &v2C);
		MR::normalizeOrZero(&v2C);
		return onTouchElectric(pSender->mPosition, v2C);
	}

	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
		return false;

	if (!MR::isSensorEnemy(pSender))
		return false;

	TVec3f v20 = TVec3f(pReceiver->mPosition);
	v20.sub(pSender->mPosition);
	MR::normalizeOrZero(&v20);
	if (MR::isNearZero(v20, 0.001f))
		return false;

	bool x = reboundPlaneWithEffect(v20, 0.f, 0.f, "Spark");

	f32 vf31 = pSender->mRadius / getSensor(SENSORNAME_BODY)->mRadius;

	TVec3f v8 = TVec3f(v20);
	v8.scale(6.f);
	TVec3f v14 = TVec3f(v8);
	v14.scale(vf31);
	MR::addVelocityLimit(this, v14);

	if (x)
	{
		MR::startActionSound(this, "BmBegomanColliBegoman", -1, -1, -1);
	}

	return true;
}
bool BossBegoman::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (MR::isMsgStarPieceReflect(msg))
		return true;

	if (MR::isMsgPlayerTrample(msg) && getSensor(SENSORNAME_TRAMPLE) == pReceiver)
		return receiveMsgTrample(pSender, pReceiver);

	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
		return false;

	if (MR::isMsgPlayerHipDrop(msg) && getSensor(SENSORNAME_TRAMPLE) == pReceiver)
	{
		MR::sendMsgAwayJump(pSender, pReceiver);
		setNerve(&NrvBossBegoman::HostTypeNrvTrampleReaction::sInstance);
		return true;
	}

	if (getSensor(SENSORNAME_BODY) != pReceiver)
		return false;

	if (MR::isMsgPlayerSpinAttack(msg) && !mHead->isSwitchOn() && MR::isPlayerExistSide(this, 80.f, 0.25f))
	{
		MR::sendMsgEnemyAttackFire(pSender, pReceiver);
		setNerve(&NrvBossBegoman::HostTypeNrvProvoke::sInstance);
		return false;
	}

	if (isNerve(&NrvBossBegoman::HostTypeNrvTrampleReaction::sInstance) && !MR::isGreaterStep(this, 0x1E))
		return false;

	if (!mHead->isEdgeOut() && MR::isMsgPlayerHitAll(msg))
	{
		calcBlowReaction(pSender->mPosition, pReceiver->mPosition, 40.f, 40.f);
		setNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance);
		return true;
	}

	return false;
}

bool BossBegoman::receiveMsgTrample(HitSensor* pSender, HitSensor* pReceiver)
{
	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
		return false;

	if (MR::isPlayerDamaging() || MR::isPlayerJumpRising())
		return false;

	if (isNerve(&NrvBossBegoman::HostTypeNrvTrampleReaction::sInstance) && MR::isLessStep(this, 5))
		return false;

	if (getSensor(SENSORNAME_BODY) == pReceiver)
		return false;

	if (getSensor(SENSORNAME_TRAMPLE) != pReceiver)
		return true;

	setNerve(&NrvBossBegoman::HostTypeNrvTrampleReaction::sInstance);
	TVec3f v2C = TVec3f(pReceiver->mPosition);
	v2C.sub(pSender->mPosition);
	MR::vecKillElement(v2C, mGravity, &v2C);
	MR::normalize(&v2C);
	//_9C.set(v2C);

	TVec3f v20 = TVec3f(*MR::getPlayerVelocity());
	MR::vecKillElement(v20, mGravity, &v20);
	if (!MR::isNearZero(v20, 0.001f))
	{
		TVec3f v14 = TVec3f(v20);
		PSVECCrossProduct((Vec*)&v14, (Vec*)&mGravity, (Vec*)&v14);
		v14.setLength(25.f);
		mVelocity.set(v14);
	}
	else
	{
		TVec3f v8;
		MR::getPlayerFrontVec(&v8);
		PSVECCrossProduct((Vec*)&v8, (Vec*)&mGravity, (Vec*)&v8);
		v8.setLength(25.f);
		mVelocity.set(v8);
	}

	return true;
}

bool BossBegoman::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver)
{
	if (isNerve(&NrvBossBegoman::HostTypeNrvBlow::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricDeath::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvElectricReturn::sInstance) ||
		isNerve(&NrvBossBegoman::HostTypeNrvJumpToInitPos::sInstance))
		return false;

	return MR::isMsgHitmarkEmit(msg);
}

void BossBegoman::calcAnim()
{
	LiveActor::calcAnim();

	PSMTXCopy(MR::getJointMtx(this, JOINTNAME_CENTER), (MtxPtr)&mMatrix);

	TVec3f vVec;
	vVec.set(mMatrix.mMtx[0][1], mMatrix.mMtx[1][1], mMatrix.mMtx[2][1]);

	if (!MR::isSameDirection(vVec, _A8, 0.01f))
	{
		MR::makeMtxUpFront(&mMatrix, vVec, _A8);
	}
}

void BossBegoman::startRotationLevelSound()
{
	if (isNerve(&NrvBossBegoman::HostTypeNrvWait::sInstance))
	{
		MR::startActionSound(this, "BmLvBBegoRotSlow", -1, -1, -1);
	}
	else if (isNerve(&NrvBossBegoman::HostTypeNrvPursue::sInstance))
	{
		MR::startActionSound(this, "BmLvBBegoPursue", -1, -1, -1);
	}
	else if (isNerve(&NrvBossBegoman::HostTypeNrvTrampleReaction::sInstance) || isNerve(&NrvBossBegoman::HostTypeNrvOnWeak::sInstance))
	{
		MR::startActionSound(this, "BmLvBBegoRotWeak", -1, -1, -1);
	}
	else
	{
		MR::startActionSound(this, "BmLvBBegoRotMiddle", -1, -1, -1);
	}

	if (!mHead->isSwitchOn())
	{
		f32 mag = PSVECMag((Vec*)&mVelocity);
		MR::startActionSound(this, "BmLvBBegoRotNeedle", MR::getLinerValueFromMinMax(mag * mag, 0.0f, 121.0f, 70.0f, 100.0f), -1, -1);
	}
}

u32 BossBegoman::getKind() const
{
	return BEGOMAN_TYPE_BOSS;
}


namespace NrvBossBegoman
{
	void HostTypeNrvPreDemoWait::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exePreDemowait();
	}

	void HostTypeNrvPreDemoWait::executeOnEnd(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->finishNoCalcWait();
	}
	HostTypeNrvPreDemoWait(HostTypeNrvPreDemoWait::sInstance);



	void HostTypeNrvFirstContactDemo::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeFirstContactDemo();
	}
	HostTypeNrvFirstContactDemo(HostTypeNrvFirstContactDemo::sInstance);



	void HostTypeNrvReady::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeReady();
	}
	HostTypeNrvReady(HostTypeNrvReady::sInstance);



	void HostTypeNrvNoCalcWait::execute(Spine* pSpine) const {
		BossBegoman* bg = ((BossBegoman*)pSpine->mExecutor);
		if (MR::isFirstStep(bg))
			bg->tryLaunchFollower();
		bg->exeNoCalcWaitCore(0.005f, &NrvBossBegoman::HostTypeNrvWait::sInstance);
	}

	void HostTypeNrvNoCalcWait::executeOnEnd(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->finishNoCalcWait();
	}
	HostTypeNrvNoCalcWait(HostTypeNrvNoCalcWait::sInstance);



	void HostTypeNrvWait::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		pBoss->updateRotateY(0.15f, 0.005f);
		pBoss->exeWaitCore(hWaitParam, &NrvBossBegoman::HostTypeNrvSignAttack::sInstance, &NrvBossBegoman::HostTypeNrvKeepDistance::sInstance, &NrvBossBegoman::HostTypeNrvNoCalcWait::sInstance);
	}
	HostTypeNrvWait(HostTypeNrvWait::sInstance);



	void HostTypeNrvSignAttack::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeSignAttack();
	}
	HostTypeNrvSignAttack(HostTypeNrvSignAttack::sInstance);



	void HostTypeNrvPursue::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exePursue();
	}
	HostTypeNrvPursue(HostTypeNrvPursue::sInstance);



	void HostTypeNrvTurn::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeTurn();
	}

	void HostTypeNrvTurn::executeOnEnd(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->mHead->tryTurnEnd();
	}
	HostTypeNrvTurn(HostTypeNrvTurn::sInstance);



	void HostTypeNrvOnWeak::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeOnWeak();
	}

	void HostTypeNrvOnWeak::executeOnEnd(Spine* pSpine) const {
		MR::startBrk(((BossBegoman*)pSpine->mExecutor)->mHead, "OffWait");
	}
	HostTypeNrvOnWeak(HostTypeNrvOnWeak::sInstance);



	void HostTypeNrvOnWeakTurn::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeOnWeak();
	}
	HostTypeNrvOnWeakTurn(HostTypeNrvOnWeakTurn::sInstance);



	void HostTypeNrvBrake::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		pBoss->updateRotateY(0.2f, 0.005f);
		MR::startActionSound(pBoss, "EmLvBegomanSpark", -1, -1, -1);
		pBoss->exeBrakeCore(&NrvBossBegoman::HostTypeNrvTurn::sInstance);
	}
	HostTypeNrvBrake(HostTypeNrvBrake::sInstance);



	void HostTypeNrvStepBack::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeStepBack();
	}
	HostTypeNrvStepBack(HostTypeNrvStepBack::sInstance);



	void HostTypeNrvStepBackOnWeak::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeStepBack();
	}
	HostTypeNrvStepBackOnWeak(HostTypeNrvStepBackOnWeak::sInstance);



	void HostTypeNrvReturn::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		pBoss->updateRotateY(0.2f, 0.005f);
		pBoss->exeReturnCore(&NrvBossBegoman::HostTypeNrvWait::sInstance);
	}
	HostTypeNrvReturn(HostTypeNrvReturn::sInstance);



	void HostTypeNrvProvoke::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		if (MR::isFirstStep(pBoss))
		{
			//what
		}
		pBoss->updateRotateY(0.3f, 0.005f);
		pBoss->exeProvokeCore(hWaitParam, &NrvBossBegoman::HostTypeNrvSignAttack::sInstance);
	}
	HostTypeNrvProvoke(HostTypeNrvProvoke::sInstance);



	void HostTypeNrvTrampleReaction::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeTrampleReaction();
	}
	HostTypeNrvTrampleReaction(HostTypeNrvTrampleReaction::sInstance);



	void HostTypeNrvAware::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeAware();
	}
	HostTypeNrvAware(HostTypeNrvAware::sInstance);



	void HostTypeNrvHitReaction::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		pBoss->updateRotateY(0.25f, 0.005f);
		pBoss->exeHitReactionCore(hHitReactionParam, &NrvBossBegoman::HostTypeNrvProvoke::sInstance);
	}
	HostTypeNrvHitReaction(HostTypeNrvHitReaction::sInstance);



	void HostTypeNrvBlow::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeBlow();
	}
	HostTypeNrvBlow(HostTypeNrvBlow::sInstance);



	void HostTypeNrvElectricDeath::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeElectricDeath();
	}
	HostTypeNrvElectricDeath(HostTypeNrvElectricDeath::sInstance);



	void HostTypeNrvElectricReturn::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeElectricReturn();
	}
	HostTypeNrvElectricReturn(HostTypeNrvElectricReturn::sInstance);



	void HostTypeNrvJumpToInitPos::execute(Spine* pSpine) const {
		((BossBegoman*)pSpine->mExecutor)->exeJumpToInitPos();
	}
	HostTypeNrvJumpToInitPos(HostTypeNrvJumpToInitPos::sInstance);



	void HostTypeNrvKeepDistance::execute(Spine* pSpine) const {
		BossBegoman* pBoss = ((BossBegoman*)pSpine->mExecutor);
		pBoss->updateRotateY(0.2f, 0.005f);
		pBoss->exeKeepDistanceCore(&NrvBossBegoman::HostTypeNrvWait::sInstance, &NrvBossBegoman::HostTypeNrvSignAttack::sInstance, &NrvBossBegoman::HostTypeNrvBrake::sInstance, 800.f, 600.f);
	}
	HostTypeNrvKeepDistance(HostTypeNrvKeepDistance::sInstance);
}