
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// An implementation for the Plantower PMS7003 air quality sensor
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
    Copyright (c) 2016 Martin F. Falatic
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "particulates-pms7003.h"

ParticulatesSensorPMS7003::ParticulatesSensorPMS7003()
{
}

char * ParticulatesSensorPMS7003::snprintfData(char* printbuf, int len) {
    printbuf[0] = 0;
    snprintf(printbuf, len, "%s[%02x %02x] (%04x) ", printbuf,
        currFrame.frameHeader[0], currFrame.frameHeader[1], currFrame.frameLen);
    snprintf(printbuf, len, "%sCF1=[%04x %04x %04x] ", printbuf,
        currFrame.concPM1_0_CF1, currFrame.concPM2_5_CF1, currFrame.concPM10_0_CF1);
    snprintf(printbuf, len, "%samb=[%04x %04x %04x] ", printbuf,
        currFrame.concPM1_0_amb, currFrame.concPM2_5_amb, currFrame.concPM10_0_amb);
    snprintf(printbuf, len, "%sraw=[%04x %04x %04x %04x %04x %04x] ", printbuf,
        currFrame.rawGt0_3um, currFrame.rawGt0_5um, currFrame.rawGt1_0um,
        currFrame.rawGt2_5um, currFrame.rawGt5_0um, currFrame.rawGt10_0um);
    snprintf(printbuf, len, "%sver=%02x err=%02x ", printbuf,
        currFrame.version, currFrame.errorCode);
    snprintf(printbuf, len, "%scsum=%04x %s xsum=%04x", printbuf,
        currFrame.checksum, (calcChecksum == currFrame.checksum ? "==" : "!="), calcChecksum);
    return printbuf;
}


bool ParticulatesSensorPMS7003::readData() {
    if (DEBUG) {
        Serial.println("-- Reading PMS7003");
    }
    Serial1.begin(9600);
    bool packetReceived = false;
    while (!packetReceived) {
        if (Serial1.available() > 32) {
            int drain = Serial1.available();
            if (DEBUG) {
                Serial.print("-- Draining buffer: ");
                Serial.println(Serial1.available(), DEC);
            }
            for (int i = drain; i > 0; i--) {
                Serial1.read();
            }
        }
        if (Serial1.available() > 0) {
            if (DEBUG) {
                Serial.print("-- Available: ");
                Serial.println(Serial1.available(), DEC);
            }
            incomingByte = Serial1.read();
            if (DEBUG) {
                Serial.print("-- READ: ");
                Serial.println(incomingByte, HEX);
            }
            if (!inFrame) {
                if (incomingByte == 0x42 && detectOff == 0) {
                    frameBuf[detectOff] = incomingByte;
                    currFrame.frameHeader[0] = incomingByte;
                    calcChecksum = incomingByte; // Checksum init!
                    detectOff++;
                }
                else if (incomingByte == 0x4D && detectOff == 1) {
                    frameBuf[detectOff] = incomingByte;
                    currFrame.frameHeader[1] = incomingByte;
                    calcChecksum += incomingByte;
                    inFrame = true;
                    detectOff++;
                }
                else {
                    if (DEBUG) {
                        Serial.print("-- Frame syncing... ");
                        Serial.print(incomingByte, HEX);
                        Serial.println();
                    }
                }
            }
            else {
                frameBuf[detectOff] = incomingByte;
                calcChecksum += incomingByte;
                detectOff++;
                uint16_t val = frameBuf[detectOff-1]+(frameBuf[detectOff-2]<<8);
                switch (detectOff) {
                    case 4:
                        currFrame.frameLen = val;
                        frameLen = val + detectOff;
                        break;
                    case 6:
                        currFrame.concPM1_0_CF1 = val;
                        break;
                    case 8:
                        currFrame.concPM2_5_CF1 = val;
                        break;
                    case 10:
                        currFrame.concPM10_0_CF1 = val;
                        break;
                    case 12:
                        currFrame.concPM1_0_amb = val;
                        break;
                    case 14:
                        currFrame.concPM2_5_amb = val;
                        break;
                    case 16:
                        currFrame.concPM10_0_amb = val;
                        break;
                    case 18:
                        currFrame.rawGt0_3um = val;
                        break;
                    case 20:
                        currFrame.rawGt0_5um = val;
                        break;
                    case 22:
                        currFrame.rawGt1_0um = val;
                        break;
                    case 24:
                        currFrame.rawGt2_5um = val;
                        break;
                    case 26:
                        currFrame.rawGt5_0um = val;
                        break;
                    case 28:
                        currFrame.rawGt10_0um = val;
                        break;
                    case 29:
                        val = frameBuf[detectOff-1];
                        currFrame.version = val;
                        break;
                    case 30:
                        val = frameBuf[detectOff-1];
                        currFrame.errorCode = val;
                        break;
                    case 32:
                        currFrame.checksum = val;
                        calcChecksum -= ((val>>8)+(val&0xFF));
                        break;
                    default:
                        break;
                }
    
                if (detectOff >= frameLen) {
                    if (DEBUG) {
                        Serial.println("-- Frame complete");
                    }
                    packetReceived = true;
                    detectOff = 0;
                    inFrame = false;
                }
            }
        }
    }
    Serial1.end();
    checksumErr = !(calcChecksum == currFrame.checksum);
    return checksumErr;
}
