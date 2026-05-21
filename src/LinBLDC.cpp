#include "LinBLDC.h"

/**
 * @file LinBLDC.cpp
 * @brief Implementation of the Lin Engineering BLDC50/BLDC100 ASCII host command driver.
 *
 * Commands are sent as optional RS-485 address character, ASCII command text, and a carriage return.
 * Responses are parsed as printable ASCII lines, usually in the `KEY=value` form documented by the drive manual.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

LinBLDC::LinBLDC(Stream &serial)
    : _serial(&serial), _address('\0'), _timeoutMs(250)
{
    _lastResponse[0] = '\0';
}

void LinBLDC::setAddress(char address)
{
    _address = address;
}

void LinBLDC::clearAddress()
{
    _address = '\0';
}

char LinBLDC::address() const
{
    return _address;
}

void LinBLDC::setTimeout(uint16_t timeoutMs)
{
    _timeoutMs = timeoutMs;
}

uint16_t LinBLDC::timeout() const
{
    return _timeoutMs;
}

bool LinBLDC::command(const __FlashStringHelper *command)
{
    char buffer[24];
    strncpy_P(buffer, reinterpret_cast<PGM_P>(command), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return this->command(buffer);
}

bool LinBLDC::command(const char *command)
{
    return sendCommand(command);
}

bool LinBLDC::query(const __FlashStringHelper *command, char *response, size_t responseSize)
{
    char buffer[24];
    strncpy_P(buffer, reinterpret_cast<PGM_P>(command), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return query(buffer, response, responseSize);
}

bool LinBLDC::query(const char *command, char *response, size_t responseSize)
{
    if (!sendCommand(command))
    {
        return false;
    }
    return readResponse(response, responseSize);
}

bool LinBLDC::getValue(const char *command, char *value, size_t valueSize)
{
    char response[ResponseCapacity];
    if (!query(command, response, sizeof(response)))
    {
        return false;
    }
    return responseValue(response, value, valueSize);
}

bool LinBLDC::getFloat(const char *command, float &value)
{
    char text[ResponseCapacity];
    if (!getValue(command, text, sizeof(text)))
    {
        return false;
    }
    value = atof(text);
    return true;
}

bool LinBLDC::getInt(const char *command, int32_t &value)
{
    char text[ResponseCapacity];
    if (!getValue(command, text, sizeof(text)))
    {
        return false;
    }
    value = atol(text);
    return true;
}

bool LinBLDC::getHex16(const char *command, uint16_t &value)
{
    char text[ResponseCapacity];
    if (!getValue(command, text, sizeof(text)))
    {
        return false;
    }
    value = static_cast<uint16_t>(strtoul(text, nullptr, 16));
    return true;
}

bool LinBLDC::setAnalogDeadband(uint16_t millivolts)
{
    return formatIntCommand("AD", millivolts);
}

bool LinBLDC::analogDeadband(uint16_t &millivolts)
{
    int32_t value = 0;
    if (!getInt("AD", value))
    {
        return false;
    }
    millivolts = static_cast<uint16_t>(value);
    return true;
}

bool LinBLDC::setAnalogFilter(uint16_t filterValue)
{
    return formatIntCommand("AF", filterValue);
}

bool LinBLDC::analogFilter(uint16_t &filterValue)
{
    int32_t value = 0;
    if (!getInt("AF", value))
    {
        return false;
    }
    filterValue = static_cast<uint16_t>(value);
    return true;
}

uint16_t LinBLDC::analogFilterValueForHz(float hz)
{
    if (hz <= 0.0f)
    {
        return 0;
    }
    const float value = 72090.0f / ((3500.0f / hz) + 2.2f);
    return static_cast<uint16_t>(value + 0.5f);
}

bool LinBLDC::setAnalogOffset(uint16_t millivolts)
{
    return formatIntCommand("AV", millivolts);
}

bool LinBLDC::analogOffset(uint16_t &millivolts)
{
    int32_t value = 0;
    if (!getInt("AV", value))
    {
        return false;
    }
    millivolts = static_cast<uint16_t>(value);
    return true;
}

bool LinBLDC::setContinuousCurrent(float ampsRms)
{
    return formatFloatCommand("CC", ampsRms);
}

bool LinBLDC::continuousCurrent(float &ampsRms)
{
    return getFloat("CC", ampsRms);
}

bool LinBLDC::enable()
{
    return command("ME");
}

bool LinBLDC::disable()
{
    return command("MD");
}

bool LinBLDC::resetAlarms()
{
    return command("AR");
}

bool LinBLDC::restart()
{
    return command("RE");
}

bool LinBLDC::setJogAcceleration(float rpsPerSecond)
{
    return formatFloatCommand("JA", rpsPerSecond);
}

bool LinBLDC::jogAcceleration(float &rpsPerSecond)
{
    return getFloat("JA", rpsPerSecond);
}

bool LinBLDC::setJogDeceleration(float rpsPerSecond)
{
    return formatFloatCommand("JL", rpsPerSecond);
}

bool LinBLDC::jogDeceleration(float &rpsPerSecond)
{
    return getFloat("JL", rpsPerSecond);
}

bool LinBLDC::setJogSpeed(float rps)
{
    return formatFloatCommand("JS", rps);
}

bool LinBLDC::jogSpeed(float &rps)
{
    return getFloat("JS", rps);
}

bool LinBLDC::startJog()
{
    return command("CJ");
}

bool LinBLDC::stopJog()
{
    return command("SJ");
}

bool LinBLDC::changeJogSpeed(float rps)
{
    return formatFloatCommand("CS", rps);
}

bool LinBLDC::stop()
{
    return command("ST");
}

bool LinBLDC::softStop()
{
    return command("STD");
}

bool LinBLDC::setStopMode(uint8_t mode)
{
    return formatIntCommand("SM", mode);
}

bool LinBLDC::stopMode(uint8_t &mode)
{
    int32_t value = 0;
    if (!getInt("SM", value))
    {
        return false;
    }
    mode = static_cast<uint8_t>(value);
    return true;
}

bool LinBLDC::setVelocityErrorRange(uint16_t value)
{
    return formatIntCommand("VR", value);
}

bool LinBLDC::velocityErrorRange(uint16_t &value)
{
    int32_t parsed = 0;
    if (!getInt("VR", parsed))
    {
        return false;
    }
    value = static_cast<uint16_t>(parsed);
    return true;
}

bool LinBLDC::modelRevision(char *model, size_t modelSize)
{
    return query("MV", model, modelSize);
}

bool LinBLDC::statusCode(uint16_t &status)
{
    return getHex16("SC", status);
}

bool LinBLDC::alarmCode(uint16_t &alarm)
{
    return getHex16("AL", alarm);
}

bool LinBLDC::inputStatus(char *bits, size_t bitsSize)
{
    return getValue("IS", bits, bitsSize);
}

bool LinBLDC::outputStatus(char *bits, size_t bitsSize)
{
    return getValue("IO", bits, bitsSize);
}

bool LinBLDC::setOutputs(uint8_t bitPattern)
{
    return formatIntCommand("IO", bitPattern);
}

bool LinBLDC::busVoltage(float &volts)
{
    int32_t tenths = 0;
    if (!getInt("IU", tenths))
    {
        return false;
    }
    volts = tenths / 10.0f;
    return true;
}

bool LinBLDC::driveTemperature(float &celsius)
{
    int32_t tenths = 0;
    if (!getInt("IT", tenths))
    {
        return false;
    }
    celsius = tenths / 10.0f;
    return true;
}

bool LinBLDC::commandedCurrent(float &ampsRms)
{
    return getFloat("IC", ampsRms);
}

bool LinBLDC::velocityRpm(int32_t &rpm)
{
    return getInt("IV", rpm);
}

bool LinBLDC::analogInput(float &voltsOrCommand, int8_t input)
{
    if (input < 0)
    {
        return getFloat("IA", voltsOrCommand);
    }
    char commandText[7];
    snprintf(commandText, sizeof(commandText), "IA%d", input);
    return getFloat(commandText, voltsOrCommand);
}

bool LinBLDC::sendCommand(const char *commandText)
{
    if (_serial == nullptr || commandText == nullptr || commandText[0] == '\0')
    {
        return false;
    }

    // Remove stale bytes so a read following this command belongs to the latest query.
    flushInput();
    if (_address != '\0')
    {
        _serial->write(_address);
    }
    _serial->print(commandText);
    _serial->write('\r');
    _serial->flush();
    return true;
}

bool LinBLDC::readResponse(char *response, size_t responseSize)
{
    if (response == nullptr || responseSize == 0)
    {
        return false;
    }

    response[0] = '\0';
    const uint32_t start = millis();
    size_t index = 0;

    while ((millis() - start) < _timeoutMs)
    {
        while (_serial->available() > 0)
        {
            const char c = static_cast<char>(_serial->read());
            // The manuals show ASCII responses; accept either CR or LF as a line terminator.
            if (c == '\r' || c == '\n')
            {
                if (index == 0)
                {
                    continue;
                }
                response[index] = '\0';
                strncpy(_lastResponse, response, sizeof(_lastResponse) - 1);
                _lastResponse[sizeof(_lastResponse) - 1] = '\0';
                return true;
            }
            if (isprint(static_cast<unsigned char>(c)) && index < responseSize - 1)
            {
                response[index++] = c;
            }
        }
    }

    response[index] = '\0';
    return index > 0;
}

bool LinBLDC::responseValue(const char *response, char *value, size_t valueSize) const
{
    if (response == nullptr || value == nullptr || valueSize == 0)
    {
        return false;
    }

    const char *equals = strchr(response, '=');
    // Raw responses like MV may not include KEY=, so the whole response is considered the value.
    const char *start = equals == nullptr ? response : equals + 1;
    while (*start == ' ')
    {
        ++start;
    }

    size_t index = 0;
    while (start[index] != '\0' && index < valueSize - 1)
    {
        value[index] = start[index];
        ++index;
    }
    value[index] = '\0';
    return value[0] != '\0';
}

bool LinBLDC::formatFloatCommand(const char *prefix, float value)
{
    char buffer[24];
    char number[16];
    dtostrf(value, 0, 3, number);

    char *start = number;
    while (*start == ' ')
    {
        ++start;
    }

    snprintf(buffer, sizeof(buffer), "%s%s", prefix, start);
    return command(buffer);
}

bool LinBLDC::formatIntCommand(const char *prefix, int32_t value)
{
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "%s%ld", prefix, static_cast<long>(value));
    return command(buffer);
}

void LinBLDC::flushInput()
{
    while (_serial->available() > 0)
    {
        _serial->read();
    }
}
