#pragma once

#include "Game/LiveActor/PartsModel.h"
#include "Game/Util/JointController.h"

class BossBegomanHead : public PartsModel {
public:
    BossBegomanHead(LiveActor*, MtxPtr);

    virtual ~BossBegomanHead();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    bool calcJointEdge(TPos3f*, const JointControllerInfo&);

    bool isSwitchOn();
    bool isEdgeOut();
    void setOpeningDemo();
    void trySwitchPushTrample();
    void tryForceRecover();
    void tryTurn();
    void tryTurnEnd();
    void exeOffWait();
    void exeTurn();
    void exeTurnEnd();
    inline void exeSwitchOff();
    inline void exeSwitchOn();
    inline void exeOnWait();
    inline void exeOpeningDemo();
    inline void exeDemoWait();

    f32 _9C;
    JointControlDelegator<BossBegomanHead>* mJointDeleg;         // _A0
};

namespace NrvBossBegomanHead {
    NERVE(HostTypeNrvDemoWait);
    NERVE(HostTypeNrvOpeningDemo);
    NERVE(HostTypeNrvOnWait);
    NERVE(HostTypeNrvOffWait);
    NERVE(HostTypeNrvSwitchOn);
    NERVE(HostTypeNrvSwitchOff);
    NERVE(HostTypeNrvTurn);
    NERVE(HostTypeNrvTurnEnd);
};