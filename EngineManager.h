// =====================================================
// EngineManager.h
// TN Mower Engine Control Interface
//
// Responsibilities:
// - ควบคุมเครื่องยนต์
// - ควบคุม throttle servo
// - ควบคุม ignition relay
// - ควบคุม starter relay
// - จัดการ engine state machine
//
// Features:
// - Starter timeout protection
// - Starter re-trigger lock
// - Servo deadband filtering
// - Servo slew limiting
// - Safe servo write
// - Throttle lock during crank
//
// Works With:
// - SystemState
// - IBusManager
//
// IMPORTANT:
// - ไฟล์นี้เป็น interface เท่านั้น
// - logic จริงอยู่ใน EngineManager.cpp
// - configuration ทั้งหมดอยู่ใน Config.h
// =====================================================

#pragma once

#include <stdint.h>

#include "Config.h"

// =====================================================
// Engine Manager Class
// =====================================================
//
// Runtime Flow:
//
// begin()
//   -> setup hardware + reset runtime state
//
// update()
//   -> update engine state machine
//
// disarmed()
//   -> normal safe shutdown
//
// failsafe()
//   -> emergency shutdown
// =====================================================
class EngineManager {
public:

  // ---------------------------------------------------
  // Initialize Engine System
  // ---------------------------------------------------
  //
  // Responsibilities:
  // - attach throttle servo
  // - initialize servo runtime state
  // - setup ignition relay
  // - setup starter relay
  // - reset engine state machine
  // - shutdown outputs
  //
  // IMPORTANT:
  // - ต้องเรียกก่อนใช้งานเสมอ
  // ---------------------------------------------------
  static void begin();

  // ---------------------------------------------------
  // Update Engine State Machine
  // ---------------------------------------------------
  //
  // Responsibilities:
  // - อ่านคำสั่งจาก IBusManager
  // - update ignition state
  // - update starter state
  // - update throttle target
  // - apply servo deadband
  // - apply servo slew limiter
  // - apply safe servo write
  // - handle engine state transition
  //
  // Features:
  // - non-blocking
  // - deterministic behavior
  // - starter timeout
  // - starter lock
  // - throttle safety lock
  // - servo smoothing
  //
  // IMPORTANT:
  // - ใช้เฉพาะ STATE_ACTIVE
  // ---------------------------------------------------
  static void update();

  // ---------------------------------------------------
  // DISARMED State
  // ---------------------------------------------------
  //
  // Responsibilities:
  // - throttle idle
  // - ignition OFF
  // - starter OFF
  // - reset engine state
  // - reset servo target
  // - reset runtime protection state
  //
  // ใช้เมื่อ:
  // - STATE_DISARMED
  // ---------------------------------------------------
  static void disarmed();

  // ---------------------------------------------------
  // FAILSAFE State
  // ---------------------------------------------------
  //
  // Responsibilities:
  // - emergency shutdown
  // - force safe outputs
  // - reset runtime state
  //
  // IMPORTANT:
  // - safety priority สูงสุด
  // - immediate shutdown
  // ---------------------------------------------------
  static void failsafe();
};

