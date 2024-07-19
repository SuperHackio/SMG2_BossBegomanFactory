#include "Game/Boss/BossBegomanHead.h"

#include "Game/NameObj/NameObjCategories.h"
#include "Game/Util.h"

BossBegomanHead::BossBegomanHead(LiveActor* pParent, MtxPtr mtx) : PartsModel(pParent, "スイッチ頭", "BossBegomanHead", mtx, MR::DrawBufferType_Enemy, false) {
    _9C = 0.0f;
    mJointDeleg = NULL;
}

void BossBegomanHead::init(const JMapInfoIter& rIter) {
    initNerve(&NrvBossBegomanHead::HostTypeNrvDemoWait::sInstance, 0);

    JointControlDelegator<BossBegomanHead>* jointCtrl = new JointControlDelegator<BossBegomanHead>();
    jointCtrl->mObjPtr = this;
    jointCtrl->mCalcJointMtxFunc = &BossBegomanHead::calcJointEdge;
    jointCtrl->mCalcJointMtxAfterChildFunc = NULL;
    MR::setJointControllerParam(jointCtrl, this, "Edge");
    mJointDeleg = jointCtrl;

    PartsModel::init(rIter);
    MR::initLightCtrl(this);
    appear();
}

bool BossBegomanHead::isSwitchOn() {
    bool ret = false;
    if (isNerve(&NrvBossBegomanHead::HostTypeNrvDemoWait::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvOpeningDemo::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvOnWait::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvSwitchOn::sInstance)) {
        ret = true;
    }

    return ret;
}

bool BossBegomanHead::isEdgeOut() {
    bool ret = false;
    if (isNerve(&NrvBossBegomanHead::HostTypeNrvOffWait::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvSwitchOff::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvTurn::sInstance)
        || isNerve(&NrvBossBegomanHead::HostTypeNrvTurnEnd::sInstance)) {
        ret = true;
    }

    return ret;
}

void BossBegomanHead::setOpeningDemo() {
    setNerve(&NrvBossBegomanHead::HostTypeNrvOpeningDemo::sInstance);
}

void BossBegomanHead::trySwitchPushTrample() {
    setNerve(&NrvBossBegomanHead::HostTypeNrvSwitchOn::sInstance);
}

void BossBegomanHead::tryForceRecover() {
    setNerve(&NrvBossBegomanHead::HostTypeNrvSwitchOff::sInstance);
}

void BossBegomanHead::tryTurn() {
    setNerve(&NrvBossBegomanHead::HostTypeNrvTurn::sInstance);
}

void BossBegomanHead::tryTurnEnd() {
    setNerve(&NrvBossBegomanHead::HostTypeNrvTurnEnd::sInstance);
}

void BossBegomanHead::exeOffWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "OffWait");
    }

    _9C -= 0.23f;
}

void BossBegomanHead::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
    }

    _9C -= 0.2f;
}

void BossBegomanHead::exeTurnEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "TurnEnd");
    }

    _9C -= 0.23f;
    MR::setNerveAtBckStopped(this, &NrvBossBegomanHead::HostTypeNrvOffWait::sInstance);
}

void BossBegomanHead::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    mJointDeleg->registerCallBack();
}

bool BossBegomanHead::calcJointEdge(TPos3f* pMtx, const JointControllerInfo&) {
    f32 v3 = _9C;
    TVec3f v8;
    v8.x = 0.0;
    v8.y = 1.0;
    v8.z = 0.0;
    TPos3f v9;
    TVec3f v7;
    v7.x = 0.0;
    v7.y = 1.0;
    v7.z = 0.0;
    v9.zeroTrans();
    v9.setRotate(v7, v3);
    pMtx->concat(*pMtx, v9);
    return true;
}

void BossBegomanHead::exeSwitchOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Off");
    }
    MR::setNerveAtBckStopped(this, &NrvBossBegomanHead::HostTypeNrvOffWait::sInstance);
}

void BossBegomanHead::exeSwitchOn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "On");
    }
}

void BossBegomanHead::exeOnWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "OnWait");
    }
}

void BossBegomanHead::exeOpeningDemo() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "OpeningDemo");
    }
}

void BossBegomanHead::exeDemoWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DemoWait");
    }
}

namespace NrvBossBegomanHead {
    HostTypeNrvDemoWait(HostTypeNrvDemoWait::sInstance);
    void HostTypeNrvDemoWait::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeDemoWait();
    }

    HostTypeNrvOpeningDemo(HostTypeNrvOpeningDemo::sInstance);
    void HostTypeNrvOpeningDemo::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeOpeningDemo();
    }

    HostTypeNrvOnWait(HostTypeNrvOnWait::sInstance);
    void HostTypeNrvOnWait::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeOnWait();
    }

    HostTypeNrvOffWait(HostTypeNrvOffWait::sInstance);
    void HostTypeNrvOffWait::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeOffWait();
    }

    HostTypeNrvSwitchOn(HostTypeNrvSwitchOn::sInstance);
    void HostTypeNrvSwitchOn::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeSwitchOn();
    }

    HostTypeNrvSwitchOff(HostTypeNrvSwitchOff::sInstance);
    void HostTypeNrvSwitchOff::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeSwitchOff();
    }

    HostTypeNrvTurn(HostTypeNrvTurn::sInstance);
    void HostTypeNrvTurn::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeTurn();
    }

    HostTypeNrvTurnEnd(HostTypeNrvTurnEnd::sInstance);
    void HostTypeNrvTurnEnd::execute(Spine* pSpine) const {
        BossBegomanHead* arm = reinterpret_cast<BossBegomanHead*>(pSpine->mExecutor);
        arm->exeTurnEnd();
    }
};

BossBegomanHead::~BossBegomanHead() {

}