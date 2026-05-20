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

// จำนวน frame ผิดติดต่อกันที่ยอมรับได้
#define IBUS_ERROR_THRESHOLD 8

// ช่วงค่าที่ถือว่า valid ก่อน constrain
#define IBUS_VALID_MIN 900
#define IBUS_VALID_MAX 2100

// -----------------------------------------------------
// Internal Objects
// -----------------------------------------------------

// Object ของไลบรารี IBusBM
static IBusBM ibus;

// ค่า channel ล่าสุด
// ค่าเริ่มต้น = neutral 1500
static int channels[10] = {
  1500, 1500, 1500, 1500, 1500,
  1500, 1500, 1500, 1500, 1500
};

// เวลา valid frame ล่าสุด
static unsigned long lastUpdateMs = 0;

// ตัวนับ frame error
static uint8_t errorCount = 0;

// -----------------------------------------------------
// Initialize IBUS
// -----------------------------------------------------
void IBusManager::begin() {

  // เปิด Serial1 สำหรับ FlySky IBUS
  Serial1.begin(115200);

  // เริ่มต้น IBusBM
  ibus.begin(Serial1);

  // reset channel ทุกช่องเป็นกลาง
  for (int i = 0; i < 10; i++) {
    channels[i] = 1500;
  }

  // reset timer
  lastUpdateMs = 0;

  // reset error counter
  errorCount = 0;
}

// -----------------------------------------------------
// Update IBUS data (non-blocking)
// -----------------------------------------------------
void IBusManager::update() {

  // parse serial data แบบ non-blocking
  ibus.loop();

  // อ่านทุก channel
  for (int i = 0; i < 10; i++) {

    // อ่านค่าช่องจาก IBusBM
    int v = ibus.readChannel(i);

    // -------------------------------------------------
    // Validate frame
    // -------------------------------------------------

    // ถ้าค่าหลุดช่วงผิดปกติ
    if (v < IBUS_VALID_MIN || v > IBUS_VALID_MAX) {

      // เพิ่มตัวนับ error
      if (errorCount < 255) {
        errorCount++;
      }

      // ถ้ายังไม่เกิน threshold
      // ให้อภัยชั่วคราว
      if (errorCount < IBUS_ERROR_THRESHOLD) {
        return;
      }

      // เกิน threshold
      // ถือว่า frame เสียจริง
      return;
    }

    // จำกัดช่วงจริงที่ใช้งาน
    v = constrain(v, 1000, 2000);

    // เก็บค่าล่าสุด
    channels[i] = v;
  }

  // -------------------------------------------------
  // Frame valid
  // -------------------------------------------------

  // reset error counter
  errorCount = 0;

  // อัปเดต heartbeat ทุกครั้งที่ได้ valid frame
  // แม้ค่าสติ๊กจะไม่เปลี่ยน
  lastUpdateMs = millis();
}

// -----------------------------------------------------
// Read channel value
// -----------------------------------------------------
int IBusManager::ch(int index) {

  // ป้องกัน index เกิน
  if (index < 0 || index > 9) {
    return 1500;
  }

  return channels[index];
}

// -----------------------------------------------------
// IBUS Alive Check
// -----------------------------------------------------
bool IBusManager::isAlive() {

  // ยังไม่เคย receive valid frame
  if (lastUpdateMs == 0) {
    return false;
  }

  // ถ้าเกิน 500ms ไม่มี valid frame
  // ถือว่าสัญญาณหาย
  return (millis() - lastUpdateMs) < 500;
}

