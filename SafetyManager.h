// =====================================================
// SafetyManager.h
// - ประเมินสถานะความปลอดภัยของระบบ
// - ตรวจสอบ ARM command
// - ตรวจสอบ FAILSAFE condition (เช่น IBUS lost)
// - ไฟล์นี้เป็น interface เท่านั้น
// =====================================================

#pragma once

#include <stdint.h>

// -----------------------------------------------------
// Safety Manager Class
// -----------------------------------------------------
// การเรียกใช้งานจาก main loop:
//   - begin()      : เรียกครั้งเดียวใน setup()
//   - update()     : เรียกทุก loop (non-blocking)
//   - isArmed()    : ใช้ตัดสินว่าอนุญาตให้ระบบ ACTIVE หรือไม่
//   - isFailsafe() : ใช้ตัดสินว่าต้องเข้าสู่ FAILSAFE หรือไม่
// -----------------------------------------------------
class SafetyManager {
public:

  // เริ่มต้นระบบความปลอดภัย
  // - reset internal state
  // - เริ่มต้นในสถานะปลอดภัย (FAILSAFE)
  static void begin();

  // อัปเดตสถานะความปลอดภัย
  // - non-blocking
  // - ต้องถูกเรียกทุก loop
  static void update();

  // ระบบถูก ARM หรือไม่
  // - ควรคืนค่า true เฉพาะเมื่อ "ปลอดภัยและอนุญาต"
  static bool isArmed();

  // ระบบอยู่ใน FAILSAFE หรือไม่
  // - true = ต้องหยุดทุกอย่างทันที
  static bool isFailsafe();
};
