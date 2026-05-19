// =====================================================
// EngineManager.h
// Engine Control Interface
//
// หน้าที่:
// - ควบคุมเครื่องยนต์ (คันเร่ง, ignition, starter)
// - ทำงานร่วมกับ SystemState (ACTIVE / DISARMED / FAILSAFE)
// - เป็น state machine ภายในตัวเอง
//
// หมายเหตุสำคัญ:
// - ไฟล์นี้เป็น interface เท่านั้น
// =====================================================

#pragma once

#include <stdint.h>

// -----------------------------------------------------
// Engine Manager Class
// -----------------------------------------------------
// การเรียกใช้งาน (โดย main loop):
//   - begin()     : เรียกครั้งเดียวใน setup()
//   - update()    : เรียกเฉพาะ STATE_ACTIVE
//   - disarmed()  : เรียกเมื่อ STATE_DISARMED
//   - failsafe()  : เรียกเมื่อ STATE_FAILSAFE
// -----------------------------------------------------
class EngineManager {
public:

  // เริ่มต้นระบบเครื่องยนต์
  // - ตั้งค่า pin
  // - reset state ภายใน
  static void begin();

  // อัปเดต state machine เครื่องยนต์
  // - ทำงานเฉพาะเมื่อระบบอยู่ใน STATE_ACTIVE
  // - non-blocking เท่านั้น
  static void update();

  // ระบบถูก DISARM
  // - ต้องดับเครื่อง
  // - throttle = 0
  // - ignition / starter OFF
  static void disarmed();

  // FAILSAFE
  // - ตัดทุกอย่างทันที
  // - ต้องปลอดภัยสูงสุด
  static void failsafe();
};
