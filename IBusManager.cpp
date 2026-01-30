// =====================================================
// IBusManager.cpp
// FlySky IBUS Input Manager (Implementation)
//
// Responsibilities:
// - อ่าน IBUS frame จาก Serial (non-blocking)
// - เก็บค่าช่องสัญญาณล่าสุด CH1–CH10
// - บอกสถานะว่า IBUS ยังมีชีวิตอยู่หรือไม่
//
// NOTE:
// - ไม่ทำ FAILSAFE ที่นี่
// - FAILSAFE decision อยู่ที่ SafetyManager
// =====================================================

#include <Arduino.h>
#include <IBusBM.h>
#include "IBusManager.h"

// -----------------------------------------------------
// Configuration
// -----------------------------------------------------
#define IBUS_ERROR_THRESHOLD 8  // จำนวน frame ผิดติดต่อกันที่ยอมรับได้
#define IBUS_VALID_MIN 900
#define IBUS_VALID_MAX 2100

// -----------------------------------------------------
// Internal Objects
// -----------------------------------------------------
static IBusBM ibus;

// ค่าเริ่มต้น = neutral (1500)
static int channels[10] = {
  1500, 1500, 1500, 1500, 1500,
  1500, 1500, 1500, 1500, 1500
};

// เวลา frame ล่าสุดที่ "เชื่อถือได้"
static unsigned long lastUpdateMs = 0;

// ตัวนับ error frame
static uint8_t errorCount = 0;

// -----------------------------------------------------
// Initialize IBUS
// -----------------------------------------------------
void IBusManager::begin() {

  Serial1.begin(115200);
  ibus.begin(Serial1);

  for (int i = 0; i < 10; i++) {
    channels[i] = 1500;
  }

  lastUpdateMs = 0;
  errorCount = 0;
}

// -----------------------------------------------------
// Update IBUS data (non-blocking)
// -----------------------------------------------------
void IBusManager::update() {

  ibus.loop();

  bool frameChanged = false;

  for (int i = 0; i < 10; i++) {

    int v = ibus.readChannel(i);

    // ตรวจ frame ผิดปกติ
    if (v < IBUS_VALID_MIN || v > IBUS_VALID_MAX) {

      // นับ error แทนการตัดทันที
      if (errorCount < 255) {
        errorCount++;
      }

      // ถ้ายังไม่ถึง threshold → ให้อภัย
      if (errorCount < IBUS_ERROR_THRESHOLD) {
        return;
      }

      // เกิน threshold → ถือว่า IBUS พังจริง
      return;
    }

    v = constrain(v, 1000, 2000);

    if (v != channels[i]) {
      channels[i] = v;
      frameChanged = true;
    }
  }

  // frame ถูกต้อง → reset error counter
  errorCount = 0;

  // อัปเดตเวลาเฉพาะเมื่อมีข้อมูลใหม่จริง
  if (frameChanged) {
    lastUpdateMs = millis();
  }
}

// -----------------------------------------------------
// Read channel value
// -----------------------------------------------------
int IBusManager::ch(int index) {

  if (index < 0 || index > 9) {
    return 1500;
  }

  return channels[index];
}

// -----------------------------------------------------
// IBUS Alive Check
// -----------------------------------------------------
bool IBusManager::isAlive() {

  if (lastUpdateMs == 0) {
    return false;
  }

  return (millis() - lastUpdateMs) < 500;
}
