#pragma once
#include <Arduino.h>

namespace Uwb {

bool begin(); 
void hardReset();
void wake();
void sleep();

uint32_t deviceId();
bool ok();

void readRegs(uint8_t file, uint16_t offset, uint8_t* buf, size_t len);
void writeRegs(uint8_t file, uint16_t offset, const uint8_t* buf, size_t len);
uint32_t readReg32(uint8_t file, uint16_t offset);

bool irqPending();

}