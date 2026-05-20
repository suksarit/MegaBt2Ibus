// =====================================================
// Config.h
// TN Mower Central Configuration File
//
// Responsibilities:
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
#define TNMOWER_CONFIG_VERSION "1.1"

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

constexpr uint8_t IBUS_CH_STEER            = 0;
constexpr uint8_t IBUS_CH_DRIVE            = 1;
constexpr uint8_t IBUS_CH_ENGINE_THROTTLE  = 2;
constexpr uint8_t IBUS_CH_RESERVED_4       = 3;
constexpr uint8_t IBUS_CH_ARM              = 4;
constexpr uint8_t IBUS_CH_IGNITION         = 5;
constexpr uint8_t IBUS_CH_STARTER          = 6;
constexpr uint8_t IBUS_CH_LIGHT            = 7;
constexpr uint8_t IBUS_CH_RESERVED_9       = 8;
constexpr uint8_t IBUS_CH_RESERVED_10      = 9;

// =====================================================
// RC SIGNAL CONFIGURATION
// =====================================================

constexpr int RC_MIN    = 1000;
constexpr int RC_CENTER = 1500;
constexpr int RC_MAX    = 2000;

constexpr int RC_RANGE  = 500;

// =====================================================
// PWM CONFIGURATION
// =====================================================

constexpr int PWM_MAX = 255;

// =====================================================
// DRIVE TUNING
// =====================================================

constexpr int DRIVE_DEADZONE  = 40;

// ลดกระชากตอนเปลี่ยนทิศ
// 3 จะ smooth กว่า 5
constexpr int DRIVE_SLEW_RATE = 3;

// =====================================================
// IBUS VALIDATION
// =====================================================

constexpr int IBUS_VALID_MIN = 900;
constexpr int IBUS_VALID_MAX = 2100;

constexpr uint8_t IBUS_ERROR_THRESHOLD = 8;

// =====================================================
// FAILSAFE TIMING
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

// -----------------------------------------------------
// Servo Angle Configuration
// -----------------------------------------------------
//
// IMPORTANT:
// - ค่าเหล่านี้อิงตาม Servo.h library
// - ไม่ใช่มุมจริงทางกล 100%
// - ต้อง calibrate กับ hardware จริง
// -----------------------------------------------------

// จำกัดช่วง servo จริง อิงตามไลบราลี่
constexpr int THROTTLE_SERVO_MIN = 30;

// full throttle limit อิงตามไลบราลี่
constexpr int THROTTLE_SERVO_MAX = 120;

// idle จริง อิงตามไลบราลี่
constexpr int THROTTLE_IDLE_ANGLE = 35;

// =====================================================
// THROTTLE SERVO FILTERING
// =====================================================

// กัน RC jitter
constexpr int THROTTLE_DEADBAND = 8;

// servo smoothing
constexpr int THROTTLE_SERVO_SLEW = 2;

// =====================================================
// MOTOR DRIVER PINS
// =====================================================
//
// IMPORTANT:
// - ใช้ PWM pin จริงทั้งหมด
// - หลีกเลี่ยง pin 13
// - grouping ตามด้านรถ
//
// Mega2560 PWM Pins:
// 2-13, 44-46
// =====================================================

// -----------------------------------------------------
// LEFT FRONT MOTOR
// -----------------------------------------------------
constexpr uint8_t LF_EN   = 22;
constexpr uint8_t LF_LPWM = 5;
constexpr uint8_t LF_RPWM = 6;

// -----------------------------------------------------
// LEFT REAR MOTOR
// -----------------------------------------------------
constexpr uint8_t LR_EN   = 23;
constexpr uint8_t LR_LPWM = 7;
constexpr uint8_t LR_RPWM = 8;

// -----------------------------------------------------
// RIGHT FRONT MOTOR
// -----------------------------------------------------
constexpr uint8_t RF_EN   = 24;
constexpr uint8_t RF_LPWM = 9;
constexpr uint8_t RF_RPWM = 10;

// -----------------------------------------------------
// RIGHT REAR MOTOR
// -----------------------------------------------------
constexpr uint8_t RR_EN   = 25;
constexpr uint8_t RR_LPWM = 11;
constexpr uint8_t RR_RPWM = 12;

// =====================================================
// ENGINE CONTROL PINS
// =====================================================

// Servo library ไม่ต้องใช้ PWM pin
constexpr uint8_t SERVO_THROTTLE_PIN = 30;

// ignition relay
constexpr uint8_t RELAY_IGNITION = 31;

// starter relay
constexpr uint8_t RELAY_STARTER  = 32;

// =====================================================
// AUXILIARY RELAY PINS
// =====================================================

constexpr uint8_t RELAY_LIGHT_PIN = 33;

// =====================================================
// SERIAL CONFIGURATION
// =====================================================

constexpr unsigned long IBUS_BAUDRATE = 115200UL;

// =====================================================
// IBUS TIMEOUT
// =====================================================

constexpr unsigned long IBUS_SIGNAL_TIMEOUT = 500UL;

// =====================================================
// SYSTEM SAFETY
// =====================================================

constexpr bool SYSTEM_BOOT_FAILSAFE = true;

// =====================================================
// END OF FILE
// =====================================================

