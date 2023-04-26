#pragma once
#include <BetterSMS/settings.hxx>


namespace SMSCoop {
    class SplitScreenSetting final : public BetterSMS::Settings::IntSetting {
    public:
        enum Kind { NONE, VERTICAL, HORIZONTAL };

    
        SplitScreenSetting(const char *name) : IntSetting(name, &mSplitScreenValue), mSplitScreenValue(VERTICAL) {
            mValueRange = {1, 2, 1};
        }
        ~SplitScreenSetting() override {}

        void getValueStr(char *dst) const override {
            switch (getInt()) {
            default:
            case Kind::VERTICAL:
                strncpy(dst, "VERTICAL", 9);
                break;
            case Kind::HORIZONTAL:
                strncpy(dst, "HORIZONTAL", 11);
                break;
            case Kind::NONE:
                strncpy(dst, "NONE", 5);
                break;
            }
        }

    private:
        int mSplitScreenValue;
    };
}