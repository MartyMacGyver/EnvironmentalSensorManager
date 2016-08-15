
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// An implementation for the MS5637 barometric sensor
// Based on spec at http://www.meas-spec.com/downloads/MS5637-02BA03.pdf
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

#ifndef BAROMETER_MS5637_H_
#define BAROMETER_MS5637_H_

#include "application.h"

class BarometricSensorMS5637 {
    public:
        BarometricSensorMS5637(int i2c_addr);
        int resetDevice();
        int readData(uint8_t cmd, int bytes, uint32_t *val);
        int readPressureAndTemperature(int OSR);
        double getPressure(int OSR);
        double getTemperature(int OSR);
        enum OverSamples { OSR256, OSR512, OSR1024, OSR2048, OSR4096, OSR8192 };
        enum PROMlocs { PROM_CRC_FAC, PROM_C1, PROM_C2, PROM_C3, PROM_C4, PROM_C5, PROM_C6, PROM_UNUSED7 };
        uint16_t promV[8];
        bool sensorReady;
        uint32_t rawTemperature;
        uint32_t rawPressure;
        double temperatureC;
        double pressureMbar;
        

    private:
        const bool I2C_KeepAlive = false;

        // Conv times: 0.5 / 1.1 / 2.1 / 4.1 / 8.22 / 16.44 ms
        const unsigned char Dev_Conv_Offset [6] = { 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A };
        const unsigned char Dev_Conv_Delay [6]  = {  1+4,  2+4,  3+4,  5+4,  9+4, 17+4 };
        const unsigned char Dev_Reset   = 0x1E;
        const unsigned char Dev_ADC_R   = 0x00;  // Reads 3 bytes
        const unsigned char Dev_PROM_R  = 0xA0;  // Add addr << 1 where addr = [0:7]
        const unsigned char Dev_Conv_D1 = 0x40;  // uses Dev_Conv_Offset
        const unsigned char Dev_Conv_D2 = 0x50;  // uses Dev_Conv_Offset

        int i2c_addr;

        int readADC(int ADC_mode, int OSR, uint32_t *val);
        int readPROM();
        int getPROMcrc();
};


#endif  // BAROMETER_MS5637_H_
