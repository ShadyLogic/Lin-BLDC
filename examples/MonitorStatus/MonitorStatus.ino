#include <LinBLDC.h>

LinBLDC drive(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(LinBLDC::DefaultBaud);
  drive.setTimeout(300);
}

void loop() {
  uint16_t status = 0;
  uint16_t alarm = 0;
  float temperature = 0.0f;
  float current = 0.0f;

  if (drive.statusCode(status)) {
    Serial.print("Status 0x");
    Serial.print(status, HEX);
    if (status & LinBLDC::StatusEnabled) {
      Serial.print(" enabled");
    }
    if (status & LinBLDC::StatusFault) {
      Serial.print(" fault");
    }
    if (status & LinBLDC::StatusJogging) {
      Serial.print(" jogging");
    }
    if (status & LinBLDC::StatusAlarm) {
      Serial.print(" alarm");
    }
    Serial.println();
  }

  if (drive.alarmCode(alarm) && alarm != 0) {
    Serial.print("Alarm 0x");
    Serial.println(alarm, HEX);
  }

  if (drive.driveTemperature(temperature)) {
    Serial.print("Temp C ");
    Serial.println(temperature, 1);
  }

  if (drive.commandedCurrent(current)) {
    Serial.print("Current A ");
    Serial.println(current, 2);
  }

  delay(1000);
}
