// =====================================================
// DriveManager.cpp
// TN Mower Drive Controller
//
// Features:
// - 4x BTS7960 motor drivers
// - Differential drive
// - Centered RC scaling
// - Deadzone filtering
// - Slew-rate limiting
// - Soft stop
// - FAILSAFE stop
//
// IMPORTANT:
// - Configuration ทั้งหมดอยู่ใน Config.h
// =====================================================

#include <Arduino.h>
#include <BTS7960.h>

#include "Config.h"
#include "DriveManager.h"
#include "IBusManager.h"

// =====================================================
// Motor Driver Objects
// =====================================================

static BTS7960 motorLF(LF_EN, LF_LPWM, LF_RPWM);
static BTS7960 motorLR(LR_EN, LR_LPWM, LR_RPWM);

static BTS7960 motorRF(RF_EN, RF_LPWM, RF_RPWM);
static BTS7960 motorRR(RR_EN, RR_LPWM, RR_RPWM);

// =====================================================
// Internal Runtime State
// =====================================================

static int targetL = 0;
static int targetR = 0;

static int outputL = 0;
static int outputR = 0;

// =====================================================
// Deadzone Protection
// =====================================================
static int applyDeadzone(int v) {

  // stick jitter protection
  if (abs(v) < DRIVE_DEADZONE) {
    return 0;
  }

  return v;
}

// =====================================================
// Convert RC Signal to PWM
// =====================================================
static int rcToPwm(int chValue) {

  // centered around 0
  int centered =
    chValue - RC_CENTER;

  // scale to PWM range
  int pwm =
    (centered * PWM_MAX)
    / RC_RANGE;

  // limit output
  return constrain(
    pwm,
    -PWM_MAX,
    PWM_MAX
  );
}

// =====================================================
// Slew Rate Limiter
// =====================================================
static int slewSafe(
  int current,
  int target
) {

  // ---------------------------------------------------
  // direction reversal protection
  // ---------------------------------------------------
  if (
    (current > 0 && target < 0) ||
    (current < 0 && target > 0)
  ) {

    target = 0;
  }

  // ---------------------------------------------------
  // ramp up
  // ---------------------------------------------------
  if (current < target) {

    current +=
      DRIVE_SLEW_RATE;

    if (current > target) {
      current = target;
    }
  }

  // ---------------------------------------------------
  // ramp down
  // ---------------------------------------------------
  else if (current > target) {

    current -=
      DRIVE_SLEW_RATE;

    if (current < target) {
      current = target;
    }
  }

  return current;
}

// =====================================================
// Set Motor Pair
// =====================================================
static void setMotorPair(
  BTS7960 &m1,
  BTS7960 &m2,
  int pwm
) {

  // ---------------------------------------------------
  // limit pwm
  // ---------------------------------------------------
  pwm = constrain(
    pwm,
    -PWM_MAX,
    PWM_MAX
  );

  // ---------------------------------------------------
  // forward
  // ---------------------------------------------------
  if (pwm > 0) {

    m1.TurnRight(pwm);
    m2.TurnRight(pwm);
  }

  // ---------------------------------------------------
  // reverse
  // ---------------------------------------------------
  else if (pwm < 0) {

    m1.TurnLeft(-pwm);
    m2.TurnLeft(-pwm);
  }

  // ---------------------------------------------------
  // stop
  // ---------------------------------------------------
  else {

    m1.Stop();
    m2.Stop();
  }
}

// =====================================================
// Initialize Drive Manager
// =====================================================
void DriveManager::begin() {

  // ---------------------------------------------------
  // enable BTS7960 drivers
  // ---------------------------------------------------
  motorLF.Enable();
  motorLR.Enable();

  motorRF.Enable();
  motorRR.Enable();

  // ---------------------------------------------------
  // reset runtime state
  // ---------------------------------------------------
  targetL = 0;
  targetR = 0;

  outputL = 0;
  outputR = 0;

  // ---------------------------------------------------
  // stop all motors
  // ---------------------------------------------------
  motorLF.Stop();
  motorLR.Stop();

  motorRF.Stop();
  motorRR.Stop();
}

// =====================================================
// Update Drive System
// =====================================================
void DriveManager::update() {

  // ===================================================
  // Ensure BTS7960 Enabled
  //
  // IMPORTANT:
  // - recover after FAILSAFE
  // - prevent driver lockout
  // ===================================================
  motorLF.Enable();
  motorLR.Enable();

  motorRF.Enable();
  motorRR.Enable();

  // ---------------------------------------------------
  // Read RC Channels
  // ---------------------------------------------------
  int rawSteer =
    IBusManager::ch(
      IBUS_CH_STEER
    );

  int rawThrottle =
    IBusManager::ch(
      IBUS_CH_DRIVE
    );

  // ---------------------------------------------------
  // Convert RC to PWM
  // ---------------------------------------------------
  int steer =
    rcToPwm(rawSteer);

  int throttle =
    rcToPwm(rawThrottle);

  // ---------------------------------------------------
  // Apply Deadzone
  // ---------------------------------------------------
  steer =
    applyDeadzone(steer);

  throttle =
    applyDeadzone(throttle);

  // ---------------------------------------------------
  // Differential Mixing
  // ---------------------------------------------------
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

  // ---------------------------------------------------
  // Apply Slew Limiter
  // ---------------------------------------------------
  outputL =
    slewSafe(
      outputL,
      targetL
    );

  outputR =
    slewSafe(
      outputR,
      targetR
    );

  // ---------------------------------------------------
  // Send PWM to Motors
  // ---------------------------------------------------
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

// =====================================================
// Controlled Stop
// =====================================================
void DriveManager::stop() {

  // ---------------------------------------------------
  // reset targets
  // ---------------------------------------------------
  targetL = 0;
  targetR = 0;

  // ---------------------------------------------------
  // soft stop
  // ---------------------------------------------------
  outputL =
    slewSafe(
      outputL,
      0
    );

  outputR =
    slewSafe(
      outputR,
      0
    );

  // ---------------------------------------------------
  // update motors
  // ---------------------------------------------------
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

// =====================================================
// FAILSAFE STOP
// =====================================================
void DriveManager::failsafe() {

  // ---------------------------------------------------
  // reset runtime state
  // ---------------------------------------------------
  targetL = 0;
  targetR = 0;

  outputL = 0;
  outputR = 0;

  // ---------------------------------------------------
  // stop all motors
  // ---------------------------------------------------
  motorLF.Stop();
  motorLR.Stop();

  motorRF.Stop();
  motorRR.Stop();

  // ---------------------------------------------------
  // disable BTS7960
  // ---------------------------------------------------
  motorLF.Disable();
  motorLR.Disable();

  motorRF.Disable();
  motorRR.Disable();
}

