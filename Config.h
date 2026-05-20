// =====================================================
// Config.h
// TN Mower Central Configuration File
//
// หน้าที่:
// - รวมค่าคงที่ของทั้งระบบไว้จุดเดียว
// - ลด magic number
// - ป้องกัน pin conflict
// - ปรับ tuning ได้ง่าย
//
// IMPORTANT:
// - ห้ามเก็บ runtime variables ที่นี่
// - ใช้เฉพาะ configuration constants เท่านั้น
// =====================================================

#pragma once

#include <stdint.h>

// =====================================================
// SYSTEM VERSION
// =====================================================
#define TNMOWER_CONFIG_VERSION "1.0"

// =====================================================
// IBUS CHANNEL MAPPING
// =====================================================
//
// FlySky Channel Layout
//
// CH1 = Steering
// CH2 = Drive Throttle
// CH3 = Engine Throttle Servo
// CH4 = Reserved
// CH5 = ARM / DISARM
// CH6 = Ignition
// CH7 = Starter
// CH8 = Light Relay
//
// IMPORTANT:
// - ค่า index เริ่มจาก 0
// - IBusManager::ch(0) = CH1
// =====================================================

// CH1
constexpr uint8_t IBUS_CH_STEER = 0;

// CH2
constexpr uint8_t IBUS_CH_DRIVE = 1;

// CH3
constexpr uint8_t IBUS_CH_ENGINE_THROTTLE = 2;

// CH4
constexpr uint8_t IBUS_CH_RESERVED_4 = 3;

// CH5
constexpr uint8_t IBUS_CH_ARM = 4;

// CH6
constexpr uint8_t IBUS_CH_IGNITION = 5;

// CH7
constexpr uint8_t IBUS_CH_STARTER = 6;

// CH8
constexpr uint8_t IBUS_CH_LIGHT = 7;

// CH9
constexpr uint8_t IBUS_CH_RESERVED_9 = 8;

// CH10
constexpr uint8_t IBUS_CH_RESERVED_10 = 9;

// =====================================================
// RC SIGNAL CONFIGURATION
// =====================================================
//
// FlySky IBUS Standard:
//   1000 = minimum
//   1500 = center
//   2000 = maximum
// =====================================================

constexpr int RC_MIN     = 1000;
constexpr int RC_CENTER  = 1500;
constexpr int RC_MAX     = 2000;

constexpr int RC_RANGE   = 500;

// =====================================================
// PWM CONFIGURATION
// =====================================================

constexpr int PWM_MAX = 255;

// =====================================================
// DRIVE TUNING
// =====================================================
//
// DEADZONE
//   - กันสติ๊กไม่นิ่ง
//
// SLEW_RATE
//   - จำกัดการเปลี่ยน PWM ต่อ loop
//   - ลดกระชาก
// =====================================================

constexpr int DRIVE_DEADZONE = 40;
constexpr int DRIVE_SLEW_RATE = 5;

// =====================================================
// IBUS VALIDATION
// =====================================================
//
// ช่วงที่ถือว่า valid ก่อน constrain
// =====================================================

constexpr int IBUS_VALID_MIN = 900;
constexpr int IBUS_VALID_MAX = 2100;

// จำนวน invalid frame ติดต่อกันที่ยอมรับได้
constexpr uint8_t IBUS_ERROR_THRESHOLD = 8;

// =====================================================
// FAILSAFE TIMING
// =====================================================
//
// FAILSAFE_DEAD_CONFIRM
//   - ต้อง dead ต่อเนื่องนานเท่าไร
//   - ก่อนเข้า FAILSAFE
//
// FAILSAFE_CLEAR_CONFIRM
//   - ต้อง alive ต่อเนื่องนานเท่าไร
//   - ก่อนออก FAILSAFE
// =====================================================

constexpr unsigned long FAILSAFE_DEAD_CONFIRM  = 150UL;
constexpr unsigned long FAILSAFE_CLEAR_CONFIRM = 200UL;

// =====================================================
// WATCHDOG CONFIGURATION
// =====================================================

constexpr unsigned long WATCHDOG_TIMEOUT_MS = 1000UL;

// =====================================================
// ENGINE CONFIGURATION
// =====================================================

// starter timeout
constexpr unsigned long ENGINE_STARTER_TIMEOUT = 2500UL;

// servo angle
constexpr int THROTTLE_SERVO_MIN = 0;
constexpr int THROTTLE_SERVO_MAX = 180;

// idle angle
constexpr int THROTTLE_IDLE_ANGLE = 0;

// =====================================================
// MOTOR DRIVER PINS
// =====================================================

// -----------------------------------------------------
// Left Front Motor
// -----------------------------------------------------
constexpr uint8_t LF_EN   = 4;
constexpr uint8_t LF_LPWM = 6;
constexpr uint8_t LF_RPWM = 5;

// -----------------------------------------------------
// Left Rear Motor
// -----------------------------------------------------
constexpr uint8_t LR_EN   = 22;
constexpr uint8_t LR_LPWM = 10;
constexpr uint8_t LR_RPWM = 11;

// -----------------------------------------------------
// Right Front Motor
// -----------------------------------------------------
constexpr uint8_t RF_EN   = 9;
constexpr uint8_t RF_LPWM = 8;
constexpr uint8_t RF_RPWM = 7;

// -----------------------------------------------------
// Right Rear Motor
// -----------------------------------------------------
constexpr uint8_t RR_EN   = 26;
constexpr uint8_t RR_LPWM = 12;
constexpr uint8_t RR_RPWM = 13;

// =====================================================
// ENGINE CONTROL PINS
// =====================================================

constexpr uint8_t SERVO_THROTTLE_PIN = 30;

constexpr uint8_t RELAY_IGNITION = 31;
constexpr uint8_t RELAY_STARTER  = 32;

// =====================================================
// AUXILIARY RELAY PINS
// =====================================================

constexpr uint8_t RELAY_LIGHT_PIN = 24;

// =====================================================
// SERIAL CONFIGURATION
// =====================================================

constexpr unsigned long IBUS_BAUDRATE = 115200UL;

// =====================================================
// IBUS TIMEOUT
// =====================================================
//
// ถ้าไม่มี valid frame ภายในเวลานี้
// ถือว่าสัญญาณหาย
// =====================================================

constexpr unsigned long IBUS_SIGNAL_TIMEOUT = 500UL;

// =====================================================
// SYSTEM SAFETY
// =====================================================

// boot default state
constexpr bool SYSTEM_BOOT_FAILSAFE = true;

// =====================================================
// END OF FILE
// =====================================================

