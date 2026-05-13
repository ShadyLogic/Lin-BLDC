#pragma once

#include <Arduino.h>

class LinBLDC {
public:
  enum Status : uint16_t {
    StatusEnabled = 0x0001,
    StatusFault = 0x0004,
    StatusJogging = 0x0020,
    StatusStopping = 0x0040,
    StatusAlarm = 0x0200
  };

  enum Alarm : uint16_t {
    AlarmOverTemperature = 0x0008,
    AlarmInternalVoltage = 0x0010,
    AlarmVoltageHigh = 0x0020,
    AlarmVoltageLow = 0x0040,
    AlarmOverCurrent = 0x0080,
    AlarmOpenWinding = 0x0100,
    AlarmHallBad = 0x0200,
    AlarmCommError = 0x0400,
    AlarmSaveFailed = 0x0800,
    AlarmDriveDisabled = 0x1000,
    AlarmCurrentLimit = 0x2000,
    AlarmNvMemoryError = 0x4000
  };

  static const uint32_t DefaultBaud = 9600;
  static const size_t ResponseCapacity = 48;

  explicit LinBLDC(Stream &serial);

  void setAddress(char address);
  void clearAddress();
  char address() const;

  void setTimeout(uint16_t timeoutMs);
  uint16_t timeout() const;

  bool command(const __FlashStringHelper *command);
  bool command(const char *command);
  bool query(const __FlashStringHelper *command, char *response, size_t responseSize);
  bool query(const char *command, char *response, size_t responseSize);

  bool getValue(const char *command, char *value, size_t valueSize);
  bool getFloat(const char *command, float &value);
  bool getInt(const char *command, int32_t &value);
  bool getHex16(const char *command, uint16_t &value);

  bool setAnalogDeadband(uint16_t millivolts);
  bool analogDeadband(uint16_t &millivolts);
  bool setAnalogFilter(uint16_t filterValue);
  bool analogFilter(uint16_t &filterValue);
  static uint16_t analogFilterValueForHz(float hz);
  bool setAnalogOffset(uint16_t millivolts);
  bool analogOffset(uint16_t &millivolts);

  bool setContinuousCurrent(float ampsRms);
  bool continuousCurrent(float &ampsRms);

  bool enable();
  bool disable();
  bool resetAlarms();
  bool restart();

  bool setJogAcceleration(float rpsPerSecond);
  bool jogAcceleration(float &rpsPerSecond);
  bool setJogDeceleration(float rpsPerSecond);
  bool jogDeceleration(float &rpsPerSecond);
  bool setJogSpeed(float rps);
  bool jogSpeed(float &rps);
  bool startJog();
  bool stopJog();
  bool changeJogSpeed(float rps);
  bool stop();
  bool softStop();

  bool setStopMode(uint8_t mode);
  bool stopMode(uint8_t &mode);
  bool setVelocityErrorRange(uint16_t value);
  bool velocityErrorRange(uint16_t &value);

  bool modelRevision(char *model, size_t modelSize);
  bool statusCode(uint16_t &status);
  bool alarmCode(uint16_t &alarm);
  bool inputStatus(char *bits, size_t bitsSize);
  bool outputStatus(char *bits, size_t bitsSize);
  bool setOutputs(uint8_t bitPattern);
  bool busVoltage(float &volts);
  bool driveTemperature(float &celsius);
  bool commandedCurrent(float &ampsRms);
  bool velocityRpm(int32_t &rpm);
  bool analogInput(float &voltsOrCommand, int8_t input = -1);

private:
  Stream *_serial;
  char _address;
  uint16_t _timeoutMs;
  char _lastResponse[ResponseCapacity];

  bool sendCommand(const char *command);
  bool readResponse(char *response, size_t responseSize);
  bool responseValue(const char *response, char *value, size_t valueSize) const;
  bool formatFloatCommand(const char *prefix, float value);
  bool formatIntCommand(const char *prefix, int32_t value);
  void flushInput();
};
