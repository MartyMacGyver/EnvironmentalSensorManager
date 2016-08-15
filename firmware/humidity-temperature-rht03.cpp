
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
// Forked 2016-04-30 by Martin Falatic
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#include "humidity-temperature-rht03.h"

#include "application.h"

#define EXPECTED_TRANSITION_INTERRUPT_COUNT 84
#define PULSE_WIDTH_TRUE_THRESHOLD_MICROSECONDS 35

static void sensorInterruptHandlerRedirect();

enum {
	StateUnknown,
	StateIdle,
	StateListening
};

RHT03HumidityTemperatureSensor *RHT03HumidityTemperatureSensor::currentlyListeningInstance = NULL;

RHT03HumidityTemperatureSensor::RHT03HumidityTemperatureSensor(int aPin)
{
	pin = aPin;
	state = StateIdle;
}

double RHT03HumidityTemperatureSensor::getTemperature()
{
	return temperature;
}

double RHT03HumidityTemperatureSensor::getHumidity()
{
	return humidity;
}

unsigned char * RHT03HumidityTemperatureSensor::getRaw()
{
	return rawdata;
}


void RHT03HumidityTemperatureSensor::update()
{
	while (1) {
		if (state == StateIdle) {
			delay(1000);

			// Pull the pin low for 10ms to request a measurement from the sensor
			pinMode(pin, OUTPUT);
			digitalWrite(pin, HIGH);
			delay(10);
			digitalWrite(pin, LOW);
			delay(10);
			digitalWrite(pin, HIGH);

			// Configure the pin for input and set up the change interrupt handler
			pinMode(pin, INPUT_PULLUP);
			listeningStartTime = micros();
			RHT03HumidityTemperatureSensor::currentlyListeningInstance = this;
			attachInterrupt(pin, sensorInterruptHandlerRedirect, CHANGE);

			state = StateListening;
		} else if (state == StateListening) {
			unsigned long listeningInterval = micros() - listeningStartTime;
			// We assume that the measurement is done after this interval
			if (interruptCount == EXPECTED_TRANSITION_INTERRUPT_COUNT || listeningInterval > 4000000) {
				// Collect the data
				unsigned int rh = payload[0] << 8 | payload[1];
   				humidity = (double)rh/10;
				unsigned int t = payload[2] << 8 | payload[3];
				if ((t & 0x8000) == 0x8000) {
					t = (t | 0x7FFF);
	  				temperature = -(double)t/10;
				}
				else {
	  				temperature = (double)t/10;
				}

				unsigned int csum =
					((payload[0] + payload[1] + payload[2] + payload[3]) & 255) - payload[4];
				if (csum != 0) {
				    humidity = (double)9999.0;
				    temperature = (double)9999.0;
				}

				// Detach interrupt handler and reset all state for the next measurement
				detachInterrupt(pin);
				lastInterruptTime = 0;
				listeningStartTime = 0;
				interruptCount = 0;
				for (int i = 0; i < PAYLOAD_BYTES; i++) {
					rawdata[i] = payload[i];
					payload[i] = 0;
				}

				RHT03HumidityTemperatureSensor::currentlyListeningInstance = NULL;
				state = StateIdle;
				break;
			}
			delay(1000);
		}

	}
}


void RHT03HumidityTemperatureSensor::handleInterrupt()
{
	interruptCount++;

	unsigned long now = micros();
	unsigned long intervalSinceLastChange = 0;
	if (lastInterruptTime) {
		intervalSinceLastChange = now - lastInterruptTime;
	}
	lastInterruptTime = now;

	if (interruptCount < 4) {
		// The first three transitions are for the two leading 80us pulses, ignore those
		return;
	} else if (interruptCount % 2) {
		// Every even transition starting with the fourth is for the rising
		// edge that ends the 50us low period that comes before every bit.
		// We note the time of that even transition (see above) but ignore
		// it otherwise.
		// Every odd transition marks the end of a bit and the length
		// of the pulse will tell us if it was a 0 or 1 bit.
		int bitIndex = (interruptCount - 5) / 2;
		unsigned char value = intervalSinceLastChange > PULSE_WIDTH_TRUE_THRESHOLD_MICROSECONDS ? 1 : 0;
		// Store the bit into the appropriate bit position of the appropriate
		// payload byte.
		payload[bitIndex/8] |= (value << (7 - (bitIndex % 8)));
	}
}


void sensorInterruptHandlerRedirect() {
	RHT03HumidityTemperatureSensor::currentlyListeningInstance->handleInterrupt();
}




