// =====================================================
// MegaBt2Ibus.ino
// TN Mower Main Runtime
//
// Responsibilities:
// - initialize all subsystems
// - run deterministic main loop
// - update system state
// - dispatch ACTIVE / DISARM / FAILSAFE behavior
//
// IMPORTANT:
// - non-blocking architecture only
// - SystemState = single source of truth
// =====================================================

#include <Arduino.h>
#include <avr/wdt.h>

#include "Config.h"
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

  // ---------------------------------------------------
  // Disable Watchdog During Boot
  // ---------------------------------------------------
  //
  // IMPORTANT:
  // - ป้องกัน reset loop ระหว่าง boot
  // ---------------------------------------------------
  wdt_disable();

  // ===================================================
  // Initialize Subsystems
  // ===================================================
  //
  // IMPORTANT:
  // - ลำดับนี้ห้ามสลับ
  // ===================================================

  // ---------------------------------------------------
  // Input System
  // ---------------------------------------------------
  //
  // ต้องเริ่มก่อน
  // เพราะ subsystem อื่นใช้ข้อมูล IBUS
  // ---------------------------------------------------
  IBusManager::begin();

  // ---------------------------------------------------
  // Safety System
  // ---------------------------------------------------
  //
  // ใช้ข้อมูลจาก IBusManager
  // ---------------------------------------------------
  SafetyManager::begin();

  // ---------------------------------------------------
  // Drive System
  // ---------------------------------------------------
  DriveManager::begin();

  // ---------------------------------------------------
  // Engine System
  // ---------------------------------------------------
  EngineManager::begin();

  // ---------------------------------------------------
  // Auxiliary Relays
  // ---------------------------------------------------
  RelayManager::begin();

  // ===================================================
  // Initial System State
  // ===================================================
  //
  // IMPORTANT:
  // - boot ต้องเริ่ม DISARM เสมอ
  // - safety priority สูงสุด
  // ===================================================
  SystemState::set(
    STATE_DISARMED
  );

  // ===================================================
  // Enable Watchdog
  // ===================================================
  //
  // IMPORTANT:
  // - AVR watchdog ใช้ enum
  // - ยังไม่ map จาก WATCHDOG_TIMEOUT_MS
  // ===================================================
  wdt_enable(WDTO_1S);
}

// =====================================================
// MAIN LOOP
// =====================================================
//
// IMPORTANT:
// - deterministic
// - non-blocking
// - no delay()
// =====================================================
void loop() {

  // ---------------------------------------------------
  // Refresh Watchdog
  // ---------------------------------------------------
  wdt_reset();

  // ===================================================
  // INPUT + SAFETY UPDATE
  // ===================================================

  // ---------------------------------------------------
  // Update IBUS
  // ---------------------------------------------------
  IBusManager::update();

  // ---------------------------------------------------
  // Update Safety Logic
  // ---------------------------------------------------
  SafetyManager::update();

  // ---------------------------------------------------
  // Update System State
  // ---------------------------------------------------
  SystemState::update();

  // ===================================================
  // SYSTEM STATE DISPATCH
  // ===================================================
  switch (SystemState::get()) {

    // =================================================
    // STATE_DISARMED
    // =================================================
    //
    // Requirements:
    // - drive stop
    // - engine OFF
    // - relay OFF
    // =================================================
    case STATE_DISARMED:

      // controlled stop
      DriveManager::stop();

      // shutdown engine
      EngineManager::disarmed();

      // relay safe state
      RelayManager::safe();

      break;

    // =================================================
    // STATE_ACTIVE
    // =================================================
    //
    // Requirements:
    // - normal operation allowed
    // =================================================
    case STATE_ACTIVE:

      // drive control
      DriveManager::update();

      // engine control
      EngineManager::update();

      // auxiliary relay update
      RelayManager::update();

      break;

    // =================================================
    // STATE_FAILSAFE
    // =================================================
    //
    // Requirements:
    // - emergency stop
    // - immediate shutdown
    // =================================================
    case STATE_FAILSAFE:

      // emergency motor stop
      DriveManager::failsafe();

      // emergency engine shutdown
      EngineManager::failsafe();

      // emergency relay shutdown
      RelayManager::failsafe();

      break;

    // =================================================
    // INVALID STATE PROTECTION
    // =================================================
    //
    // IMPORTANT:
    // - กัน corrupted state
    // - fallback safe state
    // =================================================
    default:

      SystemState::set(
        STATE_DISARMED
      );

      break;
  }
}

