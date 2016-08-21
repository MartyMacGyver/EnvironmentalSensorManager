
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

#include "barometer-ms5637.h"

#include "application.h"

BarometricSensorMS5637::BarometricSensorMS5637(int i2c_addr)  : sensorReady(false), i2c_addr( i2c_addr )
{
}

int BarometricSensorMS5637::resetDevice()
{
    // Returns 0 if all is OK
    int rcw = 0;
    sensorReady = false;
    Wire.beginTransmission(i2c_addr);
    Wire.write(Dev_Reset);
    rcw = Wire.endTransmission();
    if (rcw) {
        return rcw;
    }
    if (!readPROM()) {
        sensorReady = true;
    }
    return 0;
}

int BarometricSensorMS5637::readData(uint8_t cmd, int bytes, uint32_t *val) {
    int rcw = 0;
    *val = 0;
    Wire.beginTransmission(i2c_addr);
    Wire.write(cmd);
    rcw = Wire.endTransmission(I2C_KeepAlive);
    if (rcw) {
        return rcw;
    }
    rcw = Wire.requestFrom(i2c_addr, bytes);
    if (rcw != bytes) {
        return 42;
    }
    for (int i = bytes-1; i >= 0; i--) {
        *val |= (uint32_t)Wire.read()<<(i*8);
    }
    return 0;
}

int BarometricSensorMS5637::readPROM()
{
    int rcw = 0;
    for (int i = 0; i < 7; i++) {
        rcw = readData(Dev_PROM_R + (i<<1), 2, (uint32_t*)&promV[i]);
        if (rcw) {
            return rcw;
        }
        promV[PROM_UNUSED7] = 0;
    }
    return getPROMcrc();
}

int BarometricSensorMS5637::readADC(int ADC_mode, int OSR, uint32_t *val)
{
    int rcw = 0;
    Wire.beginTransmission(i2c_addr);
    Wire.write(ADC_mode+Dev_Conv_Offset[OSR]);
    rcw = Wire.endTransmission(I2C_KeepAlive);
    if (rcw) {
        return rcw;
    }
    delay(Dev_Conv_Delay[OSR]);
    rcw = readData(Dev_ADC_R, 3, val);
    if (rcw) {
        return rcw;
    }
    return 0;
}

int BarometricSensorMS5637::getPROMcrc()
{
    // CRC4 routine as modified from datasheet
    // Returns 0 (false) if CRC is OK
    //
    //promV[2] ^= 0x5A;
    //
    uint16_t n_rem = 0; // crc reminder
    uint8_t n_bit;
    uint16_t prom_keep_0 = promV[0];
    promV[0] &= 0x0FFF; // CRC byte is replaced by 0
    for (uint8_t cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
    { // choose LSB or MSB
        if (cnt%2==1) {
            n_rem ^= (promV[cnt>>1]) & 0x00FF;
        }
        else {
            n_rem ^= promV[cnt>>1]>>8;
        }
        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000) {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else {
                n_rem = n_rem << 1;
            }
        }
    }
    n_rem = (n_rem >> 12) & 0x000F; // final 4-bit reminder is CRC code
    promV[0] = prom_keep_0;
    return (uint8_t)((promV[0]>>12) & 0x000F) - n_rem;
}

int BarometricSensorMS5637::readPressureAndTemperature(int OSR) {
    int rcw = 0;
    uint32_t sensD1 = 0;
    rcw = readADC(Dev_Conv_D1, OSR, &sensD1);
    rawPressure = sensD1;

    uint32_t sensD2 = 0;
    rcw = readADC(Dev_Conv_D2, OSR, &sensD2);
    //sensD2 = 0x0073F075;  # Dummy data for 83.88F
    rawTemperature = sensD2;

    int64_t sensDT = sensD2 - promV[PROM_C5] * (static_cast<int64_t>(1)<<8);
    int64_t tempC  = 2000 + sensDT * promV[PROM_C6] / (static_cast<int64_t>(1)<<23);

    int64_t corrT2    = 0;
    int64_t corrOFF2  = 0;
    int64_t corrSENS2 = 0;
    
    if (tempC < 2000) {
        corrT2    = 3 * sensDT * sensDT / (static_cast<int64_t>(1)<<33);
        corrOFF2  = 61 * (tempC - 2000) * (tempC - 2000) / (static_cast<int64_t>(1)<<4);
        corrSENS2 = 29 * (tempC - 2000) * (tempC - 2000) / (static_cast<int64_t>(1)<<4);
        if (tempC < -1500) {
            corrOFF2  += 17 * (tempC + 1500) * (tempC + 1500);
            corrSENS2 +=  9 * (tempC + 1500) * (tempC + 1500);
        }
    }
    else {
        corrT2    = 5 * sensDT * sensDT / (static_cast<int64_t>(1)<<38);
        corrOFF2  = 0;
        corrSENS2 = 0;
    }
    tempC -= corrT2;

    int64_t sensOFF  = promV[PROM_C2] * (static_cast<int64_t>(1)<<17) + 
                       promV[PROM_C4] * sensDT / (static_cast<int64_t>(1)<<6);
    sensOFF -= corrOFF2;

    int64_t sensSENS = promV[PROM_C1] * (static_cast<int64_t>(1)<<16) + 
                       promV[PROM_C3] * sensDT / (static_cast<int64_t>(1)<<7);
    sensSENS -= corrSENS2;

    int64_t presM = (sensD1 * sensSENS / (static_cast<int64_t>(1)<<21) - sensOFF) / (static_cast<int64_t>(1)<<15);

    temperatureC = tempC / 100.0;
    pressureMbar = presM / 100.0;

    return rcw;
}

double BarometricSensorMS5637::getPressure(int OSR)
{
    return pressureMbar;
}

double BarometricSensorMS5637::getTemperature(int OSR)
{
    return temperatureC;
}

