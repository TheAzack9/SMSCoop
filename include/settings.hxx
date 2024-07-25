#pragma once
#include <BetterSMS/settings.hxx>


namespace SMSCoop {
    // This is bad, but i give up...
    static const bool isMemoryExpanded2() {
        return *((u32*)0x800000f0) == 0x04000000;
    }

    

    class PlayerTypeSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { MARIO, LUIGI, SHADOW_MARIO };

    
        PlayerTypeSetting(const char *name, int playerId) : IntSetting(name, &mPlayerType), mPlayerType(playerId) {
            mValueRange = {0, 3, 1};
            mIsUserEditable = playerId == 1 || isMemoryExpanded2();
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
        void load(JSUMemoryInputStream &in) override {

            int x;
            in.read(&x, 4);

            // Reset value if corrupt
            if (x < mValueRange.mStart || x > mValueRange.mStop)
                x = mValueRange.mStart;

            // Reset value if no longer editable
            if(!mIsUserEditable) x = mValueRange.mStart;

            setInt(x);
        }

        void changeCamera(int cameraType) {
            if(mPlayerId == 0) {
                if(cameraType == 0) {
                    mIsUserEditable = true;
                } else {
                    mIsUserEditable = isMemoryExpanded2();
                    setInt(Kind::MARIO);
                }
            } else {
                mIsUserEditable = cameraType != 0;
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

    
        CameraTypeSetting(const char *name, PlayerTypeSetting* player1Setting, PlayerTypeSetting* player2Setting) : IntSetting(name, &mCameraType), mCameraType(VERTICAL) {
            mValueRange = {0, 3, 1};
            mPlayer1Setting = player1Setting;
            mPlayer2Setting = player2Setting;
        }
        ~CameraTypeSetting() override {}
        
        void prevValue() override { 
            IntSetting::prevValue();
            mPlayer1Setting->changeCamera(getInt());
            mPlayer2Setting->changeCamera(getInt());
            
        }
        void nextValue() override { 
            IntSetting::nextValue();
            mPlayer1Setting->changeCamera(getInt());
            mPlayer2Setting->changeCamera(getInt());
        }

        void load(JSUMemoryInputStream &in) override {

            int x;
            in.read(&x, 4);

            // Reset value if corrupt
            if (x < mValueRange.mStart || x > mValueRange.mStop)
                x = mValueRange.mStart;

            setInt(x);
            mPlayer1Setting->changeCamera(x);
            mPlayer2Setting->changeCamera(getInt());
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
        PlayerTypeSetting* mPlayer1Setting;
        PlayerTypeSetting* mPlayer2Setting;
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