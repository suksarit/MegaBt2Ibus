// =====================================================
// EngineManager.h
// TN Mower Engine Control Interface
//
// หน้าที่:
// - ควบคุมเครื่องยนต์
// - ควบคุม throttle servo
// - ควบคุม ignition relay
// - ควบคุม starter relay
// - จัดการ engine state machine
//
// ทำงานร่วมกับ:
// - SystemState
// - IBusManager
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - configuration ทั้งหมดอยู่ใน Config.h
// =====================================================

#pragma once

#include <stdint.h>

#include "Config.h"

// =====================================================
// Engine Manager Class
// =====================================================
//
// การเรียกใช้งาน:
//
// begin()
//   - เรียกครั้งเดียวใน setup()
//
// update()
//   - เรียกเฉพาะ STATE_ACTIVE
//
// disarmed()
//   - เรียกเมื่อ STATE_DISARMED
//
// failsafe()
//   - เรียกเมื่อ STATE_FAILSAFE
// =====================================================
class EngineManager {
public:

  // ---------------------------------------------------
  // Initialize Engine System
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - attach servo
  // - ตั้งค่า relay pin
  // - reset internal state
  // - shutdown output ทั้งหมด
  //
  // IMPORTANT:
  // - ต้องเรียกก่อนใช้งานเสมอ
  // ---------------------------------------------------
  static void begin();

  // ---------------------------------------------------
  // Update Engine State Machine
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - อ่านคำสั่งจาก IBusManager
  // - ควบคุม ignition
  // - ควบคุม starter
  // - ควบคุม throttle servo
  // - จัดการ engine state transition
  //
  // IMPORTANT:
  // - non-blocking only
  // - ใช้เฉพาะ STATE_ACTIVE
  // ---------------------------------------------------
  static void update();

  // ---------------------------------------------------
  // DISARMED State
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - throttle idle
  // - ignition OFF
  // - starter OFF
  // - reset engine state
  //
  // ใช้เมื่อ:
  // - STATE_DISARMED
  // ---------------------------------------------------
  static void disarmed();

  // ---------------------------------------------------
  // FAILSAFE State
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - emergency shutdown
  // - force safe output
  //
  // IMPORTANT:
  // - safety priority สูงสุด
  // ---------------------------------------------------
  static void failsafe();
};

