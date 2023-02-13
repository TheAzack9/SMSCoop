#pragma once

#include <SMS/Player/Mario.hxx>

namespace Coop {
    void initPlayer(TMario *mario, bool isMario);
    void onStageInit(TMarDirector *tApplication);
}  // namespace Coop

class TLuigi : public TMario {
public:
    static JDrama::TNameRef *instantiate() { return new TLuigi(); }
    TLuigi();

    virtual void load(JSUMemoryInputStream &stream) override { 
        TMario::load(stream);
    }
    virtual void loadAfter() override { 
        TMario::loadAfter();
    }
    virtual void perform(u32 param_1, JDrama::TGraphics* graphics) override {
        TMario::perform(param_1, graphics);
    }
    virtual bool receiveMessage(THitActor *receiver, u32 msg) override {
        return TMario::receiveMessage(receiver, msg);
    }
    virtual Mtx44 *getTakingMtx() override {
        return TMario::getTakingMtx();
    }
    virtual u32 moveRequest(const TVec3f &destPosition) override {
        return TMario::moveRequest(destPosition);
    }
    virtual void initValues() override {
        TMario::initValues();
    }
    virtual void checkReturn() override {
        TMario::checkReturn();
    }
    void checkController() override {
        //TMario::checkController();
    }
    virtual void playerControl(JDrama::TGraphics *graphics) override {
        TMario::playerControl(graphics);
    }
    virtual void drawSpecial(JDrama::TGraphics *graphics) override {
        TMario::drawSpecial(graphics);
    }
    virtual void checkCollision() override {
        TMario::checkCollision();
    }
    virtual void damageExec(THitActor *actor, int param_1, int param_2, int param_3, f32 param_4, int param_5, f32 param_6, s16 param_7) override {
        TMario::damageExec(actor, param_1, param_2, param_3, param_4, param_5, param_6, param_7);
    }
    virtual u8 getVoiceStatus() override {
        return TMario::getVoiceStatus();
    }
    virtual void drawSyncCallback(u16 status) override {
        TMario::drawSyncCallback(status);
    }

    void initModel() override;
};