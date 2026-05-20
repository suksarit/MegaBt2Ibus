// =====================================================
// IBusManager.cpp
// TN Mower FlySky IBUS Input Manager
//
// Responsibilities:
// - อ่าน IBUS frame จาก Serial1
// - เก็บ channel ล่าสุด
// - ตรวจสอบ signal alive
// - validate frame
//
// IMPORTANT:
// - ไม่ทำ FAILSAFE ที่นี่
// - FAILSAFE decision อยู่ที่ SafetyManager
//
// Configuration:
// - ทุกค่าคงที่อยู่ใน Config.h
// =====================================================

#include <Arduino.h>
#include <IBusBM.h>

#include "Config.h"
#include "IBusManager.h"

// =====================================================
// Internal Objects
// =====================================================

// IBusBM object
static IBusBM ibus;

// =====================================================
// Internal Runtime State
// =====================================================

// channel ล่าสุด
static int channels[10] = {

  RC_CENTER,
  RC_CENTER,
  RC_CENTER,
  RC_CENTER,
  RC_CENTER,

  RC_CENTER,
  RC_CENTER,
  RC_CENTER,
  RC_CENTER,
  RC_CENTER
};

// เวลา receive valid frame ล่าสุด
static unsigned long lastUpdateMs = 0;

// ตัวนับ invalid frame
static uint8_t errorCount = 0;

// =====================================================
// Initialize IBUS
// =====================================================
void IBusManager::begin() {

  // ---------------------------------------------------
  // Start Serial1
  // ---------------------------------------------------
  Serial1.begin(
    IBUS_BAUDRATE
  );

  // ---------------------------------------------------
  // Start IBusBM
  // ---------------------------------------------------
  ibus.begin(Serial1);

  // ---------------------------------------------------
  // Reset Channels
  // ---------------------------------------------------
  for (int i = 0; i < 10; i++) {

    channels[i] =
      RC_CENTER;
  }

  // ---------------------------------------------------
  // Reset Runtime State
  // ---------------------------------------------------
  lastUpdateMs = 0;

  errorCount = 0;
}

// =====================================================
// Update IBUS Data
// =====================================================
//
// IMPORTANT:
// - non-blocking only
// - ต้องถูกเรียกทุก loop
// =====================================================
void IBusManager::update() {

  // ---------------------------------------------------
  // Parse incoming serial data
  // ---------------------------------------------------
  ibus.loop();

  // ---------------------------------------------------
  // Temporary buffer
  // IMPORTANT:
  // - ป้องกัน partial frame update
  // ---------------------------------------------------
  int tempChannels[10];

  // ===================================================
  // Read + Validate All Channels
  // ===================================================
  for (int i = 0; i < 10; i++) {

    // -------------------------------------------------
    // Read channel from IBusBM
    // -------------------------------------------------
    int v =
      ibus.readChannel(i);

    // -------------------------------------------------
    // Validate raw value
    // -------------------------------------------------
    if (
      v < IBUS_VALID_MIN ||
      v > IBUS_VALID_MAX
    ) {

      // -----------------------------------------------
      // Increase error counter
      // -----------------------------------------------
      if (errorCount < 255) {
        errorCount++;
      }

      // -----------------------------------------------
      // Invalid frame
      // discard both partial + full update
      // -----------------------------------------------
      return;
    }

    // -------------------------------------------------
    // constrain to safe RC range
    // -------------------------------------------------
    v = constrain(
      v,
      RC_MIN,
      RC_MAX
    );

    // -------------------------------------------------
    // store into temp buffer first
    // -------------------------------------------------
    tempChannels[i] = v;
  }

  // ===================================================
  // Valid Frame
  // ===================================================

  // ---------------------------------------------------
  // Reset error counter
  // ---------------------------------------------------
  errorCount = 0;

  // ---------------------------------------------------
  // Commit temp buffer
  // IMPORTANT:
  // - commit only after full validation
  // ---------------------------------------------------
  for (int i = 0; i < 10; i++) {

    channels[i] =
      tempChannels[i];
  }

  // ---------------------------------------------------
  // Update heartbeat
  // ---------------------------------------------------
  lastUpdateMs =
    millis();
}

// =====================================================
// Read Channel
// =====================================================
int IBusManager::ch(int index) {

  // ---------------------------------------------------
  // Prevent invalid index
  // ---------------------------------------------------
  if (
    index < 0 ||
    index > 9
  ) {

    return RC_CENTER;
  }

  return channels[index];
}

// =====================================================
// IBUS Alive Check
// =====================================================
//
// true:
//   ยัง receive valid frame อยู่
//
// false:
//   timeout
// =====================================================
bool IBusManager::isAlive() {

  // ---------------------------------------------------
  // Never received valid frame
  // ---------------------------------------------------
  if (lastUpdateMs == 0) {

    return false;
  }

  // ---------------------------------------------------
  // Signal timeout
  // ---------------------------------------------------
  return (
    (millis() - lastUpdateMs)
    <
    IBUS_SIGNAL_TIMEOUT
  );
}

