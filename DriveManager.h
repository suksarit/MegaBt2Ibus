// =====================================================
// DriveManager.h
// TN Mower Drive Control Interface
//
// หน้าที่:
// - ควบคุมการขับเคลื่อนซ้าย / ขวา
// - ควบคุม BTS7960 ทั้ง 4 ตัว
// - ทำ differential drive
// - ทำ soft start / soft stop
// - ตอบสนองต่อ SystemState
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - pin / tuning / channel mapping
//   ถูกกำหนดใน Config.h
// =====================================================

#pragma once

#include <stdint.h>
#include "Config.h"

// =====================================================
// Drive Manager Class
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
// stop()
//   - เรียกเมื่อ STATE_DISARMED
//   - soft stop
//
// failsafe()
//   - เรียกเมื่อ STATE_FAILSAFE
//   - emergency stop
// =====================================================
class DriveManager {
public:

  // ---------------------------------------------------
  // Initialize Drive System
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - ตั้งค่า BTS7960
  // - enable driver
  // - reset internal state
  // - stop motor ทั้งหมด
  //
  // IMPORTANT:
  // - ต้องเรียกก่อนใช้งานเสมอ
  // ---------------------------------------------------
  static void begin();

  // ---------------------------------------------------
  // Update Drive System
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - อ่าน steering / throttle จาก IBusManager
  // - ทำ differential mixing
  // - ทำ deadzone
  // - ทำ slew limiting
  // - ส่ง PWM ไป driver
  //
  // IMPORTANT:
  // - non-blocking only
  // - ใช้เฉพาะ STATE_ACTIVE
  // ---------------------------------------------------
  static void update();

  // ---------------------------------------------------
  // Controlled Stop
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - ลด PWM ลงแบบนุ่มนวล
  // - ไม่ตัดทันที
  //
  // ใช้เมื่อ:
  // - STATE_DISARMED
  // ---------------------------------------------------
  static void stop();

  // ---------------------------------------------------
  // FAILSAFE STOP
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - หยุดทันที
  // - reset output state
  // - disable BTS7960
  //
  // IMPORTANT:
  // - safety priority สูงสุด
  // ---------------------------------------------------
  static void failsafe();
};

