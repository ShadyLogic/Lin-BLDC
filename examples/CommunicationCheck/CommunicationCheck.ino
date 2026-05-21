#include <LinBLDC.h>

LinBLDC drive(Serial5);

static void printQuery(const char *label, const char *command)
{
    char response[LinBLDC::ResponseCapacity];

    Serial.print(label);
    Serial.print(" (");
    Serial.print(command);
    Serial.print("): ");

    if (drive.query(command, response, sizeof(response)))
    {
        Serial.println(response);
    }
    else
    {
        Serial.println("no response");
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000)
    {
    }

    Serial5.begin(LinBLDC::DefaultBaud);
    drive.setTimeout(500);

    Serial.println();
    Serial.println("Lin BLDC communication check");
    Serial.println("Drive serial: Serial5 at 9600 baud");
    Serial.println("Expected: model/status/voltage replies once wiring is correct.");
    Serial.println();
}

void loop()
{
    char model[LinBLDC::ResponseCapacity];
    uint16_t status = 0;
    float volts = 0.0f;

    Serial.println("Checking drive...");

    if (drive.modelRevision(model, sizeof(model)))
    {
        Serial.print("PASS model: ");
        Serial.println(model);
    }
    else
    {
        Serial.println("FAIL model: no response to MV");
    }

    if (drive.statusCode(status))
    {
        Serial.print("PASS status: 0x");
        Serial.println(status, HEX);
    }
    else
    {
        Serial.println("FAIL status: no response to SC");
    }

    if (drive.busVoltage(volts))
    {
        Serial.print("PASS bus voltage: ");
        Serial.print(volts, 1);
        Serial.println(" V");
    }
    else
    {
        Serial.println("FAIL bus voltage: no response to IU");
    }

    printQuery("Raw alarm", "AL");
    printQuery("Raw velocity", "IV");

    Serial.println();
    delay(2000);
}
