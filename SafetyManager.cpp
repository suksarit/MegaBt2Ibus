// =====================================================
// SafetyManager.cpp
// - ตรวจสอบความมีชีวิตของสัญญาณ IBUS
// - ตัดสิน FAILSAFE ระดับระบบ (LATCH)
// - ตรวจสอบ ARM command
// - ทำหน้าที่ให้ข้อมูลความปลอดภัยแก่ SystemState เท่านั้น
// =====================================================

#include <Arduino.h>
#include "SafetyManager.h"
#include "IBusManager.h"

// -----------------------------------------------------
// Configuration
// -----------------------------------------------------
#define FAILSAFE_DEAD_CONFIRM 150UL   // ms ต้อง dead ต่อเนื่องก่อนเข้า FAILSAFE
#define FAILSAFE_CLEAR_CONFIRM 200UL  // ms ต้อง alive ต่อเนื่องก่อน clear

// -----------------------------------------------------
// Internal State
// -----------------------------------------------------
static bool failsafe = true;  // latch
static bool armedCached = false;
static unsigned long ibusAliveSince = 0;
static unsigned long ibusDeadSince = 0;

// -----------------------------------------------------
// Initialize Safety Manager
// -----------------------------------------------------
void SafetyManager::begin() {
  failsafe = true;
  armedCached = false;
  ibusAliveSince = 0;
  ibusDeadSince = 0;
}

// -----------------------------------------------------
// Update Safety Status (non-blocking)
// -----------------------------------------------------
void SafetyManager::update() {

  bool ibusAlive = IBusManager::isAlive();
  bool armCmdRaw = (IBusManager::ch(4) > 1500);  // CH5 = ARM

  // ===================================================
  // IBUS DEAD HANDLING (with confirm time)
  // ===================================================
  if (!ibusAlive) {

    if (ibusDeadSince == 0) {
      ibusDeadSince = millis();
    }

    // dead ต่อเนื่องเกินเวลาที่กำหนด → FAILSAFE
    if ((millis() - ibusDeadSince) >= FAILSAFE_DEAD_CONFIRM) {
      failsafe = true;
      armedCached = false;
      ibusAliveSince = 0;
    }

    return;
  }

  // ===================================================
  // IBUS ALIVE
  // ===================================================
  ibusDeadSince = 0;

  if (ibusAliveSince == 0) {
    ibusAliveSince = millis();
  }

  // ===================================================
  // FAILSAFE latch clear rule
  // ===================================================
  if (failsafe) {

    // ต้อง alive ต่อเนื่อง + ARM OFF
    if (!armCmdRaw && (millis() - ibusAliveSince >= FAILSAFE_CLEAR_CONFIRM)) {
      failsafe = false;
    }

    return;
  }

  // ===================================================
  // Normal operation
  // ===================================================
  armedCached = armCmdRaw;
}

// -----------------------------------------------------
// ARM Status
// -----------------------------------------------------
bool SafetyManager::isArmed() {

  if (failsafe) {
    return false;
  }

  return armedCached;
}

// -----------------------------------------------------
// FAILSAFE Status
// -----------------------------------------------------
bool SafetyManager::isFailsafe() {
  return failsafe;
}
