
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

HumidityTempSensorSHT31D::HumidityTempSensorSHT31D()
{
}

char * HumidityTempSensorSHT31D::snprintfData(char* printbuf, int len) {
    printbuf[0] = 0;
//    snprintf(printbuf, len, "%s[%02x %02x] (%04x) ", printbuf,
//        currFrame.frameHeader[0], currFrame.frameHeader[1], currFrame.frameLen);
//    snprintf(printbuf, len, "%scsum=%04x %s xsum=%04x", printbuf,
//        currFrame.checksum, (calcChecksum == currFrame.checksum ? "==" : "!="), calcChecksum);
    return printbuf;
}


bool HumidityTempSensorSHT31D::readData() {
    if (DEBUG) {
        Serial.println("-- Reading SHT31D");
    }
    return checksumErr;
}
