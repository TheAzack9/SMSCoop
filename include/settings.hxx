#pragma once
#include <BetterSMS/settings.hxx>


namespace SMSCoop {
    class CameraTypeSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { REGULAR, VERTICAL, HORIZONTAL, SINGLE };

    
        CameraTypeSetting(const char *name) : IntSetting(name, &mCameraType), mCameraType(VERTICAL) {
            mValueRange = {0, 3, 1};
        }
        ~CameraTypeSetting() override {}
        
        void prevValue() override { 
            IntSetting::prevValue();
            
        }
        void nextValue() override { 
            IntSetting::nextValue();
        }

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
    
}