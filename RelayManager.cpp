// =====================================================
// RelayManager.cpp
// TN Mower Auxiliary Relay Controller
//
// Controls:
// - Light Relay
//
// Features:
// - Relay chatter protection
// - Safe shutdown
// - FAILSAFE forced shutdown
//
// IMPORTANT:
// - ไม่ตัดสิน FAILSAFE ที่นี่
// - รับคำสั่งจาก IBusManager เท่านั้น
// - Configuration อยู่ใน Config.h
// =====================================================

#include <Arduino.h>

#include "Config.h"
#include "RelayManager.h"
#include "IBusManager.h"

// =====================================================
// Internal Runtime State
// =====================================================

// สถานะ relay ล่าสุด
// ใช้ลด relay chatter
static bool lightState = false;

// =====================================================
// Apply Relay Output
// =====================================================
//
// IMPORTANT:
// - เขียน output เฉพาะเมื่อ state เปลี่ยน
// - ลด relay chatter
// - ลด relay wear
// =====================================================
static void applyLight(bool on) {

  // ---------------------------------------------------
  // update only when state changed
  // ---------------------------------------------------
  if (lightState != on) {

    lightState = on;

    digitalWrite(
      RELAY_LIGHT_PIN,
      on ? HIGH : LOW
    );
  }
}

// =====================================================
// Initialize Relay Manager
// =====================================================
void RelayManager::begin() {

  // ---------------------------------------------------
  // setup relay output
  // ---------------------------------------------------
  pinMode(
    RELAY_LIGHT_PIN,
    OUTPUT
  );

  // ---------------------------------------------------
  // default safe state
  // ---------------------------------------------------
  digitalWrite(
    RELAY_LIGHT_PIN,
    LOW
  );

  // ---------------------------------------------------
  // reset runtime state
  // ---------------------------------------------------
  lightState = false;
}

// =====================================================
// Update Relay State
// =====================================================
//
// IMPORTANT:
// - ใช้เฉพาะ STATE_ACTIVE
// - non-blocking only
// =====================================================
void RelayManager::update() {

  // ---------------------------------------------------
  // Read IBUS Command
  // ---------------------------------------------------
  bool cmdLight =
    (
      IBusManager::ch(
        IBUS_CH_LIGHT
      )
      >
      RC_CENTER
    );

  // ---------------------------------------------------
  // Apply Relay Output
  // ---------------------------------------------------
  applyLight(cmdLight);
}

// =====================================================
// Safe State
// =====================================================
//
// ใช้เมื่อ:
// - STATE_DISARMED
// =====================================================
void RelayManager::safe() {

  applyLight(false);
}

// =====================================================
// FAILSAFE State
// =====================================================
//
// IMPORTANT:
// - highest safety priority
// - force relay OFF
// =====================================================
void RelayManager::failsafe() {

  // ---------------------------------------------------
  // force output mode
  // ---------------------------------------------------
  pinMode(
    RELAY_LIGHT_PIN,
    OUTPUT
  );

  // ---------------------------------------------------
  // force relay OFF
  // ---------------------------------------------------
  digitalWrite(
    RELAY_LIGHT_PIN,
    LOW
  );

  // ---------------------------------------------------
  // reset runtime state
  // ---------------------------------------------------
  lightState = false;
}

