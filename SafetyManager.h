// =====================================================
// SafetyManager.h
// TN Mower Safety Controller Interface
//
// Responsibilities:
// - ตรวจสอบ IBUS alive
// - ตรวจสอบ ARM command
// - จัดการ FAILSAFE latch
// - ให้ข้อมูล safety แก่ SystemState
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - ไม่ควบคุม hardware โดยตรง
// - ไม่สั่ง motor / engine / relay
// - Configuration อยู่ใน Config.h
// =====================================================

#pragma once

#include <stdint.h>

#include "Config.h"

// =====================================================
// Safety Manager Class
// =====================================================
//
// การเรียกใช้งาน:
//
// begin()
//   - เรียกครั้งเดียวใน setup()
//
// update()
//   - เรียกทุก loop
//
// isArmed()
//   - ใช้ตัดสิน ACTIVE state
//
// isFailsafe()
//   - ใช้ตัดสิน FAILSAFE state
// =====================================================
class SafetyManager {
public:

  // ---------------------------------------------------
  // Initialize Safety System
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - reset runtime state
  // - reset timers
  // - เริ่มต้นใน FAILSAFE
  //
  // IMPORTANT:
  // - safety default = SAFE
  // ---------------------------------------------------
  static void begin();

  // ---------------------------------------------------
  // Update Safety Status
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - ตรวจสอบ IBUS alive
  // - ตรวจสอบ ARM command
  // - จัดการ FAILSAFE latch
  // - update safety state
  //
  // IMPORTANT:
  // - non-blocking only
  // - ต้องถูกเรียกทุก loop
  // ---------------------------------------------------
  static void update();

  // ---------------------------------------------------
  // ARM Status
  // ---------------------------------------------------
  //
  // true:
  //   อนุญาตให้เข้า STATE_ACTIVE
  //
  // false:
  //   ไม่อนุญาต ACTIVE
  //
  // IMPORTANT:
  // - FAILSAFE จะ force false เสมอ
  // ---------------------------------------------------
  static bool isArmed();

  // ---------------------------------------------------
  // FAILSAFE Status
  // ---------------------------------------------------
  //
  // true:
  //   emergency stop required
  //
  // false:
  //   normal operation allowed
  //
  // IMPORTANT:
  // - FAILSAFE เป็น latch
  // ---------------------------------------------------
  static bool isFailsafe();
};

