#include "sound.h"

namespace lightsaber {

Sound::Sound()
    : mp3Serial(D1, D6) // RX, TX
    , mp3(mp3Serial)
{
}

void Sound::begin()
{
    mp3.begin();
    mp3.setVolume(m_volume);

    m_total_track_count = mp3.getTotalTrackCount();
    m_total_folder_count = mp3.getTotalFolderCount();
    m_folder_1_track_count = mp3.getFolderTrackCount(1);
    m_folder_2_track_count = mp3.getFolderTrackCount(2);
    m_advert_track_count = m_total_track_count - m_folder_1_track_count - m_folder_2_track_count;

    Serial.printf("getTotalTrackCount: %u\n", m_total_track_count);
    Serial.printf("getTotalFolderCount: %u\n", m_total_folder_count);
    Serial.printf("getFolderTrackCount(1): %u\n", m_folder_1_track_count);
    Serial.printf("getFolderTrackCount(2): %u\n", m_folder_2_track_count);
    Serial.printf("Advert Track Count: %u\n", m_advert_track_count);
}

void Sound::loop()
{
    mp3.loop();
}

void Sound::volumeUp()
{
    m_volume += 8;
    if (m_volume > MAX_VOLUME) {
        m_volume = MAX_VOLUME;
    }
    mp3.setVolume(m_volume);
    mp3.playAdvertisement(ADVERT_SOUND_LOUDER);
    Serial.printf("Volume: %u\n", m_volume);
}

void Sound::volumeDown()
{
    m_volume -= 8;
    if (m_volume < 0) {
        m_volume = 0;
    }
    mp3.playAdvertisement(ADVERT_SOUND_QUIETER);
    mp3.setVolume(m_volume);
    Serial.printf("Volume: %u\n", m_volume);
}

void Sound::playEndlessHum()
{
    mp3.stop();
    m_story_index = 0;
    mp3.playFolderTrack16(1, 1);
}
void Sound::silence()
{
    mp3.stop();
    mp3.playFolderTrack16(1, 2);
}

void Sound::pauseResume()
{
    if (m_pause) {
        m_pause = false;
        mp3.start();
    } else {
        m_pause = true;
        mp3.pause();
    }
}

void Sound::advert(bool previous)
{
    if (previous) {
        --m_advert_index;
        if (m_advert_index <= NUMBER_SYSTEM_SOUNDS) {
            m_advert_index = m_advert_track_count;
        }
    } else {
        ++m_advert_index;
        if (m_advert_index > m_advert_track_count) {
            m_advert_index = NUMBER_SYSTEM_SOUNDS + 1;
        }
    }
    Serial.printf("Advert Index: %i\n", m_advert_index);
    mp3.playAdvertisement(m_advert_index);
}

void Sound::playChange(uint8_t forColorIndex)
{
    if (forColorIndex < 4) {
        mp3.playAdvertisement(ADVERT_SOUND_CHANGE);
    } else if (forColorIndex == 4) {
        mp3.playAdvertisement(ADVERT_SOUND_RAINBOW);
    } else if (forColorIndex == 5) {
        mp3.playAdvertisement(ADVERT_SOUND_SIREN);
    }
}

void Sound::playOn()
{
    playEndlessHum();
    mp3.playAdvertisement(ADVERT_SOUND_ON);
}
void Sound::playOff(bool story)
{
    if (!story) {
        silence();
    }
    mp3.playAdvertisement(ADVERT_SOUND_OFF);
}

void Sound::playBatteryLow()
{
    silence();
    mp3.playAdvertisement(ADVERT_SOUND_BATTERY_LOW);
}

void Sound::story(bool previous)
{
    mp3.stop();
    if (previous) {
        --m_story_index;
        if (m_story_index <= 0) {
            m_story_index = m_folder_2_track_count;
        }
    } else {
        ++m_story_index;
        if (m_story_index > m_folder_2_track_count) {
            m_story_index = 1;
        }
    }
    Serial.printf("Story Index: %i\n", m_story_index);
    mp3.playFolderTrack16(2, m_story_index); // sd:/02/0001*.mp3
}

/////////////////////////////////////////////
/////////////////////////////////////////////

#if 0
SD Card structure:

01/0001*.mp3 long playing humming sound
01/0002*.mp3 long playing silence sound

02/0001*.mp3 ..
02/9999*.mp3 story files

advert/0001*.mp3 on sound
advert/0002*.mp3 off sound
advert/0003*.mp3 off-on change sound
advert/0004*.mp3 rainbow sound
advert/0005*.mp3 siren sound
advert/0006*.mp3 battery low
advert/0007*.mp3 .. 9999*.mp3  switch through sounds

#endif

/////////////////////////////////////////////
/////////////////////////////////////////////

void Mp3Notify::OnError(uint16_t errorCode)
{
    // see DfMp3_Error for code meaning
    Serial.print("Com Error ");
    Serial.println(errorCode);
}
void Mp3Notify::OnPlayFinished(uint16_t track)
{
    Serial.print("Play finished for #");
    Serial.println(track);
}
void Mp3Notify::OnCardOnline(uint16_t code)
{
    Serial.println("Card online ");
}
void Mp3Notify::OnUsbOnline(uint16_t code)
{
    Serial.println("USB Disk online ");
}
void Mp3Notify::OnCardInserted(uint16_t code)
{
    Serial.println("Card inserted ");
}
void Mp3Notify::OnUsbInserted(uint16_t code)
{
    Serial.println("USB Disk inserted ");
}
void Mp3Notify::OnCardRemoved(uint16_t code)
{
    Serial.println("Card removed ");
}
void Mp3Notify::OnUsbRemoved(uint16_t code)
{
    Serial.println("USB Disk removed ");
}

} // namespace lightsaber
