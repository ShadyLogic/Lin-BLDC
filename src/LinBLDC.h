#pragma once

#include <Arduino.h>

/**
 * @file LinBLDC.h
 * @brief Arduino Stream-based driver for Lin Engineering BLDC50/BLDC100 drives.
 */

/**
 * @brief Controls a Lin Engineering BLDC50/BLDC100 motor drive over its ASCII host command interface.
 *
 * The drive uses 9600 baud ASCII commands over either RS-232 or RS-485, depending on the hardware model.
 * This class does not own or configure the serial port; call `SerialX.begin(LinBLDC::DefaultBaud)` before use.
 *
 * For BLDC50/BLDC100 RS-232 models, use a true RS-232 level shifter between the MCU UART and the drive.
 * For RS-485 multidrop models, call setAddress() with the single-character drive address.
 */
class LinBLDC
{
public:
    /**
     * @brief Bit masks returned by statusCode().
     */
    enum Status : uint16_t
    {
        /// Drive is enabled.
        StatusEnabled = 0x0001,
        /// Drive is faulted.
        StatusFault = 0x0004,
        /// Drive is jogging.
        StatusJogging = 0x0020,
        /// Drive is stopping.
        StatusStopping = 0x0040,
        /// Drive has an alarm condition.
        StatusAlarm = 0x0200
    };

    /**
     * @brief Bit masks returned by alarmCode().
     */
    enum Alarm : uint16_t
    {
        /// Main board temperature exceeded the drive limit.
        AlarmOverTemperature = 0x0008,
        /// Internal voltage alarm.
        AlarmInternalVoltage = 0x0010,
        /// DC bus voltage is too high.
        AlarmVoltageHigh = 0x0020,
        /// DC bus voltage is too low.
        AlarmVoltageLow = 0x0040,
        /// Motor phase current exceeded the drive limit.
        AlarmOverCurrent = 0x0080,
        /// Motor phase/open winding fault.
        AlarmOpenWinding = 0x0100,
        /// Hall sensor signal fault.
        AlarmHallBad = 0x0200,
        /// Serial communication error.
        AlarmCommError = 0x0400,
        /// Non-volatile save failed.
        AlarmSaveFailed = 0x0800,
        /// Drive disabled alarm.
        AlarmDriveDisabled = 0x1000,
        /// Current limit alarm.
        AlarmCurrentLimit = 0x2000,
        /// Non-volatile memory error.
        AlarmNvMemoryError = 0x4000
    };

    /**
     * @brief Fixed serial baud rate documented for BLDC50/BLDC100 RS-232 and RS-485 models.
     */
    static const uint32_t DefaultBaud = 9600;

    /**
     * @brief Default response buffer size used by the library examples and internal queries.
     */
    static const size_t ResponseCapacity = 48;

    /**
     * @brief Construct a drive interface on an existing Arduino Stream.
     * @param serial Serial stream connected to the drive.
     */
    explicit LinBLDC(Stream &serial);

    /**
     * @brief Set the RS-485 address prefix used before each command.
     * @param address Single printable drive address character.
     *
     * RS-232 models do not use an address prefix.
     */
    void setAddress(char address);

    /**
     * @brief Stop prefixing commands with an RS-485 address.
     */
    void clearAddress();

    /**
     * @brief Get the configured RS-485 address prefix.
     * @return Address character, or `'\0'` when no address is configured.
     */
    char address() const;

    /**
     * @brief Set the maximum time to wait for a command response.
     * @param timeoutMs Timeout in milliseconds.
     */
    void setTimeout(uint16_t timeoutMs);

    /**
     * @brief Get the current response timeout.
     * @return Timeout in milliseconds.
     */
    uint16_t timeout() const;

    /**
     * @brief Send a write-only command stored in flash.
     * @param command Command text without address prefix or carriage return.
     * @return true if the command was written to the serial stream.
     */
    bool command(const __FlashStringHelper *command);

    /**
     * @brief Send a write-only command.
     * @param command Command text without address prefix or carriage return.
     * @return true if the command was written to the serial stream.
     */
    bool command(const char *command);

    /**
     * @brief Send a read command stored in flash and collect the raw response.
     * @param command Command text without address prefix or carriage return.
     * @param response Destination buffer for the printable response.
     * @param responseSize Size of `response`, including space for the terminator.
     * @return true if any non-empty response was received before timeout.
     */
    bool query(const __FlashStringHelper *command, char *response, size_t responseSize);

    /**
     * @brief Send a read command and collect the raw response.
     * @param command Command text without address prefix or carriage return.
     * @param response Destination buffer for the printable response.
     * @param responseSize Size of `response`, including space for the terminator.
     * @return true if any non-empty response was received before timeout.
     */
    bool query(const char *command, char *response, size_t responseSize);

    /**
     * @brief Query a command and return only the value after `=`.
     * @param command Read command such as `"SC"` or `"IU"`.
     * @param value Destination buffer for the value text.
     * @param valueSize Size of `value`, including space for the terminator.
     * @return true if a non-empty value was parsed.
     */
    bool getValue(const char *command, char *value, size_t valueSize);

