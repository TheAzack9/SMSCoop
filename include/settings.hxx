#pragma once
#include <BetterSMS/settings.hxx>


namespace SMSCoop {

    class PlayerTypeSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { MARIO, LUIGI, SHADOW_MARIO };

    
        PlayerTypeSetting(const char *name, int playerId) : IntSetting(name, &mPlayerType), mPlayerType(playerId) {
            mValueRange = {0, 3, 1};
            mIsUserEditable = true;
            mPlayerId = playerId;
            mHidden = false;
        }
        ~PlayerTypeSetting() override {}

        void getValueName(char *dst) const override {
            switch (getInt()) {
            default:
            case Kind::MARIO:
                strncpy(dst, "MARIO", 6);
                break;
            case Kind::LUIGI:
                strncpy(dst, "LUIGI", 6);
                break;
            case Kind::SHADOW_MARIO:
                strncpy(dst, "SHADOW MARIO", 13);
                break;
            }
        }

    private:
        int mPlayerType;
        int mPlayerId;
        bool mHidden;
    };
    
    class CameraTypeSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { REGULAR, VERTICAL, HORIZONTAL, SINGLE };

    
        CameraTypeSetting(const char *name) : IntSetting(name, &mCameraType), mCameraType(VERTICAL) {
            mValueRange = {0, 3, 1};
        }
        ~CameraTypeSetting() override {}
        
        void load(JSUMemoryInputStream &in) override {

            int x;
            in.read(&x, 4);

            // Reset value if corrupt
            if (x < mValueRange.mStart || x > mValueRange.mStop)
                x = mValueRange.mStart;

            setInt(x);
        }

        void getValueName(char *dst) const override {
            switch (getInt()) {
            default:
            case Kind::VERTICAL:
                strncpy(dst, "Vertical Splitscreen", 21);
                break;
            case Kind::HORIZONTAL:
                strncpy(dst, "Horizontal Splitscreen", 23);
                break;
            case Kind::SINGLE:
                strncpy(dst, "Multiplayer camera", 19);
                break;
            case Kind::REGULAR:
                strncpy(dst, "Singleplayer", 13);
                break;
            }
        }

    private:
        int mCameraType;
    };

    class ShineGrabDistanceSetting final : public BetterSMS::Settings::BoolSetting {
    public:
    
        ShineGrabDistanceSetting(const char *name) : BoolSetting(name, &mEnableShineGrabRange), mEnableShineGrabRange(true) {
        }
        ~ShineGrabDistanceSetting() override {}


    private:
        bool mEnableShineGrabRange;
    };
    
    class SpeedrunSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { DISABLED, SPEEDRUN, PRACTICE };
    
        SpeedrunSetting(const char *name) : IntSetting(name, &mSpeedrunState), mSpeedrunState(0) {
            mValueRange = {0, 2, 1};
        }
        ~SpeedrunSetting() override {}
        
        void getValueName(char *dst) const override {
            switch (getInt()) {
            default:
            case Kind::DISABLED:
                strncpy(dst, "Disabled", 9);
                break;
            case Kind::SPEEDRUN:
                strncpy(dst, "Enabled", 8);
                break;
            case Kind::PRACTICE:
                strncpy(dst, "Practice mode", 14);
                break;
            }
        }

    private:
        int mSpeedrunState;
    };
    
    class ExplainSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { EXPLAIN };
    
        ExplainSetting(const char *name, const char* text, int length) : IntSetting(name, &mExplainState), mExplainState(0) {
            mValueRange = {0, 0, 1};
            mText = text;
            mLength = length;
            mIsUserEditable = false;
        }
        ~ExplainSetting() override {}
        
        void getValueName(char *dst) const override {
            switch (getInt()) {
            default:
            case Kind::EXPLAIN:
                strncpy(dst, mText, mLength);
                break;
            }
        }

    private:
        int mExplainState;
        const char* mText;
        u32 mLength;
    };
}