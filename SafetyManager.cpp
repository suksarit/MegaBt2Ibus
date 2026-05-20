// =====================================================
// SafetyManager.cpp
// TN Mower Safety Controller
//
// Responsibilities:
// - ตรวจสอบ IBUS alive
// - จัดการ FAILSAFE latch
// - ตรวจสอบ ARM command
// - ให้ข้อมูล safety แก่ SystemState
//
// IMPORTANT:
// - ไม่ควบคุม hardware โดยตรง
// - ไม่สั่ง motor / relay / engine
// - Configuration อยู่ใน Config.h
// =====================================================

#include <Arduino.h>

#include "Config.h"
#include "SafetyManager.h"
#include "IBusManager.h"

// =====================================================
// Internal Runtime State
// =====================================================

// FAILSAFE latch
static bool failsafe = SYSTEM_BOOT_FAILSAFE;

// cached ARM state
static bool armedCached = false;

// เวลาเริ่ม detect alive
static unsigned long ibusAliveSince = 0;

// เวลาเริ่ม detect dead
static unsigned long ibusDeadSince = 0;

// =====================================================
// Initialize Safety Manager
// =====================================================
void SafetyManager::begin() {

  // ---------------------------------------------------
  // default safe state
  // ---------------------------------------------------
  failsafe =
    SYSTEM_BOOT_FAILSAFE;

  armedCached = false;

  // ---------------------------------------------------
  // reset timers
  // ---------------------------------------------------
  ibusAliveSince = 0;

  ibusDeadSince = 0;
}

// =====================================================
// Update Safety Status
// =====================================================
//
// IMPORTANT:
// - non-blocking only
// - ต้องเรียกทุก loop
// =====================================================
void SafetyManager::update() {

  // ---------------------------------------------------
  // Read IBUS status
  // ---------------------------------------------------
  bool ibusAlive =
    IBusManager::isAlive();

  // ---------------------------------------------------
  // Read ARM command
  // ---------------------------------------------------
  bool armCmdRaw =
    (
      IBusManager::ch(
        IBUS_CH_ARM
      )
      >
      RC_CENTER
    );

  // ===================================================
  // IBUS DEAD HANDLING
  // ===================================================
  //
  // dead ต่อเนื่องเกินเวลาที่กำหนด
  // → FAILSAFE
  // ===================================================
  if (!ibusAlive) {

    // -------------------------------------------------
    // start dead timer
    // -------------------------------------------------
    if (ibusDeadSince == 0) {

      ibusDeadSince =
        millis();
    }

    // -------------------------------------------------
    // dead timeout confirm
    // -------------------------------------------------
    if (
      (millis() - ibusDeadSince)
      >=
      FAILSAFE_DEAD_CONFIRM
    ) {

      failsafe = true;

      armedCached = false;

      // reset alive timer
      ibusAliveSince = 0;
    }

    return;
  }

  // ===================================================
  // IBUS ALIVE
  // ===================================================

  // ---------------------------------------------------
  // clear dead timer
  // ---------------------------------------------------
  ibusDeadSince = 0;

  // ---------------------------------------------------
  // start alive timer
  // ---------------------------------------------------
  if (ibusAliveSince == 0) {

    ibusAliveSince =
      millis();
  }

  // ===================================================
  // FAILSAFE LATCH CLEAR RULE
  // ===================================================
  //
  // ต้อง:
  // 1. alive ต่อเนื่อง
  // 2. ARM OFF
  // ===================================================
  if (failsafe) {

    if (
      !armCmdRaw
      &&
      (
        millis() - ibusAliveSince
      )
      >=
      FAILSAFE_CLEAR_CONFIRM
    ) {

      failsafe = false;
    }

    return;
  }

  // ===================================================
  // NORMAL OPERATION
  // ===================================================

  armedCached = armCmdRaw;
}

// =====================================================
// ARM Status
// =====================================================
//
// true:
//   ระบบอนุญาต ACTIVE
//
// false:
//   ไม่อนุญาต ACTIVE
// =====================================================
bool SafetyManager::isArmed() {

  // FAILSAFE = ห้าม ARM
  if (failsafe) {

    return false;
  }

  return armedCached;
}

// =====================================================
// FAILSAFE Status
// =====================================================
//
// true:
//   emergency stop required
//
// false:
//   normal operation allowed
// =====================================================
bool SafetyManager::isFailsafe() {

  return failsafe;
}

