// =====================================================
// RelayManager.cpp
// Auxiliary Relay Control (Implementation)
//
// Controls:
// - Light relay (CH8)
//
// Safety Policy:
// - safe()     : ปิดรีเลย์แบบปกติ (DISARM)
// - failsafe() : ปิดรีเลย์แบบบังคับ (FAILSAFE)
//
// NOTE:
// - ไม่ตัดสิน ARM / FAILSAFE ที่นี่
// - รับคำสั่งจาก IBusManager เท่านั้น
// =====================================================

#include <Arduino.h>
#include "RelayManager.h"
#include "IBusManager.h"

// -----------------------------------------------------
// Hardware Configuration
// -----------------------------------------------------
#define RELAY_LIGHT_PIN 24

// -----------------------------------------------------
// Internal State
// -----------------------------------------------------
static bool lightState = false;

// -----------------------------------------------------
// Apply Relay Output (STATE_ACTIVE / SAFE only)
// -----------------------------------------------------
static void applyLight(bool on) {

  // เขียนเฉพาะเมื่อ state เปลี่ยน (ลด relay chatter)
  if (lightState != on) {
    lightState = on;
    digitalWrite(RELAY_LIGHT_PIN, on ? HIGH : LOW);
  }
}

// -----------------------------------------------------
// Initialize Relay Manager
// -----------------------------------------------------
void RelayManager::begin() {

  pinMode(RELAY_LIGHT_PIN, OUTPUT);   // ตั้งโหมดก่อน
  digitalWrite(RELAY_LIGHT_PIN, LOW); // ปิดรีเลย์เสมอ

  lightState = false;
}

// -----------------------------------------------------
// Update Relay State (STATE_ACTIVE only)
// -----------------------------------------------------
void RelayManager::update() {

  // CH8 > 1500 = เปิดไฟ
  bool cmdLight = (IBusManager::ch(7) > 1500);

  applyLight(cmdLight);
}

// -----------------------------------------------------
// Safe State (DISARM)
// -----------------------------------------------------
void RelayManager::safe() {

  applyLight(false);
}

// -----------------------------------------------------
// FAILSAFE State (Highest Priority)
// -----------------------------------------------------
void RelayManager::failsafe() {

  // FAILSAFE = force state ให้ปลอดภัยเสมอ
  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  digitalWrite(RELAY_LIGHT_PIN, LOW);

  lightState = false;
}
