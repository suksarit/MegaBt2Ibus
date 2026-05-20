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
// Runtime State Tracking
// =====================================================
static SystemState_t lastState =
  STATE_DISARMED;

// =====================================================
// Convert Config Timeout -> AVR Watchdog Enum
// =====================================================
//
// IMPORTANT:
// - AVR watchdog ไม่รับ millisecond ตรง ๆ
// - ต้อง map เป็น enum ของ AVR เท่านั้น
// =====================================================
static uint8_t watchdogConfig() {

  if (WATCHDOG_TIMEOUT_MS <= 15UL) {
    return WDTO_15MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 30UL) {
    return WDTO_30MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 60UL) {
    return WDTO_60MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 120UL) {
    return WDTO_120MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 250UL) {
    return WDTO_250MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 500UL) {
    return WDTO_500MS;
  }

  if (WATCHDOG_TIMEOUT_MS <= 1000UL) {
    return WDTO_1S;
  }

  if (WATCHDOG_TIMEOUT_MS <= 2000UL) {
    return WDTO_2S;
  }

  if (WATCHDOG_TIMEOUT_MS <= 4000UL) {
    return WDTO_4S;
  }

  return WDTO_8S;
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  // ---------------------------------------------------
  // Disable Watchdog During Boot
  // ---------------------------------------------------
  wdt_disable();

  // ===================================================
  // Initialize Subsystems
  // ===================================================

  IBusManager::begin();

  SafetyManager::begin();

  DriveManager::begin();

  EngineManager::begin();

  RelayManager::begin();

  // ===================================================
  // Initial System State
  // ===================================================
  SystemState::set(
    STATE_DISARMED
  );

  // reset runtime state tracking
  lastState =
    STATE_DISARMED;

  // ===================================================
  // Enable Watchdog
  // ===================================================
  wdt_enable(
    watchdogConfig()
  );
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {

  // ---------------------------------------------------
  // Refresh Watchdog
  // ---------------------------------------------------
  wdt_reset();

  // ===================================================
  // INPUT + SAFETY UPDATE
  // ===================================================
  IBusManager::update();

  SafetyManager::update();

  SystemState::update();

  // ===================================================
  // Get Current State
  // ===================================================
  SystemState_t currentState =
    SystemState::get();

  // ===================================================
  // SYSTEM STATE DISPATCH
  // ===================================================
  switch (currentState) {

    // =================================================
    // STATE_DISARMED
    // =================================================
    case STATE_DISARMED:

      DriveManager::stop();

      EngineManager::disarmed();

      RelayManager::safe();

      break;

    // =================================================
    // STATE_ACTIVE
    // =================================================
    case STATE_ACTIVE:

      DriveManager::update();

      EngineManager::update();

      RelayManager::update();

      break;

    // =================================================
    // STATE_FAILSAFE
    // =================================================
    case STATE_FAILSAFE:

      // -----------------------------------------------
      // Execute FAILSAFE only on state entry
      // -----------------------------------------------
      if (lastState != STATE_FAILSAFE) {

        DriveManager::failsafe();

        EngineManager::failsafe();

        RelayManager::failsafe();
      }

      break;

    // =================================================
    // INVALID STATE PROTECTION
    // =================================================
    default:

      SystemState::set(
        STATE_DISARMED
      );

      break;
  }

  // ===================================================
  // Update Runtime State Tracking
  // ===================================================
  lastState =
    currentState;
}

