// =====================================================
// EngineManager.cpp
// Controls:
// - Throttle Servo
// - Ignition Relay
// - Starter Relay
//
// Safety Features:
// - Starter timeout
// - Starter re-trigger lock (must release before retry)
// - Throttle disabled until RUNNING
// - Immediate shutdown on DISARM / FAILSAFE
// =====================================================

#include <Arduino.h>
#include <Servo.h>
#include "EngineManager.h"
#include "IBusManager.h"

// -----------------------------------------------------
// Hardware Configuration
// -----------------------------------------------------
#define SERVO_THROTTLE_PIN 9
#define RELAY_IGNITION     22
#define RELAY_STARTER      23

// -----------------------------------------------------
// Timing Configuration
// -----------------------------------------------------
#define STARTER_TIMEOUT    2500UL   // ms

// -----------------------------------------------------
// Engine Internal State
// -----------------------------------------------------
enum EngineState {
  ENG_OFF,
  ENG_IGNITION_ON,
  ENG_STARTING,
  ENG_RUNNING
};

static EngineState engState = ENG_OFF;

// -----------------------------------------------------
// Internal Variables
// -----------------------------------------------------
static unsigned long starterStartTime = 0;
static bool starterLock = false;     // ล็อกการสตาร์ตซ้ำ
static Servo throttleServo;

// -----------------------------------------------------
// Initialize Engine Manager
// -----------------------------------------------------
void EngineManager::begin() {

  throttleServo.attach(SERVO_THROTTLE_PIN);
  throttleServo.write(0);   // throttle ปิดเสมอ

  pinMode(RELAY_IGNITION, OUTPUT);
  pinMode(RELAY_STARTER, OUTPUT);

  digitalWrite(RELAY_IGNITION, LOW);
  digitalWrite(RELAY_STARTER, LOW);

  engState = ENG_OFF;
  starterLock = false;
}

// -----------------------------------------------------
// Update Engine State Machine (ACTIVE only)
// -----------------------------------------------------
void EngineManager::update() {

  // อ่านคำสั่งจาก IBUS
  bool ignitionCmd = (IBusManager::ch(5) > 1500); // CH6
  bool starterCmd  = (IBusManager::ch(6) > 1500); // CH7

  switch (engState) {

    // ================================
    // ENGINE OFF
    // ================================
    case ENG_OFF:

      throttleServo.write(0);
      digitalWrite(RELAY_STARTER, LOW);
      digitalWrite(RELAY_IGNITION, LOW);
      starterLock = false;

      if (ignitionCmd) {
        digitalWrite(RELAY_IGNITION, HIGH);
        engState = ENG_IGNITION_ON;
      }
      break;

    // ================================
    // IGNITION ON (WAIT FOR START)
    // ================================
    case ENG_IGNITION_ON:

      throttleServo.write(0);
      digitalWrite(RELAY_STARTER, LOW);

      if (!ignitionCmd) {
        digitalWrite(RELAY_IGNITION, LOW);
        engState = ENG_OFF;
      }
      else if (starterCmd && !starterLock) {
        digitalWrite(RELAY_STARTER, HIGH);
        starterStartTime = millis();
        starterLock = true;          // ล็อกทันที
        engState = ENG_STARTING;
      }
      break;

    // ================================
    // STARTING (TIME-LIMITED)
    // ================================
    case ENG_STARTING:

      throttleServo.write(0);

      // ครบเวลา หรือ ปล่อยปุ่ม → หยุดสตาร์ต
      if ((millis() - starterStartTime >= STARTER_TIMEOUT) || !starterCmd) {
        digitalWrite(RELAY_STARTER, LOW);
        engState = ENG_RUNNING;      // ignition ยัง ON → อนุญาต RUNNING
      }
      break;

    // ================================
    // RUNNING (NO SENSOR CONFIRM)
    // ================================
    case ENG_RUNNING:

      // ดับ ignition = ดับเครื่อง
      if (!ignitionCmd) {
        throttleServo.write(0);
        digitalWrite(RELAY_IGNITION, LOW);
        digitalWrite(RELAY_STARTER, LOW);
        engState = ENG_OFF;
      }
      else {

        // อนุญาต throttle เฉพาะเมื่อปล่อย starter แล้ว
        if (!starterCmd) {
          int angle = map(IBusManager::ch(2), 1000, 2000, 0, 180);
          throttleServo.write(constrain(angle, 0, 180));
        } else {
          throttleServo.write(0); // กันเร่งระหว่างกด starter
        }
      }
      break;
  }

  // ---------------------------------------------------
  // ปลด starter lock เฉพาะเมื่อ
  // - ไม่ได้อยู่ใน STARTING
  // - และปล่อยปุ่มแล้วจริง
  // ---------------------------------------------------
  if (!starterCmd && engState != ENG_STARTING) {
    starterLock = false;
  }
}

// -----------------------------------------------------
// DISARMED STATE
// -----------------------------------------------------
void EngineManager::disarmed() {

  throttleServo.write(0);
  digitalWrite(RELAY_IGNITION, LOW);
  digitalWrite(RELAY_STARTER, LOW);

  engState = ENG_OFF;
  starterLock = false;
}

// -----------------------------------------------------
// FAILSAFE STATE (HIGHEST PRIORITY)
// -----------------------------------------------------
void EngineManager::failsafe() {
  disarmed();
}
