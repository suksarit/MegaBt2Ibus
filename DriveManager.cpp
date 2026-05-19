// =====================================================
// DriveManager.cpp
// Tank Drive Controller (4 Motors / 4 BTS7960)
//
// Configuration:
// - Left Front  + Left Rear  = Left Track
// - Right Front + Right Rear = Right Track
//
// Safety Policy:
// - stop()     = soft stop
// - failsafe() = immediate stop + disable drivers
// =====================================================

#include <Arduino.h>
#include <BTS7960.h>
#include "DriveManager.h"
#include "IBusManager.h"

// -----------------------------------------------------
// Hardware Configuration (ADJUST TO YOUR WIRING)
// -----------------------------------------------------

// -----------------------------------------------------
// Left Front Motor
// -----------------------------------------------------
#define LF_EN     4
#define LF_LPWM   6
#define LF_RPWM   5

// -----------------------------------------------------
// Left Rear Motor
// -----------------------------------------------------
#define LR_EN     22
#define LR_LPWM   10
#define LR_RPWM   11

// -----------------------------------------------------
// Right Front Motor
// -----------------------------------------------------
#define RF_EN     9
#define RF_LPWM   8
#define RF_RPWM   7

// -----------------------------------------------------
// Right Rear Motor
// -----------------------------------------------------
#define RR_EN     26
#define RR_LPWM   12
#define RR_RPWM   13

// -----------------------------------------------------
// Drive Configuration
// -----------------------------------------------------
#define PWM_MAX    255
#define DEADZONE   40
#define SLEW_RATE  5

// -----------------------------------------------------
// Motor Driver Objects
// -----------------------------------------------------
static BTS7960 motorLF(LF_EN, LF_LPWM, LF_RPWM);
static BTS7960 motorLR(LR_EN, LR_LPWM, LR_RPWM);
static BTS7960 motorRF(RF_EN, RF_LPWM, RF_RPWM);
static BTS7960 motorRR(RR_EN, RR_LPWM, RR_RPWM);

// -----------------------------------------------------
// Internal State
// -----------------------------------------------------
static int targetL = 0;
static int targetR = 0;

static int outputL = 0;
static int outputR = 0;

// -----------------------------------------------------
// Deadzone Protection
// -----------------------------------------------------
static int applyDeadzone(int v) {

  // ถ้าค่าอยู่ใกล้กลางเกินไป
  // ให้ถือว่าเป็น 0
  if (abs(v) < DEADZONE) {
    return 0;
  }

  return v;
}

// -----------------------------------------------------
// Slew Limiter
// ป้องกัน PWM เปลี่ยนเร็วเกิน
// และป้องกันกลับทิศทันที
// -----------------------------------------------------
static int slewSafe(int current, int target) {

  // ถ้าจะกลับทิศ
  // ต้องผ่าน 0 ก่อนเสมอ
  if ((current > 0 && target < 0) ||
      (current < 0 && target > 0)) {

    target = 0;
  }

  // ramp up
  if (current < target) {

    current += SLEW_RATE;

    if (current > target) {
      current = target;
    }
  }

  // ramp down
  else if (current > target) {

    current -= SLEW_RATE;

    if (current < target) {
      current = target;
    }
  }

  return current;
}

// -----------------------------------------------------
// Control Motor Pair
// -----------------------------------------------------
static void setMotorPair(
  BTS7960 &m1,
  BTS7960 &m2,
  int pwm
) {

  // จำกัดค่า PWM
  pwm = constrain(pwm, -PWM_MAX, PWM_MAX);

  // เดินหน้า
  if (pwm > 0) {

    m1.TurnRight(pwm);
    m2.TurnRight(pwm);
  }

  // ถอยหลัง
  else if (pwm < 0) {

    m1.TurnLeft(-pwm);
    m2.TurnLeft(-pwm);
  }

  // หยุด
  else {

    m1.Stop();
    m2.Stop();
  }
}

// -----------------------------------------------------
// Initialize Drive Manager
// -----------------------------------------------------
void DriveManager::begin() {

  // enable drivers
  motorLF.Enable();
  motorLR.Enable();
  motorRF.Enable();
  motorRR.Enable();

  // reset targets
  targetL = 0;
  targetR = 0;

  // reset outputs
  outputL = 0;
  outputR = 0;

  // stop all motors
  motorLF.Stop();
  motorLR.Stop();
  motorRF.Stop();
  motorRR.Stop();
}

// -----------------------------------------------------
// Update Drive
// -----------------------------------------------------
void DriveManager::update() {

  // อ่านค่า steering
  int steer = map(
    IBusManager::ch(0),
    1000,
    2000,
    -PWM_MAX,
    PWM_MAX
  );

  // อ่านค่า throttle
  int throttle = map(
    IBusManager::ch(1),
    1000,
    2000,
    -PWM_MAX,
    PWM_MAX
  );

  // apply deadzone
  steer = applyDeadzone(steer);
  throttle = applyDeadzone(throttle);

  // differential mixing
  targetL = constrain(
    throttle + steer,
    -PWM_MAX,
    PWM_MAX
  );

  targetR = constrain(
    throttle - steer,
    -PWM_MAX,
    PWM_MAX
  );

  // apply slew limiter
  outputL = slewSafe(outputL, targetL);
  outputR = slewSafe(outputR, targetR);

  // ส่ง PWM ไปมอเตอร์
  setMotorPair(
    motorLF,
    motorLR,
    outputL
  );

  setMotorPair(
    motorRF,
    motorRR,
    outputR
  );
}

// -----------------------------------------------------
// Controlled Stop
// -----------------------------------------------------
void DriveManager::stop() {

  // target = 0
  targetL = 0;
  targetR = 0;

  // soft stop
  outputL = slewSafe(outputL, 0);
  outputR = slewSafe(outputR, 0);

  // update motors
  setMotorPair(
    motorLF,
    motorLR,
    outputL
  );

  setMotorPair(
    motorRF,
    motorRR,
    outputR
  );
}

// -----------------------------------------------------
// FAILSAFE
// Immediate Stop + Disable Driver
// -----------------------------------------------------
void DriveManager::failsafe() {

  // reset all states
  targetL = 0;
  targetR = 0;

  outputL = 0;
  outputR = 0;

  // stop + disable all drivers
  motorLF.Stop();
  motorLF.Disable();

  motorLR.Stop();
  motorLR.Disable();

  motorRF.Stop();
  motorRF.Disable();

  motorRR.Stop();
  motorRR.Disable();
}