
#include <Arduino.h>
#include "pins.h"
#include "uwb.h"
#include "hmi.h"

enum class Role : uint8_t { Anchor, Tag };
static Role g_role   = Role::Anchor;
static uint16_t g_addr   = 0x0001;
static float g_vbat   = 0.0f;

static void printStatus() {
    Serial.printf("role : %s\n", g_role == Role::Anchor ? "anchor" : "tag");
    Serial.printf("addr : 0x%04X\n", g_addr);
    Serial.printf("uwb : %s (devid 0x%08X)\n", Uwb::ok() ? "ok" : "NOT DETECTED", (unsigned int)Uwb::deviceId());
    Serial.printf("battery: %.2f V (%u%%)\n", g_vbat, Power::batteryPercent());
    Serial.printf("heap : %u bytes free\n", (unsigned)ESP.getFreeHeap());
}

static void handleCommand(char* cmd) {
    if (!strcmp(cmd, "help")) {
        Serial.println("help | id | stat | reset | batt | beep |");
        Serial.println("led on|off | role anchor|tag | mac <n>");
    } else if (!strcmp(cmd, "id")) {
        Serial.printf("devid 0x%08X\n", (unsigned int)Uwb::deviceId());
    } else if (!strcmp(cmd, "stat")) {
        printStatus();
    } else if (!strcmp(cmd, "reset")) {
        bool ok = Uwb::begin();
        Serial.printf("uwb reset -> %s\n", ok ? "ok" : "FAIL");
    } else if (!strcmp(cmd, "batt")) {
        Serial.printf("%.3f V  (%u%%)\n", Power::batteryVolts(),
                      Power::batteryPercent());
    } else if (!strcmp(cmd, "beep")) {
        Hmi::beep(120);
    } else if (!strcmp(cmd, "led on"))  { 
        Hmi::led(true); 
    } else if (!strcmp(cmd, "led off")) { 
        Hmi::led(false); 
    } else if (!strcmp(cmd, "role anchor")) {
        g_role = Role::Anchor; Serial.println("role = anchor");
    } else if (!strcmp(cmd, "role tag")) {
        g_role = Role::Tag;    Serial.println("role = tag");
    } else if (!strncmp(cmd, "mac ", 4)) {
        g_addr = (uint16_t)strtol(cmd + 4, nullptr, 0);
        Serial.printf("addr = 0x%04X\n", g_addr);
    } else if (cmd[0]) {
        Serial.println("? try 'help'");
    }
}

void setup() {
    Serial.begin(115200);

    Hmi::begin();
    Power::begin();

    while (!Serial && millis() - t0 < 2000) { delay(10); }

    bool ok = Uwb::begin();
    g_vbat  = Power::batteryVolts();

    printStatus();

    Hmi::beep(ok ? 60 : 300); 
}

void loop() {
    static char    line[64];
    static uint8_t n = 0;
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (n) { line[n] = '\0'; handleCommand(line); n = 0; }
        } else if (n < sizeof(line) - 1) {
            line[n++] = c;
        }
    }


    static uint32_t tHk = 0;
    if (millis() - tHk >= 500) {
        tHk = millis();
        g_vbat = Power::batteryVolts();

        if (g_vbat < VBAT_WARN)   Hmi::blink(125);
        else if (!Uwb::ok())      Hmi::blink(500);
        else                      Hmi::led(true);

        static bool warned = false;
        if (g_vbat < VBAT_CRITICAL && !warned) {
            warned = true;
            Serial.printf("!! battery critical: %.2f V\n", g_vbat);
            Hmi::beep(400);
        } else if (g_vbat > VBAT_WARN) {
            warned = false;
        }
    }
}