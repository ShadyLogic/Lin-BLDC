#include <LinBLDC.h>

LinBLDC drive(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(LinBLDC::DefaultBaud);

  drive.setTimeout(300);
  drive.disable();
  drive.resetAlarms();

  drive.setJogAcceleration(10.0f);
  drive.setJogDeceleration(25.0f);
  drive.setJogSpeed(1.0f);
  drive.enable();
  drive.startJog();
}

void loop() {
  static uint32_t lastPrint = 0;

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    int32_t rpm = 0;
    float volts = 0.0f;
    uint16_t status = 0;

    if (drive.velocityRpm(rpm)) {
      Serial.print("RPM: ");
      Serial.println(rpm);
    }
    if (drive.busVoltage(volts)) {
      Serial.print("Bus V: ");
      Serial.println(volts, 1);
    }
    if (drive.statusCode(status)) {
      Serial.print("Status: 0x");
      Serial.println(status, HEX);
    }
  }
}
