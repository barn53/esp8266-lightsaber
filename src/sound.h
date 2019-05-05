#pragma once
#include <Arduino.h>
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>

namespace lightsaber {

// implement a notification class,
// its member methods will get called
//
struct Mp3Notify {
    static void OnError(uint16_t errorCode);
    static void OnPlayFinished(uint16_t track);
    static void OnCardOnline(uint16_t code);
    static void OnUsbOnline(uint16_t code);
    static void OnCardInserted(uint16_t code);
    static void OnUsbInserted(uint16_t code);
    static void OnCardRemoved(uint16_t code);
    static void OnUsbRemoved(uint16_t code);
};

class Sound {
    static const uint8_t ADVERT_SOUND_ON = 1;
    static const uint8_t ADVERT_SOUND_OFF = 2;
    static const uint8_t ADVERT_SOUND_CHANGE = 3;
    static const uint8_t ADVERT_SOUND_RAINBOW = 4;
    static const uint8_t ADVERT_SOUND_SIREN = 5;
    static const uint8_t ADVERT_SOUND_BATTERY_LOW = 6;
    static const uint8_t ADVERT_SOUND_LOUDER = 7;
    static const uint8_t ADVERT_SOUND_QUIETER = 8;
    static const uint8_t NUMBER_SYSTEM_SOUNDS = 8;
    static const int8_t MAX_VOLUME = 30;

public:
    Sound();

    void begin();
    void loop();

    void volumeUp();
    void volumeDown();

    void playEndlessHum();
    void silence();
    void pauseResume();
    void advert(bool previous);
    void playChange(uint8_t forColorIndex);
    void playOn();
    void playOff();
    void playBatteryLow();
    void story(bool previous);

private:
    SoftwareSerial mp3Serial;
    DFMiniMp3<SoftwareSerial, Mp3Notify> mp3;

    uint16_t m_total_track_count{ 0 };
    uint16_t m_total_folder_count{ 0 };
    uint16_t m_folder_1_track_count{ 0 };
    uint16_t m_folder_2_track_count{ 0 };
    uint16_t m_advert_track_count{ 0 };

    int16_t m_advert_index{ NUMBER_SYSTEM_SOUNDS };
    int16_t m_story_index{ 0 };
    int8_t m_volume{ 20 };
    bool m_pause{ false };
};

} // namespace lightsaber
