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

// Left Front Motor
#define LF_EN     4
#define LF_LPWM   6
#define LF_RPWM   5

// Left Rear Motor
#define LR_EN     22
#define LR_LPWM   24
#define LR_RPWM   23

// Right Front Motor
#define RF_EN     9
#define RF_LPWM   8
#define RF_RPWM   7

// Right Rear Motor
#define RR_EN     26
#define RR_LPWM   28
#define RR_RPWM   27

// -----------------------------------------------------
// Drive Configuration
// -----------------------------------------------------
#define PWM_MAX    255
#define DEADZONE   40
#define SLEW_RATE  5    // PWM step per update()

// -----------------------------------------------------
// Motor Driver Objects (4 Motors)
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
// Utility Functions
// -----------------------------------------------------

// Deadzone protection
static int applyDeadzone(int v) {
  if (abs(v) < DEADZONE) return 0;
  return v;
}

// Slew limiter (no direction flip through zero)
static int slewSafe(int current, int target) {

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

// Control 2 motors as a pair
static void setMotorPair(
  BTS7960 &m1,
  BTS7960 &m2,
  int pwm
) {
  pwm = constrain(pwm, -PWM_MAX, PWM_MAX);

  if (pwm > 0) {
    m1.TurnRight(pwm);
    m2.TurnRight(pwm);
  } else if (pwm < 0) {
    m1.TurnLeft(-pwm);
    m2.TurnLeft(-pwm);
  } else {
    m1.Stop();
    m2.Stop();
  }
}

// -----------------------------------------------------
// Initialize Drive Manager
// -----------------------------------------------------
void DriveManager::begin() {

  motorLF.Enable();
  motorLR.Enable();
  motorRF.Enable();
  motorRR.Enable();

  targetL = 0;
  targetR = 0;
  outputL = 0;
  outputR = 0;

  motorLF.Stop();
  motorLR.Stop();
  motorRF.Stop();
  motorRR.Stop();
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

  setMotorPair(motorLF, motorLR, outputL);
  setMotorPair(motorRF, motorRR, outputR);
}

// -----------------------------------------------------
// Controlled Stop (DISARM)
// -----------------------------------------------------
void DriveManager::stop() {

  targetL = 0;
  targetR = 0;

  outputL = slewSafe(outputL, 0);
  outputR = slewSafe(outputR, 0);

  setMotorPair(motorLF, motorLR, outputL);
  setMotorPair(motorRF, motorRR, outputR);
}

// -----------------------------------------------------
// FAILSAFE (Immediate Stop)
// -----------------------------------------------------
void DriveManager::failsafe() {

  targetL = 0;
  targetR = 0;
  outputL = 0;
  outputR = 0;

  motorLF.Stop(); motorLF.Disable();
  motorLR.Stop(); motorLR.Disable();
  motorRF.Stop(); motorRF.Disable();
  motorRR.Stop(); motorRR.Disable();
}
