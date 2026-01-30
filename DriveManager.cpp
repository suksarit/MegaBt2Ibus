// =====================================================
// DriveManager.cpp
// Tank Drive Controller (BTS7960)
//
// Features:
// - Tank drive (Left / Right)
// - Soft start / soft stop (slew rate)
// - Deadzone protection
// - Non-blocking
//
// Safety Policy:
// - stop()     = controlled stop (DISARM)
// - failsafe() = immediate stop (FAILSAFE)
//
// NOTE:
// - ไม่ตัดสิน ARM / FAILSAFE ที่นี่
// - DriveManager ทำหน้าที่ "ลงมือขับ" เท่านั้น
// =====================================================

#include <Arduino.h>
#include <BTS7960.h>
#include "DriveManager.h"
#include "IBusManager.h"

// -----------------------------------------------------
// Hardware Configuration
// -----------------------------------------------------
#define L_EN    4
#define L_LPWM  6
#define L_RPWM  5

#define R_EN    9
#define R_LPWM  8
#define R_RPWM  7

// -----------------------------------------------------
// Drive Configuration
// -----------------------------------------------------
#define PWM_MAX    255
#define DEADZONE   40
#define SLEW_RATE  5   // PWM step per loop

// -----------------------------------------------------
// Motor Drivers
// -----------------------------------------------------
static BTS7960 motorL(L_EN, L_LPWM, L_RPWM);
static BTS7960 motorR(R_EN, R_LPWM, R_RPWM);

// -----------------------------------------------------
// Internal State
// -----------------------------------------------------
static int targetL = 0;
static int targetR = 0;
static int outputL = 0;
static int outputR = 0;

// -----------------------------------------------------
// Utility Functions
// -----------------------------------------------------
static int applyDeadzone(int v) {
  if (abs(v) < DEADZONE) return 0;
  return v;
}

// slew ที่ไม่อนุญาตให้ "ข้ามผ่าน 0"
static int slewSafe(int current, int target) {

  // ถ้ากำลังจะกลับทิศ → ต้องหยุดที่ 0 ก่อน
  if ((current > 0 && target < 0) ||
      (current < 0 && target > 0)) {
    target = 0;
  }

  if (current < target) {
    current += SLEW_RATE;
    if (current > target) current = target;
  } else if (current > target) {
    current -= SLEW_RATE;
    if (current < target) current = target;
  }
  return current;
}

static void setMotor(BTS7960 &motor, int pwm) {
  pwm = constrain(pwm, -PWM_MAX, PWM_MAX);

  if (pwm > 0) {
    motor.TurnRight(pwm);
  } else if (pwm < 0) {
    motor.TurnLeft(-pwm);
  } else {
    motor.Stop();
  }
}

// -----------------------------------------------------
// Initialize Drive Manager
// -----------------------------------------------------
void DriveManager::begin() {

  motorL.Enable();
  motorR.Enable();

  targetL = targetR = 0;
  outputL = outputR = 0;

  motorL.Stop();
  motorR.Stop();
}

// -----------------------------------------------------
// Update Drive (STATE_ACTIVE only)
// -----------------------------------------------------
void DriveManager::update() {

  int steer    = map(IBusManager::ch(0), 1000, 2000, -PWM_MAX, PWM_MAX);
  int throttle = map(IBusManager::ch(1), 1000, 2000, -PWM_MAX, PWM_MAX);

  steer    = applyDeadzone(steer);
  throttle = applyDeadzone(throttle);

  targetL = constrain(throttle + steer, -PWM_MAX, PWM_MAX);
  targetR = constrain(throttle - steer, -PWM_MAX, PWM_MAX);

  outputL = slewSafe(outputL, targetL);
  outputR = slewSafe(outputR, targetR);

  setMotor(motorL, outputL);
  setMotor(motorR, outputR);
}

// -----------------------------------------------------
// Controlled Stop (DISARM)
// -----------------------------------------------------
void DriveManager::stop() {

  targetL = 0;
  targetR = 0;

  outputL = slewSafe(outputL, 0);
  outputR = slewSafe(outputR, 0);

  setMotor(motorL, outputL);
  setMotor(motorR, outputR);
}

// -----------------------------------------------------
// FAILSAFE (Immediate Stop)
// -----------------------------------------------------
void DriveManager::failsafe() {

  targetL = 0;
  targetR = 0;
  outputL = 0;
  outputR = 0;

  motorL.Stop();
  motorR.Stop();

  // FAILSAFE = disable driver ชัดเจน
  motorL.Disable();
  motorR.Disable();
}
