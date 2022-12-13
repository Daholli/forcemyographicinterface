/*
  SerialNINAPassthrough - Use esptool to flash the u-blox NINA (ESP32) module
  Arduino MKR WiFi 1010, Arduino MKR Vidor 4000, and Arduino UNO WiFi Rev.2.

  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// This file was modified by Christoph Hollizeck for use with an MKRWifi1010 to enhance readability and provide the functionality necessary to flash the module.

// set the baudrate to be compatible with the Nina module
unsigned long baud = 115200;

void setup()
{
    Serial.begin(baud);
    SerialNina.begin(baud);

    // set the pins the proper order to signal that following scripts should be uploaded to the Nina module
    pinMode(NINA_GPIO0, OUTPUT);
    pinMode(NINA_RESETN, OUTPUT);
    digitalWrite(NINA_GPIO0, LOW);
    digitalWrite(NINA_RESETN, LOW);
    //delays made it work more consistently
    delay(100);
    digitalWrite(NINA_RESETN, HIGH);
    delay(100);
    digitalWrite(NINA_RESETN, LOW);
}
void loop()
{
    // Read data from the Serial input and write it to the Nina Module
    if (Serial.available())
    {
        SerialNina.write(Serial.read());
    }

    // Read data from the Nina Module and forward it to the Serial output
    if (SerialNina.available())
    {
        Serial.write(SerialNina.read());
    }
}
