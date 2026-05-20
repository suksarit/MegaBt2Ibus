// =====================================================
// IBusManager.h
// TN Mower FlySky IBUS Input Interface
//
// Responsibilities:
// - อ่านสัญญาณ IBUS จาก Serial1
// - เก็บค่าช่องสัญญาณล่าสุด
// - ตรวจสอบ signal alive
// - validate frame ก่อนใช้งาน
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - ไม่ทำ FAILSAFE decision ที่นี่
// - FAILSAFE อยู่ใน SafetyManager
//
// Configuration:
// - ทุกค่าคงที่อยู่ใน Config.h
// =====================================================

#pragma once

#include <stdint.h>

#include "Config.h"

// =====================================================
// IBus Manager Class
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
// ch()
//   - อ่าน channel ล่าสุด
//
// isAlive()
//   - ตรวจสอบ signal timeout
// =====================================================
class IBusManager {
public:

  // ---------------------------------------------------
  // Initialize IBUS System
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - start Serial1
  // - initialize IBusBM
  // - reset channel buffer
  // - reset runtime state
  //
  // IMPORTANT:
  // - ต้องเรียกก่อนใช้งานเสมอ
  // ---------------------------------------------------
  static void begin();

  // ---------------------------------------------------
  // Update IBUS Data
  // ---------------------------------------------------
  //
  // หน้าที่:
  // - parse serial data
  // - อ่านทุก channel
  // - validate frame
  // - update heartbeat
  //
  // IMPORTANT:
  // - non-blocking only
  // - ต้องถูกเรียกทุก loop
  // ---------------------------------------------------
  static void update();

  // ---------------------------------------------------
  // Read Channel
  // ---------------------------------------------------
  //
  // index:
  //   0 = CH1
  //   1 = CH2
  //   ...
  //   9 = CH10
  //
  // return:
  //   ~1000 ถึง 2000
  //
  // invalid index:
  //   return RC_CENTER
  // ---------------------------------------------------
  static int ch(int index);

  // ---------------------------------------------------
  // IBUS Alive Status
  // ---------------------------------------------------
  //
  // true:
  //   ยัง receive valid frame อยู่
  //
  // false:
  //   signal timeout
  //
  // IMPORTANT:
  // - ใช้สำหรับ SafetyManager เท่านั้น
  // ---------------------------------------------------
  static bool isAlive();
};

