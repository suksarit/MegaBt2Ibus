// =====================================================
// DriveManager.h
// Drive / Motion Control Interface (Tank Drive)
//
// Responsibilities:
// - ควบคุมการขับเคลื่อนรถ (ซ้าย / ขวา)
// - ทำงานร่วมกับ BTS7960
// - รองรับ soft start / soft stop
// - ตอบสนองต่อ SystemState (ACTIVE / DISARMED / FAILSAFE)
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - Logic การคำนวณความเร็วต้องอยู่ใน DriveManager.cpp
// =====================================================

#pragma once

#include <stdint.h>

// -----------------------------------------------------
// Drive Manager Class
// -----------------------------------------------------
// การเรียกใช้งานจาก main loop:
//   - begin()    : เรียกครั้งเดียวใน setup()
//   - update()   : เรียกเฉพาะ STATE_ACTIVE
//   - stop()     : เรียกเมื่อ STATE_DISARMED
//   - failsafe() : เรียกเมื่อ STATE_FAILSAFE
// -----------------------------------------------------
class DriveManager {
public:

  // เริ่มต้นระบบขับเคลื่อน
  // - ตั้งค่า pin
  // - enable driver
  // - reset internal state
  static void begin();

  // อัปเดตการขับเคลื่อน
  // - non-blocking เท่านั้น
  // - ใช้ข้อมูลจาก IBusManager
  static void update();

  // หยุดการเคลื่อนที่แบบควบคุม
  // - ลดความเร็วลงอย่างนุ่มนวล (soft stop)
  static void stop();

  // FAILSAFE
  // - หยุดทันที
  // - ปลอดภัยสูงสุด
  static void failsafe();
};
