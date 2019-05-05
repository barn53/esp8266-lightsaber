#pragma once
#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>

namespace lightsaber {

class Light {
public:
    enum class Sequence {
        On,
        Change,
        BatteryLow,
        Off
    };

    Light();

    void begin();

    uint8_t beginSequence(Sequence sequence);
    void loop();

private:
    void onAnimation(const AnimationParam& param);
    void changeAnimation(const AnimationParam& param);
    void rainbowAnimation(const AnimationParam& param);
    void sirenAnimation(const AnimationParam& param);
    void offAnimation(const AnimationParam& param);
    void batteryLowAnimation_1(const AnimationParam& param);
    void batteryLowAnimation_2(const AnimationParam& param);

    static RgbColor colorForIndex(uint8_t index);
    static RgbColor rainbow(float progress);
    static NeoGamma<NeoGammaTableMethod> m_colorGamma;

    RgbColor m_color;
    RgbColor m_color_blend;
    uint8_t m_color_index{ 0 };

    const static uint16_t m_pixel_count = 24;
    const static uint8_t m_darken_by = 80;

    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> m_strip;
    NeoPixelAnimator m_animations;
};

} // namespace lightsaber
