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
// - Servo deadband
// - Servo slew limiter
// - Servo write protection
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
// Servo Runtime State
// =====================================================

// มุมเป้าหมาย
static int targetServoAngle =
  THROTTLE_IDLE_ANGLE;

// มุมปัจจุบัน
static int currentServoAngle =
  THROTTLE_IDLE_ANGLE;

// กัน write ซ้ำ
static int lastServoAngle = -1;

// =====================================================
// Safe Servo Write
// =====================================================
//
// IMPORTANT:
// - write เฉพาะเมื่อ angle เปลี่ยน
// - ลด servo jitter
// - ลด interrupt load
// =====================================================
static void writeServoSafe(int angle) {

  // limit range
  angle = constrain(
    angle,
    THROTTLE_SERVO_MIN,
    THROTTLE_SERVO_MAX
  );

  // write only if changed
  if (angle != lastServoAngle) {

    throttleServo.write(angle);

    lastServoAngle = angle;
  }
}

// =====================================================
// Servo Slew Limiter
// =====================================================
//
// IMPORTANT:
// - ลด throttle jump
// - ลด linkage shock
// =====================================================
static int servoSlew(
  int current,
  int target
) {

  if (current < target) {

    current +=
      THROTTLE_SERVO_SLEW;

    if (current > target) {
      current = target;
    }
  }

  else if (current > target) {

    current -=
      THROTTLE_SERVO_SLEW;

    if (current < target) {
      current = target;
    }
  }

  return current;
}

// =====================================================
// Convert RC Signal to Servo Angle
// =====================================================
static int rcToServoAngle(int chValue) {

  // centered around 0
  int centered =
    chValue - RC_CENTER;

  // ---------------------------------------------------
  // Servo Deadband
  // ---------------------------------------------------
  if (
    abs(centered)
    <
    THROTTLE_DEADBAND
  ) {

    centered = 0;
  }

  // ---------------------------------------------------
  // Scale RC -> Servo
  // ---------------------------------------------------
  int angle =
    (
      (centered + RC_RANGE)
      *
      (
        THROTTLE_SERVO_MAX -
        THROTTLE_SERVO_MIN
      )
    )
    /
    (RC_RANGE * 2);

  // ---------------------------------------------------
  // Limit Output
  // ---------------------------------------------------
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
  // Attach Servo
  // ---------------------------------------------------
  throttleServo.attach(
    SERVO_THROTTLE_PIN
  );

  // ---------------------------------------------------
  // Initialize Servo Position
  // ---------------------------------------------------
  targetServoAngle =
    THROTTLE_IDLE_ANGLE;

  currentServoAngle =
    THROTTLE_IDLE_ANGLE;

  writeServoSafe(
    THROTTLE_IDLE_ANGLE
  );

  // ---------------------------------------------------
  // Relay Outputs
  // ---------------------------------------------------
  pinMode(RELAY_IGNITION, OUTPUT);
  pinMode(RELAY_STARTER, OUTPUT);

  digitalWrite(RELAY_IGNITION, LOW);
  digitalWrite(RELAY_STARTER, LOW);

  // ---------------------------------------------------
  // Reset State
  // ---------------------------------------------------
  engState = ENG_OFF;

  starterLock = false;
}

// =====================================================
// Update Engine State Machine
// =====================================================
void EngineManager::update() {

  // ---------------------------------------------------
  // Read IBUS Commands
  // ---------------------------------------------------

  // ignition switch
  bool ignitionCmd =
    (
      IBusManager::ch(
        IBUS_CH_IGNITION
      )
      >
      RC_CENTER
    );

  // starter switch
  bool starterCmd =
    (
      IBusManager::ch(
        IBUS_CH_STARTER
      )
      >
      RC_CENTER
    );

  // ===================================================
  // Engine State Machine
  // ===================================================
  switch (engState) {

    // =================================================
    // ENGINE OFF
    // =================================================
    case ENG_OFF:

      targetServoAngle =
        THROTTLE_IDLE_ANGLE;

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

      targetServoAngle =
        THROTTLE_IDLE_ANGLE;

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

        starterLock = true;

        engState =
          ENG_STARTING;
      }

      break;

    // =================================================
    // STARTING
    // =================================================
    case ENG_STARTING:

      // lock throttle during crank
      targetServoAngle =
        THROTTLE_IDLE_ANGLE;

      // timeout หรือปล่อย starter
      if (
        (
          millis() -
          starterStartTime
        )
        >=
        ENGINE_STARTER_TIMEOUT
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

      // ignition OFF
      if (!ignitionCmd) {

        targetServoAngle =
          THROTTLE_IDLE_ANGLE;

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

          targetServoAngle =
            rcToServoAngle(
              IBusManager::ch(
                IBUS_CH_ENGINE_THROTTLE
              )
            );
        }

        // ---------------------------------------------
        // lock throttle during crank
        // ---------------------------------------------
        else {

          targetServoAngle =
            THROTTLE_IDLE_ANGLE;
        }
      }

      break;
  }

  // ===================================================
  // Servo Slew Update
  // ===================================================
  currentServoAngle =
    servoSlew(
      currentServoAngle,
      targetServoAngle
    );

  // ===================================================
  // Servo Output Update
  // ===================================================
  writeServoSafe(
    currentServoAngle
  );

  // ===================================================
  // Starter Lock Release
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

  targetServoAngle =
    THROTTLE_IDLE_ANGLE;

  currentServoAngle =
    THROTTLE_IDLE_ANGLE;

  writeServoSafe(
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
void EngineManager::failsafe() {

  disarmed();
}

