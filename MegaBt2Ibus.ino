// =====================================================
// MegaBt2Ibus.ino ฺ ฺ ฺBy TN Mower
// =====================================================

#include <Arduino.h>     // ให้แน่ใจว่า core Arduino ถูกโหลด
#include <avr/wdt.h>

#include "SystemState.h"
#include "IBusManager.h"
#include "DriveManager.h"
#include "EngineManager.h"
#include "RelayManager.h"
#include "SafetyManager.h"

// =====================================================
// SETUP
// =====================================================
void setup() {

  // --- ปิด watchdog ก่อน (กันรีเซตตอนบูต) ---
  wdt_disable();

  // --- Initialize subsystems ---
  // ลำดับนี้ถูกต้องแล้ว ห้ามสลับ
  IBusManager::begin();     // ต้องมาก่อน เพื่อให้มี input
  SafetyManager::begin();   // safety ใช้ข้อมูลจาก IBUS
  DriveManager::begin();    // มอเตอร์
  EngineManager::begin();   // เครื่องยนต์
  RelayManager::begin();    // รีเลย์เสริม

  // --- Initial state ---
  // เริ่มต้นต้อง DISARM เสมอ (เส้นแดงด้านความปลอดภัย)
  SystemState::set(STATE_DISARMED);

  // --- เปิด watchdog หลังทุกอย่างพร้อม ---
  // ถ้า loop ค้างเกิน 1 วินาที → รีเซต MCU
  wdt_enable(WDTO_1S);
}

// =====================================================
// LOOP (NON-BLOCKING)
// =====================================================
void loop() {

  // รีเฟรช watchdog ทุก loop
  wdt_reset();

  // ---------- INPUT & SAFETY ----------
  IBusManager::update();     // อ่าน IBUS (non-blocking)
  SafetyManager::update();   // ตรวจ ARM / FAILSAFE
  SystemState::update();     // ตัดสินใจ state กลาง

  // ---------- SYSTEM STATE ----------
  switch (SystemState::get()) {

    // ================================
    // DISARMED
    // ================================
    // - ห้ามขยับทุกอย่าง
    // - เครื่องยนต์ต้องดับ
    // - รีเลย์ทั้งหมด OFF
    case STATE_DISARMED:

      DriveManager::stop();        // มอเตอร์หยุด
      EngineManager::disarmed();   // Ignition OFF + Throttle 0
      RelayManager::safe();        // รีเลย์ OFF

      break;

    // ================================
    // ACTIVE (ARM แล้ว และไม่ FAILSAFE)
    // ================================
    case STATE_ACTIVE:

      DriveManager::update();      // ขับเคลื่อน
      EngineManager::update();     // ควบคุมเครื่องยนต์
      RelayManager::update();      // ไฟ / รีเลย์เสริม

      break;

    // ================================
    // FAILSAFE (IBUS หาย / error)
    // ================================
    // ต้อง "ตัดทุกอย่าง" แบบเด็ดขาด
    case STATE_FAILSAFE:

      DriveManager::failsafe();    // มอเตอร์หยุด
      EngineManager::failsafe();   // Ignition OFF (สำคัญมาก)
      RelayManager::failsafe();    // รีเลย์ OFF

      break;

    // ================================
    // กัน state หลุด (ป้องกัน bug เงียบ)
    // ================================
    default:
      SystemState::set(STATE_DISARMED);
      break;
  }
}