    /**
     * @brief Query a command and parse the value as floating-point text.
     * @param command Read command.
     * @param value Parsed floating-point value.
     * @return true if a value was received and parsed.
     */
    bool getFloat(const char *command, float &value);

    /**
     * @brief Query a command and parse the value as signed decimal integer text.
     * @param command Read command.
     * @param value Parsed integer value.
     * @return true if a value was received and parsed.
     */
    bool getInt(const char *command, int32_t &value);

    /**
     * @brief Query a command and parse the value as a 16-bit hexadecimal word.
     * @param command Read command.
     * @param value Parsed hex value.
     * @return true if a value was received and parsed.
     */
    bool getHex16(const char *command, uint16_t &value);

    /**
     * @brief Set analog input deadband with the `AD` command.
     * @param millivolts Deadband in millivolts.
     * @return true if the command was written.
     */
    bool setAnalogDeadband(uint16_t millivolts);

    /**
     * @brief Read analog input deadband with the `AD` command.
     * @param millivolts Receives deadband in millivolts.
     * @return true if the value was read.
     */
    bool analogDeadband(uint16_t &millivolts);

    /**
     * @brief Set analog input filter value with the `AF` command.
     * @param filterValue Raw filter value documented by the drive manual.
     * @return true if the command was written.
     */
    bool setAnalogFilter(uint16_t filterValue);

    /**
     * @brief Read analog input filter value with the `AF` command.
     * @param filterValue Receives raw filter value.
     * @return true if the value was read.
     */
    bool analogFilter(uint16_t &filterValue);

    /**
     * @brief Convert a desired analog filter bandwidth to the drive's `AF` parameter.
     * @param hz Desired filter bandwidth in hertz. Values less than or equal to zero return zero.
     * @return Rounded raw `AF` filter value.
     */
    static uint16_t analogFilterValueForHz(float hz);

    /**
     * @brief Set analog input offset with the `AV` command.
     * @param millivolts Offset in millivolts.
     * @return true if the command was written.
     */
    bool setAnalogOffset(uint16_t millivolts);

    /**
     * @brief Read analog input offset with the `AV` command.
     * @param millivolts Receives offset in millivolts.
     * @return true if the value was read.
     */
    bool analogOffset(uint16_t &millivolts);

    /**
     * @brief Set maximum continuous RMS current with the `CC` command.
     * @param ampsRms Continuous current in amps RMS.
     * @return true if the command was written.
     */
    bool setContinuousCurrent(float ampsRms);

    /**
     * @brief Read maximum continuous RMS current with the `CC` command.
     * @param ampsRms Receives current in amps RMS.
     * @return true if the value was read.
     */
    bool continuousCurrent(float &ampsRms);

    /**
     * @brief Enable motor current with the `ME` command.
     * @return true if the command was written.
     *
     * Enabling may allow motion immediately if external or analog controls are active.
     */
    bool enable();

    /**
     * @brief Disable motor current with the `MD` command.
     * @return true if the command was written.
     */
    bool disable();

    /**
     * @brief Clear drive alarms with the `AR` command.
     * @return true if the command was written.
     */
    bool resetAlarms();

    /**
     * @brief Restart/reinitialize the drive with the `RE` command.
     * @return true if the command was written.
     */
    bool restart();

    /**
     * @brief Set jog acceleration with the `JA` command.
     * @param rpsPerSecond Acceleration in revolutions per second per second.
     * @return true if the command was written.
     */
    bool setJogAcceleration(float rpsPerSecond);

    /**
     * @brief Read jog acceleration with the `JA` command.
     * @param rpsPerSecond Receives acceleration in revolutions per second per second.
     * @return true if the value was read.
     */
    bool jogAcceleration(float &rpsPerSecond);

    /**
     * @brief Set jog deceleration with the `JL` command.
     * @param rpsPerSecond Deceleration in revolutions per second per second.
     * @return true if the command was written.
     */
    bool setJogDeceleration(float rpsPerSecond);

    /**
     * @brief Read jog deceleration with the `JL` command.
     * @param rpsPerSecond Receives deceleration in revolutions per second per second.
     * @return true if the value was read.
     */
    bool jogDeceleration(float &rpsPerSecond);

    /**
     * @brief Set jog speed with the `JS` command.
     * @param rps Speed in revolutions per second. Multiply by 60 for RPM.
     * @return true if the command was written.
     */
    bool setJogSpeed(float rps);

    /**
     * @brief Read jog speed with the `JS` command.
     * @param rps Receives speed in revolutions per second.
     * @return true if the value was read.
     */
    bool jogSpeed(float &rps);

    /**
     * @brief Start jogging with the `CJ` command.
     * @return true if the command was written.
     */
    bool startJog();

    /**
     * @brief Stop jogging with the `SJ` command.
     * @return true if the command was written.
     */
    bool stopJog();

