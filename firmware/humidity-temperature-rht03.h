
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Code to read out the humidity and temperature values from a Maxdetect RHT03
// humidity/temperature sensor. That sensor is apparently also known as DHT22
// and AM2302.
// From: https://github.com/liyanage/maxdetect-rht03-humidity-sensor
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
Copyright (c) 2014, Marc Liyanage
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Original - https://github.com/liyanage/maxdetect-rht03-humidity-sensor
// Forked 2016-04-30 by Martin Falatic
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef RHT03_HUMIDITY_TEMPERATURE_SENSOR_H_
#define RHT03_HUMIDITY_TEMPERATURE_SENSOR_H_

#define PAYLOAD_BITS 40
#define PAYLOAD_BYTES (PAYLOAD_BITS/8)

class RHT03HumidityTemperatureSensor {
    public:
        RHT03HumidityTemperatureSensor(int pin);
        double getTemperature();
        double getHumidity();
        unsigned char * getRaw();
        void update();
        void handleInterrupt();
        static RHT03HumidityTemperatureSensor *currentlyListeningInstance;

    private:
        int pin;
        double temperature;
        double humidity;
        int state;
        unsigned long listeningStartTime = 0;
        volatile unsigned long lastInterruptTime = 0;
        volatile int interruptCount = 0;
        volatile unsigned char payload[PAYLOAD_BYTES];
        unsigned char rawdata[PAYLOAD_BYTES];
};

#endif  // RHT03_HUMIDITY_TEMPERATURE_SENSOR_H_
