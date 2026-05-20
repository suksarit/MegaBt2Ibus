// =====================================================
// EngineManager.cpp
// TN Mower Engine Controller
//
// Controls:
// - Throttle Servo
// - Ignition Relay
// - Starter Relay
//
// Features:
// - Starter timeout protection
// - Starter re-trigger lock
// - Throttle lock during cranking
// - Immediate shutdown on DISARM / FAILSAFE
//
// IMPORTANT:
// - Configuration อยู่ใน Config.h
// =====================================================

#include <Arduino.h>
#include <Servo.h>

#include "Config.h"
#include "EngineManager.h"
#include "IBusManager.h"

// =====================================================
// Engine State Machine
// =====================================================
enum EngineState {

  // ทุกอย่าง OFF
  ENG_OFF,

  // ignition ON
  // รอสั่ง starter
  ENG_IGNITION_ON,

  // กำลัง crank starter
  ENG_STARTING,

  // engine running
  ENG_RUNNING
};

// =====================================================
// Internal Runtime State
// =====================================================

static EngineState engState = ENG_OFF;

// =====================================================
// Internal Variables
// =====================================================

// เวลาเริ่ม crank starter
static unsigned long starterStartTime = 0;

// กัน trigger starter ซ้ำ
static bool starterLock = false;

// servo object
static Servo throttleServo;

// =====================================================
// Convert RC Signal to Servo Angle
// =====================================================
//
// RC Input:
//   1000 = minimum
//   1500 = center
//   2000 = maximum
//
// Servo Output:
//   0 ... 180 degree
// =====================================================
static int rcToServoAngle(int chValue) {

  // centered around 0
  int centered =
    chValue - RC_CENTER;

  // scale to servo range
  int angle =
    ((centered + RC_RANGE) *
     (THROTTLE_SERVO_MAX - THROTTLE_SERVO_MIN))
    / (RC_RANGE * 2);

  // limit output
  return constrain(
    angle,
    THROTTLE_SERVO_MIN,
    THROTTLE_SERVO_MAX
  );
}

// =====================================================
// Initialize Engine Manager
// =====================================================
void EngineManager::begin() {

  // ---------------------------------------------------
  // attach throttle servo
  // ---------------------------------------------------
  throttleServo.attach(
    SERVO_THROTTLE_PIN
  );

  // throttle idle
  throttleServo.write(
    THROTTLE_IDLE_ANGLE
  );

  // ---------------------------------------------------
  // relay outputs
  // ---------------------------------------------------
  pinMode(RELAY_IGNITION, OUTPUT);
  pinMode(RELAY_STARTER, OUTPUT);

  digitalWrite(RELAY_IGNITION, LOW);
  digitalWrite(RELAY_STARTER, LOW);

  // ---------------------------------------------------
  // reset state
  // ---------------------------------------------------
  engState = ENG_OFF;

  starterLock = false;
}

// =====================================================
// Update Engine State Machine
// =====================================================
//
// IMPORTANT:
// - เรียกเฉพาะ STATE_ACTIVE
// - non-blocking only
// =====================================================
void EngineManager::update() {

  // ---------------------------------------------------
  // Read IBUS Commands
  // ---------------------------------------------------

  // ignition switch
  bool ignitionCmd =
    (IBusManager::ch(
      IBUS_CH_IGNITION
    ) > RC_CENTER);

  // starter switch
  bool starterCmd =
    (IBusManager::ch(
      IBUS_CH_STARTER
    ) > RC_CENTER);

  // ===================================================
  // Engine State Machine
  // ===================================================
  switch (engState) {

    // =================================================
    // ENGINE OFF
    // =================================================
    case ENG_OFF:

      throttleServo.write(
        THROTTLE_IDLE_ANGLE
      );

      digitalWrite(RELAY_STARTER, LOW);
      digitalWrite(RELAY_IGNITION, LOW);

      starterLock = false;

      // ignition ON
      if (ignitionCmd) {

        digitalWrite(
          RELAY_IGNITION,
          HIGH
        );

        engState =
          ENG_IGNITION_ON;
      }

      break;

    // =================================================
    // IGNITION ON
    // =================================================
    case ENG_IGNITION_ON:

      throttleServo.write(
        THROTTLE_IDLE_ANGLE
      );

      digitalWrite(RELAY_STARTER, LOW);

      // ignition OFF
      if (!ignitionCmd) {

        digitalWrite(
          RELAY_IGNITION,
          LOW
        );

        engState = ENG_OFF;
      }

      // start request
      else if (
        starterCmd &&
        !starterLock
      ) {

        digitalWrite(
          RELAY_STARTER,
          HIGH
        );

        starterStartTime =
          millis();

        // lock re-trigger
        starterLock = true;

        engState =
          ENG_STARTING;
      }

      break;

    // =================================================
    // STARTING
    // =================================================
    case ENG_STARTING:

      // กันเร่งระหว่าง crank
      throttleServo.write(
        THROTTLE_IDLE_ANGLE
      );

      // timeout หรือปล่อย starter
      if (
        (millis() - starterStartTime >=
          ENGINE_STARTER_TIMEOUT)
        ||
        !starterCmd
      ) {

        digitalWrite(
          RELAY_STARTER,
          LOW
        );

        // ยังไม่มี RPM confirm
        engState =
          ENG_RUNNING;
      }

      break;

    // =================================================
    // RUNNING
    // =================================================
    case ENG_RUNNING:

      // ignition OFF = shutdown
      if (!ignitionCmd) {

        throttleServo.write(
          THROTTLE_IDLE_ANGLE
        );

        digitalWrite(
          RELAY_IGNITION,
          LOW
        );

        digitalWrite(
          RELAY_STARTER,
          LOW
        );

        engState = ENG_OFF;
      }

      else {

        // ---------------------------------------------
        // allow throttle only when
        // starter released
        // ---------------------------------------------
        if (!starterCmd) {

          int angle =
            rcToServoAngle(
              IBusManager::ch(
                IBUS_CH_ENGINE_THROTTLE
              )
            );

          throttleServo.write(angle);
        }

        // ---------------------------------------------
        // lock throttle during crank
        // ---------------------------------------------
        else {

          throttleServo.write(
            THROTTLE_IDLE_ANGLE
          );
        }
      }

      break;
  }

  // ===================================================
  // Starter Lock Release
  // ===================================================
  //
  // ปลด lock เฉพาะเมื่อ:
  // - ไม่อยู่ใน STARTING
  // - และปล่อยปุ่ม starter แล้ว
  // ===================================================
  if (
    !starterCmd &&
    engState != ENG_STARTING
  ) {

    starterLock = false;
  }
}

// =====================================================
// DISARMED STATE
// =====================================================
void EngineManager::disarmed() {

  throttleServo.write(
    THROTTLE_IDLE_ANGLE
  );

  digitalWrite(RELAY_IGNITION, LOW);
  digitalWrite(RELAY_STARTER, LOW);

  engState = ENG_OFF;

  starterLock = false;
}

// =====================================================
// FAILSAFE STATE
// =====================================================
//
// IMPORTANT:
// - highest priority
// =====================================================
void EngineManager::failsafe() {

  disarmed();
}

