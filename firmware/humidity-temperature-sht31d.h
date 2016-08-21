
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

#ifndef HUMIDITY_TEMPERATURE_SHT31D_H_
#define HUMIDITY_TEMPERATURE_SHT31D_H_

#include "application.h"

class HumidityTempSensorSHT31D {
    public:
        HumidityTempSensorSHT31D(int i2c_addr);
        char * snprintfData(char *, int);
        bool readData();
        bool checksumErr = true;
        double temperatureC;
        double rhPercentage;

    private:
        uint8_t calcCRC1wire(uint8_t crc, uint8_t data);
        const static bool DEBUG = false;
        int i2c_addr = 0x44;
        uint8_t vals[6];
        uint16_t tmVal;
        uint8_t  tmCRC;
        uint8_t  tmCRCcalc;
        uint16_t rhVal;
        uint8_t  rhCRC;
        uint8_t  rhCRCcalc;
};


#endif  // HUMIDITY_TEMPERATURE_SHT31D_H_
