#include <Arduino.h>
#include <JeVe_EasyOTA.h>
#include <ESP8266WiFi.h>

#include "light.h"
#include "sound.h"
#include "secrets.h"
#include <push_button.h>

using lightsaber::Light;
using lightsaber::Sound;
using pb::PushButton;

PushButton button1(D5);
PushButton button2(D2);
PushButton button3(D7);
PushButton button4(D3);

Light light;
Sound sound;
Ticker tick;

EasyOTA OTA(hostname);

bool on(true);
bool story(false);

uint32_t lastADC;
bool otaRequested(false);

void setup()
{
    Serial.begin(115200);
    Serial.printf("Go! \n");

    light.begin();

    if (digitalRead(D5) == 0)
    {
        otaRequested = true;

        OTA.onMessage([](const String& message, int line) {
            Serial.println(message);
        });
        OTA.addAP(ssid, password);
        Serial.printf("OTA active!\n");

        light.beginSequence(Light::Sequence::OTA);
    } else {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.forceSleepBegin();
    }

    pinMode(D8, OUTPUT);
    digitalWrite(D8, LOW);

    lastADC = millis();
}

#if 0
button | short            | long               | double               | triple
-----------------------------------------------------------------------------------------------
 -1-   | change color     |                    |                      | battery low simulation
       | press on startup:|                    |                      |
       |   start in OTA   |                    |                      |
       |   mode           |                    |                      |
       |                  |                    |                      |
 -2-   | retract/extend   | retract + off      |                      |
       |                  |                    |                      |
 -3-   | advert/story     | advert/story       | voume up             |
       |   next           |                    |                      |
 -4-   | advert/story     | sound off/on       | volume down          |
       |   previous       |                    |                      |

#endif

bool lowBatterySignaled(false);
bool initialized(false);

void loop()
{
    if (otaRequested) {
        OTA.loop();
        light.loop();
        return;
    }

    if (!initialized) {
        sound.begin();
        sound.playOn();
        uint32_t t(millis());
        while (millis() - t < 200) {
            sound.loop();
            delay(1);
        }
        light.beginSequence(Light::Sequence::On);
        initialized = true;
    }

    if (initialized) {
        light.loop();
        sound.loop();

        PushButton::Event e1(button1.getEvent());
        PushButton::Event e2(button2.getEvent());
        PushButton::Event e3(button3.getEvent());
        PushButton::Event e4(button4.getEvent());

        // Button 1
        if (e1 == PushButton::Event::SHORT_PRESS) {
            Serial.printf("button 1 - PushButton::Event::SHORT_PRESS\n");
            if (on) {
                Serial.printf("Change\n");
                uint8_t index(light.beginSequence(Light::Sequence::Change));
                sound.playChange(index);
            }
        } else if (e1 == PushButton::Event::TRIPLE_PRESS) {
            Serial.printf("button 1 - PushButton::Event::TRIPLE_PRESS\n");
            if (on) {
                Serial.printf("Battery Low Simulation\n");
                sound.playBatteryLow();
                light.beginSequence(Light::Sequence::BatteryLow);
            }
        }

        // Button 2
        if (e2 == PushButton::Event::SHORT_PRESS) {
            Serial.printf("button 2 - PushButton::Event::SHORT_PRESS\n");
            if (on) {
                Serial.printf("Retract\n");
                sound.playOff();
                light.beginSequence(Light::Sequence::Off);
            } else {
                Serial.printf("Extend\n");
                sound.playOn();
                light.beginSequence(Light::Sequence::On);
            }
            on = !on;
        } else if (e2 == PushButton::Event::LONG_PRESS) {
            Serial.printf("button 2 - PushButton::Event::LONG_PRESS\n");
            if (on) {
                Serial.printf("Retract + Switch Off\n");
                sound.playOff();
                light.beginSequence(Light::Sequence::Off);
                tick.once_ms(2000, []() {
                    digitalWrite(D8, HIGH);
                });
            }
        }

        // Button 3
        if (e3 == PushButton::Event::SHORT_PRESS) {
            Serial.printf("button 3 - PushButton::Event::SHORT_PRESS\n");
            if (story) {
                Serial.printf("Story Next\n");
                sound.story(false);
            } else if (on) {
                Serial.printf("Sound Next\n");
                sound.advert(false);
            }
        } else if (e3 == PushButton::Event::LONG_PRESS
                   || e3 == PushButton::Event::LONG_HOLD) {
            Serial.printf("button 3 - PushButton::Event::LONG_PRESS\n");
            if (story) {
                Serial.printf("Advert Mode\n");
                story = false;
                sound.playEndlessHum();
            } else {
                Serial.printf("Story Mode\n");
                story = true;
                sound.story(false);
            }
        } else if (e3 == PushButton::Event::DOUBLE_PRESS) {
            Serial.printf("button 3 - PushButton::Event::DOUBLE_PRESS\n");
            Serial.printf("Volume Up\n");
            sound.volumeUp();
        }

        // Button 4
        if (e4 == PushButton::Event::SHORT_PRESS) {
            Serial.printf("button 4 - PushButton::Event::SHORT_PRESS\n");
            if (story) {
                Serial.printf("Story Previous\n");
                sound.story(true);
            } else if (on) {
                Serial.printf("Sound Previous\n");
                sound.advert(true);
            }
        } else if (e4 == PushButton::Event::LONG_PRESS) {
            Serial.printf("button 4 - PushButton::Event::LONG_PRESS\n");
            Serial.printf("Toggle Pause, Resume\n");
            sound.pauseResume();
        } else if (e4 == PushButton::Event::DOUBLE_PRESS) {
            Serial.printf("button 4 - PushButton::Event::DOUBLE_PRESS\n");
            Serial.printf("Volume Down\n");
            sound.volumeDown();
        }

        if (millis() - lastADC > 5000) {
            int vBat(analogRead(A0));

            Serial.printf("Battery Voltage: %iV\n", vBat);
            lastADC = millis();

            if (!lowBatterySignaled
                && vBat < 670) {
                lowBatterySignaled = true;
                sound.playBatteryLow();
                light.beginSequence(Light::Sequence::BatteryLow);

                tick.once_ms(15000, []() {
                    digitalWrite(D8, HIGH);
                });
            }
        }
    }
}
