
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// An implementation for the Sensirion SHT31-D digital humidity/temperature sensor
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

#include "humidity-temperature-sht31d.h"

HumidityTempSensorSHT31D::HumidityTempSensorSHT31D(int i2c_addr) : i2c_addr( i2c_addr )
{
}

char * HumidityTempSensorSHT31D::snprintfData(char* printbuf, int len) {
    printbuf[0] = 0;
    snprintf(printbuf, len, "%s%.2f %.2f %02x%02x%02x %02x %02x%02x%02x %02x", printbuf,
        temperatureC, rhPercentage,
        vals[0], vals[1], vals[2], tmCRCcalc, vals[3], vals[4], vals[5], rhCRCcalc);
//    snprintf(printbuf, len, "%scsum=%04x %s xsum=%04x", printbuf,
//        currFrame.checksum, (calcChecksum == currFrame.checksum ? "==" : "!="), calcChecksum);
    return printbuf;
}


bool HumidityTempSensorSHT31D::readData() {
    if (DEBUG) {
        Serial.println("-- Reading SHT31D");
    }
    for (int i = 0; i < 6; i++) {
        vals[i] = 0;
    }
    int rcw = 0;
    Wire.beginTransmission(i2c_addr);
    Wire.write(0x24); //2C06 = clock stretching
    Wire.write(0x00);
    rcw = Wire.endTransmission(); // no I2C_KeepAlive
    if (rcw) {
        return rcw;
    }
    delay(100);
    rcw = Wire.requestFrom(i2c_addr, 6);
    if (rcw != 6) {
        return true;
    }
    for (int i = 0; i < 6; i++) {
        vals[i] = Wire.read();
    }
    tmVal = (vals[0] << 8) | vals[1];
    temperatureC = -45+175*(static_cast<double>(tmVal)/((1<<16)-1));
    tmCRC =  vals[2];
    rhVal = (vals[3] << 8) | vals[4];
    rhPercentage = 100*(static_cast<double>(rhVal)/((1<<16)-1));
    rhCRC =  vals[5];
    tmCRCcalc = calcCRC1wire(0xFF, vals[0]);
    tmCRCcalc = calcCRC1wire(tmCRCcalc, vals[1]);
    rhCRCcalc = calcCRC1wire(0xFF, vals[3]);
    rhCRCcalc = calcCRC1wire(rhCRCcalc, vals[4]);
    return checksumErr;
}

//########################################
// Is this CRC algo incorrect?
//########################################
uint8_t HumidityTempSensorSHT31D::calcCRC1wire(uint8_t crc, uint8_t data)
{
    // Uses CRC-8-Dallas/Maxim 1-Wire x^8+x^5+x^4+1  0x31 / 0x8C / 0x98 
    uint8_t i;
    
    crc = crc ^ data;
    for (i = 0; i < 8; i++)
    {
        if (crc & 0x01)
            crc = (crc >> 1) ^ 0x31;
        else
            crc >>= 1;
    }

    return crc;
}
