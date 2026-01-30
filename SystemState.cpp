// =====================================================
// SystemState.cpp
// Central System State Decision Logic
//
// หน้าที่:
// - ตัดสิน state กลางของระบบแบบ deterministic
// - FAILSAFE มี priority สูงสุด และเป็น latch
// - ไม่สร้าง side-effect (ไม่สั่ง hardware)
//
// หมายเหตุ:
// - Logic ความปลอดภัยต้องมาจาก SafetyManager เท่านั้น
// =====================================================

#include <Arduino.h>
#include "SystemState.h"
#include "SafetyManager.h"

// -----------------------------------------------------
// Internal State Storage
// -----------------------------------------------------
static SystemState_t currentState  = STATE_DISARMED;
static SystemState_t previousState = STATE_DISARMED;

// -----------------------------------------------------
// Set state explicitly (ใช้เฉพาะ init / override)
// -----------------------------------------------------
void SystemState::set(SystemState_t s) {
  previousState = currentState;
  currentState  = s;
}

// -----------------------------------------------------
// Get current state
// -----------------------------------------------------
SystemState_t SystemState::get() {
  return currentState;
}

// -----------------------------------------------------
// Update state based on safety conditions
// -----------------------------------------------------
void SystemState::update() {

  // ===================================================
  // FAILSAFE : highest priority (LATCH)
  // ===================================================
  if (SafetyManager::isFailsafe()) {

    // เข้า FAILSAFE ครั้งแรก
    if (currentState != STATE_FAILSAFE) {
      previousState = currentState;
      currentState  = STATE_FAILSAFE;
    }

    // ล็อก state ไว้ ไม่ให้ไหลต่อ
    return;
  }

  // ===================================================
  // FAILSAFE recovery rule
  // ===================================================
  // ออกจาก FAILSAFE ได้ "ทางเดียว":
  // 1) Failsafe หายแล้ว
  // 2) ต้อง DISARM ก่อนเสมอ
  if (currentState == STATE_FAILSAFE) {

    if (!SafetyManager::isArmed()) {
      previousState = currentState;
      currentState  = STATE_DISARMED;
    }

    return; // ห้ามข้าม state
  }

  // ===================================================
  // Normal operation
  // ===================================================
  if (SafetyManager::isArmed()) {

    // อนุญาตให้เข้า ACTIVE เฉพาะจาก DISARMED
    if (currentState == STATE_DISARMED) {
      previousState = currentState;
      currentState  = STATE_ACTIVE;
    }

  } else {

    // ไม่ ARM → ต้องกลับ DISARMED เสมอ
    if (currentState != STATE_DISARMED) {
      previousState = currentState;
      currentState  = STATE_DISARMED;
    }
  }
}
