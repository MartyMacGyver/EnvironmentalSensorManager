
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

#ifndef PARTICULATES_PMS7003_H_
#define PARTICULATES_PMS7003_H_

#include "application.h"

class ParticulatesSensorPMS7003 {
    public:
        struct PMS7003_framestruct {
            uint8_t  frameHeader[2];
            uint16_t frameLen = MAX_FRAME_LEN;
            uint16_t concPM1_0_CF1;
            uint16_t concPM2_5_CF1;
            uint16_t concPM10_0_CF1;
            uint16_t concPM1_0_amb;
            uint16_t concPM2_5_amb;
            uint16_t concPM10_0_amb;
            uint16_t rawGt0_3um;
            uint16_t rawGt0_5um;
            uint16_t rawGt1_0um;
            uint16_t rawGt2_5um;
            uint16_t rawGt5_0um;
            uint16_t rawGt10_0um;
            uint8_t  version;
            uint8_t  errorCode;
            uint16_t checksum;
        };
        ParticulatesSensorPMS7003();
        bool readData();
        char * snprintfData(char *, int);
        bool checksumErr = true;

    private:
        PMS7003_framestruct currFrame;
        const static bool DEBUG = false;
        const static int MAX_FRAME_LEN = 64;
        int incomingByte = 0; // for incoming serial data
        char frameBuf[MAX_FRAME_LEN];
        int  frameLen = MAX_FRAME_LEN;
        int detectOff = 0;
        bool inFrame = false;
        uint16_t calcChecksum;
};


#endif  // PARTICULATES_PMS7003_H_
