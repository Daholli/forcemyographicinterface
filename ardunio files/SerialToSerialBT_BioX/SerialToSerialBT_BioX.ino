//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

// modified by Christoph Hollizeck - 2022


#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// Adress / Name the Arduino uses to connect to the slave device
uint8_t address[6] = {0xfc, 0xf5, 0xc4, 0x0f, 0x26, 0xee};

// initialize the message as empty
uint8_t message_values[] = {0, 0, 0, 0, 0, 0, 0, 0};

String deviceName = "Arduino BioX";

// setup constants and definitions
bool connected;

#define STARTUP_SIGNAL 234
#define STARTUP_END 255

#define INFO_CONNECTED 250
#define INFO_HANDSHAKE_CONFIRMED 251
#define INFO_CALIBRATION_START 252

#define ERR_NO_STARTSIG 200
#define ERR_NO_CONNECTION 240
#define ERR_NO_ACK 241


void setup()
{
    Serial.begin(115200);

    // wait for startup signal
    while (true)
    {
        if (Serial.available())
        {
            uint8_t message = Serial.read();
            if (message == STARTUP_SIGNAL)
                break;
        }
        else
        {
            message_values[0] = ERR_NO_STARTSIG;
            Serial.write(message_values, 8); // no startup signal received
            delay(500);
        }
    }

    // Serial.println("Starting BluetoothSerial..");
    SerialBT.begin(deviceName, true);

    // Serial.println("The device started in master mode, make sure remote BT device is on!");

    // connect(address) is fast (upto 10 secs max), connect(name) is slow (upto 30 secs max) as it needs
    // to resolve name to address first, but it allows to connect to different devices with the same name.
    // Set CoreDebugLevel to Info to view devices bluetooth address and device names

    // connected = SerialBT.connect(name);
    connected = SerialBT.connect(address);

    if (connected)
    {
        message_values[0] = INFO_CONNECTED;
        Serial.write(message_values, 8);
    }
    else
    {
        while (!SerialBT.connected(10000))
        {
            // Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
            message_values[0] = ERR_NO_CONNECTION;
            Serial.write(message_values, 8);
        }
    }
    // disconnect() may take upto 10 secs max
    if (SerialBT.disconnect())
    {
        continue;
        // Serial.println("Disconnected Successfully!");
    }

    // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
    SerialBT.connect();

    // First part of the handshake
    SerialBT.write('0');

    // wait for the band to confirm the connection and read the response
    while (!SerialBT.available())
        ;
    char ack = SerialBT.read();

    // confirm handshake
    if (ack = '1')
    {
        // Serial.println("Handshake with BioX device confirmed!");
        message_values[0] = INFO_HANDSHAKE_CONFIRMED;
        Serial.write(message_values, 8);
    }
    else
    {
        // Serial.println("No Acknowledgement from BioX band recieved!");
        message_values[0] = ERR_NO_ACK;
        Serial.write(message_values, 8);
    }

    // Serial.println("Calibration will commence in 5 seconds.");
    message_values[0] = INFO_CALIBRATION_START;
    Serial.write(message_values, 8);

    // make first and wait for three seconds
    SerialBT.write('5');
    delay(3000);

    // send the message that the startup is complete and the buffer should be flushed
    message_values[0] = STARTUP_END;
    Serial.write(message_values, 8);
    // flushing buffer
    message_values[0] = 0;
    while (Serial.available() > 0)
    {
        char _ = Serial.read();
    }

    // start transmission of the data from the BioX band
    SerialBT.write('2');
}

void loop()
{
    // check if the main chip wants to send additional commands
    if (Serial.available())
    {
        // send commands to the Bluetooth device
        SerialBT.write(Serial.read());
    }

    // wait for messages from the Bluetooth device
    if (SerialBT.available())
    {
        Serial.write(SerialBT.read());
    }
}
