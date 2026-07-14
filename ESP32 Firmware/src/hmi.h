#pragma once
#include <Arduino.h>
#include "pins.h"

namespace Hmi {

#if STATUS_LED_ACTIVE_LOW
static constexpr uint8_t LED_ON  = LOW;
static constexpr uint8_t LED_OFF = HIGH;
#else
static constexpr uint8_t LED_ON  = HIGH;
static constexpr uint8_t LED_OFF = LOW;
#endif

static constexpr uint8_t BUZZ_CH   = 0;
static constexpr uint32_t BUZZ_RES = 2700;

inline void begin() {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LED_OFF);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PIN_BUZZER, BUZZ_RES, 10);
    ledcWrite(PIN_BUZZER, 0);
#else
    ledcSetup(BUZZ_CH, BUZZ_RES, 10);
    ledcAttachPin(PIN_BUZZER, BUZZ_CH);
    ledcWrite(BUZZ_CH, 0);
#endif
}

inline void led(bool on) {
    digitalWrite(PIN_STATUS_LED, on ? LED_ON : LED_OFF);
}

inline void blink(uint16_t period_ms) {
    if (period_ms == 0) { led(true); return; }
    led(((millis() / period_ms) & 1) == 0);
}

inline void buzzOn(uint16_t freq = BUZZ_RES) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(PIN_BUZZER, freq);
#else
    ledcWriteTone(BUZZ_CH, freq);
#endif
}

inline void buzzOff() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(PIN_BUZZER, 0);
#else
    ledcWrite(BUZZ_CH, 0);
#endif
}

inline void beep(uint16_t ms = 80, uint16_t freq = BUZZ_RES) {
    buzzOn(freq);
    delay(ms);
    buzzOff();
}

}

namespace Power {

inline void begin() {
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_VMON_ADC, ADC_11db);
}

inline float batteryVolts() {
    uint32_t acc = 0;
    for (uint8_t i = 0; i < 16; i++) acc += analogReadMilliVolts(PIN_VMON_ADC);
    return (acc / 16.0f) / 1000.0f * VBAT_DIVIDER;
}

inline uint8_t batteryPercent() {
    float v = batteryVolts();
    float pct = (v - 3.30f) / (4.15f - 3.30f) * 100.0f;
    return (uint8_t)constrain(pct, 0.0f, 100.0f);
}

}