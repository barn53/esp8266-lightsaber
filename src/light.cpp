#include "light.h"

namespace lightsaber {

Light::Light()
    : m_strip(m_pixel_count)
    , m_animations(2)
{
}

void Light::begin()
{
    m_strip.Begin();
    m_strip.Show();
}

void Light::onAnimation(const AnimationParam& param)
{
    float progress = NeoEase::QuinticOut(param.progress);
    m_color = colorForIndex(m_color_index);
    for (uint16_t index = 0; index < (m_pixel_count * progress); ++index) {
        m_strip.SetPixelColor(index, m_color);
    }
}
void Light::changeAnimation(const AnimationParam& param)
{
    if (param.state == AnimationState_Started) {
        m_color = m_strip.GetPixelColor(0);
        m_color_blend = colorForIndex(m_color_index);
    }
    if (param.progress <= 0.5f) {
        float progress = NeoEase::QuinticOut(param.progress * 2.0f);
        m_strip.ClearTo(RgbColor::LinearBlend(m_color, RgbColor(0x0, 0x0, 0x0), progress));
    } else {
        float progress = NeoEase::QuinticIn((param.progress - 0.5f) * 2.0f);
        m_strip.ClearTo(RgbColor::LinearBlend(RgbColor(0x0, 0x0, 0x0), m_color_blend, progress));
    }
}
void Light::rainbowAnimation(const AnimationParam& param)
{
    float progress = NeoEase::QuinticOut(param.progress);
    RgbColor color = rainbow(progress);
    m_strip.SetPixelColor(static_cast<uint16_t>(m_pixel_count * progress), color);
}
void Light::sirenAnimation(const AnimationParam& param)
{
    static bool toggler(true);
    RgbColor color1(0xff, 0x0, 0x0);
    RgbColor color2(0x0, 0x0, 0xff);
    color1.Darken(m_darken_by);
    color2.Darken(m_darken_by);
    color1 = m_colorGamma.Correct(color1);
    color2 = m_colorGamma.Correct(color2);

    if (param.state == AnimationState_Completed) {
        toggler = !toggler;
        for (uint16_t index = 0; index < m_pixel_count; ++index) {
            if (toggler
                    ? index <= (m_pixel_count / 2)
                    : index > (m_pixel_count / 2)) {
                m_strip.SetPixelColor(index, color1);
            } else {
                m_strip.SetPixelColor(index, color2);
            }
        }
        m_animations.RestartAnimation(param.index);
    }
}
void Light::offAnimation(const AnimationParam& param)
{
    float progress = NeoEase::QuinticIn(param.progress);
    m_strip.SetPixelColor(m_pixel_count - (m_pixel_count * progress), RgbColor(0, 0, 0));
}
void Light::batteryLowAnimation_1(const AnimationParam& param)
{
    float progress = NeoEase::QuinticIn(param.progress);
    m_strip.SetPixelColor(m_pixel_count - (m_pixel_count * progress), RgbColor(0, 0, 0));
    if (param.state == AnimationState_Completed) {
        m_animations.StartAnimation(0, 3000, std::bind(&Light::batteryLowAnimation_2, this, std::placeholders::_1));
    }
}
void Light::batteryLowAnimation_2(const AnimationParam& param)
{
    float progress = NeoEase::QuinticOut(param.progress);
    m_strip.SetPixelColor(m_pixel_count * progress, RgbColor::LinearBlend(RgbColor(0x33, 0x0, 0x0), RgbColor(0x0, 0x0, 0x0), progress));
}

RgbColor Light::colorForIndex(uint8_t index)
{
    RgbColor ret(0x0, 0x0, 0x0);
    switch (index) {
    case 0:
        ret = RgbColor(0x0, 0x0, 0xff);
        break;
    case 1:
        ret = RgbColor(0x0, 0xff, 0x0);
        break;
    case 2:
        ret = RgbColor(0xff, 0x0, 0x0);
        break;
    case 3:
        ret = RgbColor(0xff, 0x0, 0xdd);
        break;
    }
    ret.Darken(m_darken_by);
    ret = m_colorGamma.Correct(ret);
    return ret;
}

RgbColor Light::rainbow(float progress)
{
    RgbColor ret(0x0, 0x0, 0x0);
    if (progress <= (1.f / 6.f)) {
        // red - orange
        ret = RgbColor::LinearBlend(RgbColor(0xff, 0x0, 0x0), RgbColor(0xff, 0x7f, 0x0), progress * 6.0f);
    } else if (progress <= (2.f / 6.f)) {
        // orange - yellow
        ret = RgbColor::LinearBlend(RgbColor(0xff, 0x7f, 0x0), RgbColor(0xff, 0xff, 0x0), (progress - (1.0f / 6.0f)) * 6.0f);
    } else if (progress <= (3.f / 6.f)) {
        // yellow - green
        ret = RgbColor::LinearBlend(RgbColor(0xff, 0xff, 0x0), RgbColor(0x0, 0xff, 0x0), (progress - (2.0f / 6.0f)) * 6.0f);
    } else if (progress <= (4.f / 6.f)) {
        // green - turqoise
        ret = RgbColor::LinearBlend(RgbColor(0x0, 0xff, 0x0), RgbColor(0x0, 0xff, 0xff), (progress - (3.0f / 6.0f)) * 6.0f);
    } else if (progress <= (5.f / 6.f)) {
        // turqoise - blue
        ret = RgbColor::LinearBlend(RgbColor(0x0, 0xff, 0xff), RgbColor(0x0, 0x0, 0xff), (progress - (4.0f / 6.0f)) * 6.0f);
    } else if (progress <= (6.f / 6.f)) {
        // blue - violet
        ret = RgbColor::LinearBlend(RgbColor(0x0, 0x0, 0xff), RgbColor(0xff, 0x0, 0xff), (progress - (5.0f / 6.0f)) * 6.0f);
    }
    ret.Darken(m_darken_by);
    ret = m_colorGamma.Correct(ret);
    return ret;
}

uint8_t Light::beginSequence(Sequence sequence)
{
    m_animations.StopAnimation(0);
    m_animations.StopAnimation(1);
    switch (sequence) {
    case Sequence::On:
        if (m_color_index < 4) {
            m_animations.StartAnimation(0, 1500, std::bind(&Light::onAnimation, this, std::placeholders::_1));
        } else if (m_color_index == 4) {
            m_animations.StartAnimation(0, 1500, std::bind(&Light::rainbowAnimation, this, std::placeholders::_1));
        } else if (m_color_index == 5) {
            m_animations.StartAnimation(1, 100, std::bind(&Light::sirenAnimation, this, std::placeholders::_1));
        }
        break;
    case Sequence::Change:
        ++m_color_index;
        if (m_color_index > 5) {
            m_color_index = 0;
        }
        if (m_color_index < 4) {
            m_animations.StartAnimation(0, 3000, std::bind(&Light::changeAnimation, this, std::placeholders::_1));
        } else if (m_color_index == 4) {
            m_animations.StartAnimation(0, 1500, std::bind(&Light::rainbowAnimation, this, std::placeholders::_1));
        } else if (m_color_index == 5) {
            m_animations.StartAnimation(1, 100, std::bind(&Light::sirenAnimation, this, std::placeholders::_1));
        }
        break;
    case Sequence::BatteryLow:
        m_animations.StartAnimation(0, 2000, std::bind(&Light::batteryLowAnimation_1, this, std::placeholders::_1));
        break;
    case Sequence::Off:
        m_animations.StartAnimation(0, 1500, std::bind(&Light::offAnimation, this, std::placeholders::_1));
        break;
    default:
        break;
    }
    return m_color_index;
}

void Light::loop()
{
    m_animations.UpdateAnimations();
    m_strip.Show();
}

} // namespace lightsaber
