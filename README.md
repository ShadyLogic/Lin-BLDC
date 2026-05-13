# Lin-BLDC

Arduino library for Lin Engineering BLDC50/BLDC100 brushless DC motor drives.

The drive host interface is a 9600 baud ASCII serial protocol over RS-232 or RS-485. This library wraps the commands from the BLDC50/BLDC100 manual and the BLDC100 host command guide in a small `Stream`-based API, so it works with Arduino, Teensy, ESP32, and RS-485 adapters.

## Wiring Notes

- BLDC50/BLDC100 RS-232 models use CN5 RX/TX/GND and fixed 9600 baud.
- BLDC51/BLDC101 RS-485 models also use fixed 9600 baud.
- For RS-485 multidrop, call `setAddress()` with the drive address character.
- The library sends commands terminated with carriage return (`\r`) and reads line-style responses such as `SC=0004` or `IV=1000`.

## Basic Use

```cpp
#include <LinBLDC.h>

LinBLDC drive(Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(LinBLDC::DefaultBaud);

  drive.disable();
  drive.resetAlarms();
  drive.setJogAcceleration(10.0f);
  drive.setJogDeceleration(25.0f);
  drive.setJogSpeed(1.0f);
  drive.enable();
  drive.startJog();
}

void loop() {
  int32_t rpm = 0;
  if (drive.velocityRpm(rpm)) {
    Serial.println(rpm);
  }
  delay(500);
}
```

## Common Commands

- `enable()`, `disable()`, `resetAlarms()`, `restart()`
- `setJogSpeed()`, `setJogAcceleration()`, `setJogDeceleration()`
- `startJog()`, `changeJogSpeed()`, `stopJog()`, `stop()`, `softStop()`
- `statusCode()`, `alarmCode()`, `velocityRpm()`, `busVoltage()`
- `driveTemperature()`, `commandedCurrent()`, `inputStatus()`, `outputStatus()`
- `command()` and `query()` for host commands not wrapped yet

## Safety

`enable()` restores motor current and may cause motion if the drive is already configured for analog velocity or external input control. Keep external enable, stop, and direction inputs sequenced correctly before enabling the drive.