    /**
     * @brief Change jog speed while jogging with the `CS` command.
     * @param rps New speed in revolutions per second.
     * @return true if the command was written.
     */
    bool changeJogSpeed(float rps);

    /**
     * @brief Stop motion with regenerative braking using the `ST` command.
     * @return true if the command was written.
     */
    bool stop();

    /**
     * @brief Stop motion according to the configured stop mode using the `STD` command.
     * @return true if the command was written.
     */
    bool softStop();

    /**
     * @brief Set soft stop mode with the `SM` command.
     * @param mode Stop mode value documented by the manual.
     * @return true if the command was written.
     */
    bool setStopMode(uint8_t mode);

    /**
     * @brief Read soft stop mode with the `SM` command.
     * @param mode Receives stop mode value.
     * @return true if the value was read.
     */
    bool stopMode(uint8_t &mode);

    /**
     * @brief Set velocity error range with the `VR` command.
     * @param value Raw velocity error range value.
     * @return true if the command was written.
     */
    bool setVelocityErrorRange(uint16_t value);

    /**
     * @brief Read velocity error range with the `VR` command.
     * @param value Receives raw velocity error range value.
     * @return true if the value was read.
     */
    bool velocityErrorRange(uint16_t &value);

    /**
     * @brief Read model and firmware string with the `MV` command.
     * @param model Destination buffer for the raw model/revision response.
     * @param modelSize Size of `model`, including space for the terminator.
     * @return true if a response was received.
     */
    bool modelRevision(char *model, size_t modelSize);

    /**
     * @brief Read the hexadecimal status word with the `SC` command.
     * @param status Receives ORed Status bit masks.
     * @return true if the value was read.
     */
    bool statusCode(uint16_t &status);

    /**
     * @brief Read the hexadecimal alarm word with the `AL` command.
     * @param alarm Receives ORed Alarm bit masks.
     * @return true if the value was read.
     */
    bool alarmCode(uint16_t &alarm);

    /**
     * @brief Read digital input status with the `IS` command.
     * @param bits Destination buffer for the returned bit string.
     * @param bitsSize Size of `bits`, including space for the terminator.
     * @return true if the value was read.
     */
    bool inputStatus(char *bits, size_t bitsSize);

    /**
     * @brief Read digital output status with the `IO` command.
     * @param bits Destination buffer for the returned bit string.
     * @param bitsSize Size of `bits`, including space for the terminator.
     * @return true if the value was read.
     */
    bool outputStatus(char *bits, size_t bitsSize);

    /**
     * @brief Set digital outputs with the `IO` command.
     * @param bitPattern Decimal equivalent of the output bit pattern.
     * @return true if the command was written.
     */
    bool setOutputs(uint8_t bitPattern);

    /**
     * @brief Read DC bus voltage with the `IU` command.
     * @param volts Receives bus voltage in volts.
     * @return true if the value was read.
     */
    bool busVoltage(float &volts);

    /**
     * @brief Read drive temperature with the `IT` command.
     * @param celsius Receives main board temperature in degrees Celsius.
     * @return true if the value was read.
     */
    bool driveTemperature(float &celsius);

    /**
     * @brief Read commanded RMS current with the `IC` command.
     * @param ampsRms Receives commanded current in amps RMS.
     * @return true if the value was read.
     */
    bool commandedCurrent(float &ampsRms);

    /**
     * @brief Read immediate motor velocity with the `IV` command.
     * @param rpm Receives velocity in RPM.
     * @return true if the value was read.
     */
    bool velocityRpm(int32_t &rpm);

    /**
     * @brief Read immediate analog command or raw analog input with the `IA` command.
     * @param voltsOrCommand Receives analog command/raw input value as reported by the drive.
     * @param input Optional raw input index. Pass -1 for the default analog command.
     * @return true if the value was read.
     */
    bool analogInput(float &voltsOrCommand, int8_t input = -1);

private:
    /// Serial stream connected to the drive.
    Stream *_serial;
    /// Optional RS-485 address prefix, or `'\0'` for no prefix.
    char _address;
    /// Response timeout in milliseconds.
    uint16_t _timeoutMs;
    /// Last complete printable response captured by readResponse().
    char _lastResponse[ResponseCapacity];

    /**
     * @brief Send the command prefix, ASCII command text, and carriage return terminator.
     */
    bool sendCommand(const char *command);

    /**
     * @brief Read a printable response line until CR/LF or timeout.
     */
    bool readResponse(char *response, size_t responseSize);

    /**
     * @brief Copy the value portion from a raw `KEY=value` response.
     */
    bool responseValue(const char *response, char *value, size_t valueSize) const;

    /**
     * @brief Format a command with a floating-point parameter.
     */
    bool formatFloatCommand(const char *prefix, float value);

    /**
     * @brief Format a command with an integer parameter.
     */
    bool formatIntCommand(const char *prefix, int32_t value);

    /**
     * @brief Discard pending bytes before starting a new command.
     */
    void flushInput();
};
