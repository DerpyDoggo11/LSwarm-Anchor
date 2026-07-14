#pragma once
#include <Arduino.h>


static constexpr int PIN_DW_WAKEUP = 1;
static constexpr int PIN_DW_IRQ    = 3;
static constexpr int PIN_DW_MISO   = 4;
static constexpr int PIN_DW_MOSI   = 5;
static constexpr int PIN_DW_CLK    = 6;
static constexpr int PIN_DW_CS     = 7;
static constexpr int PIN_DW_RST    = 10;

// HMI
static constexpr int PIN_STATUS_LED = 2; 
static constexpr int PIN_BUZZER = 21;

#define STATUS_LED_ACTIVE_LOW 1

// Power
static constexpr int   PIN_VMON_ADC = 0;
static constexpr float VBAT_DIVIDER = 2.0f;

static constexpr float VBAT_FULL     = 4.10f;
static constexpr float VBAT_WARN     = 3.40f;
static constexpr float VBAT_CRITICAL = 3.10f;