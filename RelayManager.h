// =====================================================
// RelayManager.h
// Auxiliary Relay Control Interface
// - ควบคุมรีเลย์อุปกรณ์เสริม (เช่น ไฟส่องสว่าง)
// - ทำงานตาม SystemState (ACTIVE / DISARMED / FAILSAFE)
// - แยกจาก EngineManager อย่างชัดเจน
// - ไฟล์นี้เป็น interface เท่านั้น
// =====================================================

#pragma once

#include <stdint.h>

// -----------------------------------------------------
// Relay Manager Class
// -----------------------------------------------------
// การเรียกใช้งานจาก main loop:
//   - begin()    : เรียกครั้งเดียวใน setup()
//   - update()   : เรียกเฉพาะ STATE_ACTIVE
//   - safe()     : เรียกเมื่อ STATE_DISARMED
//   - failsafe() : เรียกเมื่อ STATE_FAILSAFE
// -----------------------------------------------------
class RelayManager {
public:

  // เริ่มต้นระบบรีเลย์
  // - ตั้งค่า pin
  // - ปิดรีเลย์ทั้งหมดเป็นค่าเริ่มต้น
  static void begin();

  // อัปเดตสถานะรีเลย์ตามคำสั่ง (STATE_ACTIVE เท่านั้น)
  // - non-blocking
  static void update();

  // ปิดรีเลย์ทั้งหมดแบบปกติ (DISARM)
  static void safe();

  // ปิดรีเลย์ทั้งหมดแบบบังคับ (FAILSAFE)
  // - ต้องเป็นสถานะที่ปลอดภัยสูงสุด
  static void failsafe();
};
