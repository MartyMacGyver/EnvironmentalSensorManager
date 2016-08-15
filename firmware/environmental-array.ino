//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Environmental Data Array data collection firmware
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

// This #include statement was automatically added by the Particle IDE.
//#include "rht03-humidity-temperature-sensor/rht03-humidity-temperature-sensor.h"
#include "humidity-temperature-rht03.h"

// This #include statement was automatically added by the Particle IDE.
#include "barometer-ms5637.h"

const int printbufSize = 256;
char global_printbuf[printbufSize];

//const char degsym[] = "\xB0";  // Serial
//const char degsym[] = "&deg;"; // HTML
const char degsym[] = "°";     // Log interface

int rhtSensorPin    = D3;
int baroSensorAddr  = 0x76;
int co2SensorAddr   = 0x69;  // 0x68
int i2cDataRate     = CLOCK_SPEED_100KHZ; // CLOCK_SPEED_400KHZ
const int MaxErrCnt = 10;
const char DataSource [] = "EnvSensorArray_1";

RHT03HumidityTemperatureSensor rhtSensor(rhtSensorPin);
BarometricSensorMS5637 baroSensor(baroSensorAddr);

uint32_t seqNum = 0;
int globalErrorCount = 0;
bool initializing = true;

bool readCo2sensor(char outbuf[], int address) {
    int rcw = 0;
    int co2errors = 0;
    unsigned char buffer[4];
    int co2ppm;
    
    Wire.beginTransmission(address);
    Wire.write(0x00);
    rcw = Wire.endTransmission();
    rcw = 0;
    if (rcw) {
        snprintf(global_printbuf, printbufSize, "! CO2 COMMS error1 - RC = %d", rcw);
        Particle.publish(DataSource, global_printbuf, 60, PRIVATE);
        co2errors++;
        Wire.end();
        delay(500);
        Wire.begin();
        Wire.beginTransmission(address);
        Wire.write(0x00);
        rcw = Wire.endTransmission();
    }
    delay(10);
 
    Wire.beginTransmission(address);
    Wire.write(0x22);
    Wire.write(0x00);
    Wire.write(0x08);
    Wire.write(0x2A);
    rcw = Wire.endTransmission();
    if (rcw) {
        snprintf(global_printbuf, printbufSize, "! CO2 COMMS error2 - RC = %d", rcw);
        Particle.publish(DataSource, global_printbuf, 60, PRIVATE);
        co2errors++;
        rcw = Wire.endTransmission();
        rcw = Wire.endTransmission();
        rcw = Wire.endTransmission();
    }
    delay(10);
    
    Wire.requestFrom(address, 4);
    
    buffer[0] = 0; buffer[1] = 0; buffer[2] = 0; buffer[3] = 0;
    uint8_t i = 0;
    while(Wire.available() && i < 4)
    {
        buffer[i] = (uint8_t)Wire.read();
        i++;
    }

    co2ppm = ((buffer[1] & 0xFF)<<8)|(buffer[2] & 0xFF);
    uint8_t cksum = buffer[0] + buffer[1] + buffer[2] - buffer[3];

    bool rc = true;
    if (cksum) {
        co2errors++;
        co2ppm = 0;
        rc = false;
    }
    else if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 0) {
        co2errors++;
        co2ppm = 0;
        rc = false;
    }

    snprintf(outbuf, 64, "%4d %02X%02X%02X%02X",
    		 co2ppm,
	         buffer[0], buffer[1], buffer[2], buffer[3]);

    if (co2errors > 0) {
        globalErrorCount++;
    }
    else {
        if (globalErrorCount > 0) {
            globalErrorCount--;
        }
    }
    return rc;
}

bool readBaroSensor(char outbuf[]) {
    // Gather MS5637 sensor data
    if (!baroSensor.sensorReady) {
        baroSensor.resetDevice();
        delay(500);
        if (baroSensor.sensorReady) {
            uint16_t *vals = baroSensor.promV;
            snprintf(global_printbuf, printbufSize, "! MS5637 READY - PROM=[%04X,%04X,%04X,%04X,%04X,%04X,%04X]",
                	 vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6]);
            Particle.publish(DataSource, global_printbuf, 60, PRIVATE);
            delay(500);
        }
        else {
            uint16_t *vals = baroSensor.promV;
            snprintf(global_printbuf, printbufSize, "! MS5637 FAIL! - PROM=[%04X,%04X,%04X,%04X,%04X,%04X,%04X]",
                	 vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6]);
            Particle.publish(DataSource, global_printbuf, 60, PRIVATE);
            delay(500);
        }
    }
    baroSensor.readPressureAndTemperature(baroSensor.OSR8192);
    snprintf(outbuf, 64, "%6.2f %6.2f %08X %08X",
	         baroSensor.pressureMbar, baroSensor.temperatureC,
     		 baroSensor.rawPressure,  baroSensor.rawTemperature);
    return true;
}

bool readRhtSensor(char outbuf[]) {
    rhtSensor.update();
    double rhtTemperature = rhtSensor.getTemperature();
    double rhtHumidity = rhtSensor.getHumidity();
    uint8_t * rhtRawData = rhtSensor.getRaw();
    snprintf(outbuf, 64, "%6.2f %5.2f  %02X%02X%02X%02X%02X",
    		 rhtTemperature, rhtHumidity,
	         rhtRawData[0], rhtRawData[1], rhtRawData[2], rhtRawData[3], rhtRawData[4]);
    return true;
}

void setup() {
    Serial.begin(9600);
    Wire.setSpeed(i2cDataRate);
    Wire.begin();
    delay(500);
}


void loop() {
    int prevErrCnt = globalErrorCount;

	if (initializing) {
		const char header_co2[]  = "K30: CO2_ppm CO2_RAW";
		const char header_baro[] = "MS5637: P_mBar T_C P_RAW T_RAW";
		const char header_rht[]  = "RHT03: T_C RH_pct D_RAW";
	    snprintf(global_printbuf, printbufSize, "SEQ_NUM %s %s %s",
	    		 seqNum, header_co2, header_baro, header_rht);
	    Particle.publish(DataSource, global_printbuf, 60, PRIVATE);
	    // 12345678 9999 FFFFFFFF 9999.99 -999.99 FFFFFFFF FFFFFFFF -999.99 44.50 FFFFFFFFFF
	    initializing = false;
    	delay(2000);
	}

    // Gather CO2 sensor data
	char outbuf_co2[64];
    readCo2sensor(outbuf_co2, co2SensorAddr);
    delay(1000);

    // Gather MS5637 sensor data
	char outbuf_baro[64];
    readBaroSensor(outbuf_baro);
    delay(1000);

    // Gather RHT sensor data
	char outbuf_rht[64];
    readRhtSensor(outbuf_rht);
    delay(500);

	// Return whatever data we got
    snprintf(global_printbuf, printbufSize, "%08X CO2: %s MS5637: %s RHT03: %s",
    		 seqNum, outbuf_co2, outbuf_baro, outbuf_rht);
    Particle.publish(DataSource, global_printbuf, 60, PRIVATE);

    // Global error handling
    if (globalErrorCount >= MaxErrCnt) {
        Particle.publish(DataSource, "! Errors maxed: rebooting...", 60, PRIVATE);
        delay(1000);
        System.reset();
    }
    if (prevErrCnt != 0 && globalErrorCount == 0) {
        Particle.publish(DataSource, "! Error counter is clear", 60, PRIVATE);
    }

    seqNum++;
    delay(500); // Aim for 5 seconds in the each normal loop
}

