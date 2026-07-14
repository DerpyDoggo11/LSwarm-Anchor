#include "uwb.h"
#include "pins.h"
#include <SPI.h>

static SPIClass      dwSpi(FSPI);
static uint32_t      s_devid = 0;
static bool          s_ok    = false;
static volatile bool s_irq   = false;

static const SPISettings kSlow(2000000,  MSBFIRST, SPI_MODE0);
static const SPISettings kFast(16000000, MSBFIRST, SPI_MODE0);

static void IRAM_ATTR dwIsr() { s_irq = true; }

static uint16_t makeHeader(uint8_t file, uint16_t offset, bool write) {
    uint16_t h = (uint16_t)((file & 0x1F) << 9) | (uint16_t)((offset & 0x7F) << 2);
    if (write) h |= 0x8000;
    return h;
}

static void xfer(uint8_t file, uint16_t offset, uint8_t* buf, size_t len, bool write) {
    uint16_t hdr = makeHeader(file, offset, write);

    dwSpi.beginTransaction(s_ok ? kFast : kSlow);
    digitalWrite(PIN_DW_CS, LOW);
    dwSpi.transfer((uint8_t)(hdr >> 8));
    dwSpi.transfer((uint8_t)(hdr & 0xFF));
    for (size_t i = 0; i < len; i++) {
        uint8_t out = write ? buf[i] : 0x00;
        uint8_t in  = dwSpi.transfer(out);
        if (!write) buf[i] = in;
    }
    digitalWrite(PIN_DW_CS, HIGH);
    dwSpi.endTransaction();
}

void Uwb::readRegs(uint8_t file, uint16_t offset, uint8_t* buf, size_t len) {
    xfer(file, offset, buf, len, false);
}

void Uwb::writeRegs(uint8_t file, uint16_t offset, const uint8_t* buf,
                    size_t len) {
    xfer(file, offset, (uint8_t*)buf, len, true);
}

uint32_t Uwb::readReg32(uint8_t file, uint16_t offset) {
    uint8_t b[4] = {0, 0, 0, 0};
    readRegs(file, offset, b, 4);
    return (uint32_t)b[3] << 24 | (uint32_t)b[2] << 16 | (uint32_t)b[1] << 8  | (uint32_t)b[0];
}

void Uwb::hardReset() {
    pinMode(PIN_DW_RST, OUTPUT);
    digitalWrite(PIN_DW_RST, LOW);
    delay(2);
    pinMode(PIN_DW_RST, INPUT);
    delay(5);
}

void Uwb::wake() {
    digitalWrite(PIN_DW_WAKEUP, HIGH);
    delayMicroseconds(600); 
    digitalWrite(PIN_DW_WAKEUP, LOW);
    delay(2);
}

void Uwb::sleep() {
    digitalWrite(PIN_DW_WAKEUP, LOW);
}

bool Uwb::begin() {
    s_ok = false;

    pinMode(PIN_DW_CS, OUTPUT);
    digitalWrite(PIN_DW_CS, HIGH);

    pinMode(PIN_DW_WAKEUP, OUTPUT);
    digitalWrite(PIN_DW_WAKEUP, LOW);

    pinMode(PIN_DW_IRQ, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_DW_IRQ), dwIsr, RISING);

    dwSpi.begin(PIN_DW_CLK, PIN_DW_MISO, PIN_DW_MOSI, PIN_DW_CS);

    hardReset();
    delay(5);

    s_devid = readReg32(0x00, 0x00);

    s_ok = ((s_devid & 0xFFFF0000u) == 0xDECA0000u);
    return s_ok;
}

uint32_t Uwb::deviceId() { return s_devid; }
bool Uwb::ok() { return s_ok; }

bool Uwb::irqPending() {
    if (!s_irq) return false;
    s_irq = false;
    return true;
}