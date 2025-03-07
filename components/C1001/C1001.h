/*
Portions of this code are adapted from the DFRobot HumanDetection library under the following license:

MIT License

Copyright (c) 2024 DFRobot

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

/**
 * @brief Sleep composite state data
 */
typedef struct
{
    uint8_t presence;           ///< Presence state
    uint8_t sleepState;         ///< Sleep state
    uint8_t averageRespiration; ///< Average respiration
    uint8_t averageHeartbeat;   ///< Average heartbeat
    uint8_t turnoverNumber;     /// Turnover number
    uint8_t largeBodyMove;      ///< Large body movement percentage
    uint8_t minorBodyMove;      ///< Minor body movement percentage
    uint8_t apneaEvents;        ///< Apnea events

} sSleepComposite;

/**
 * @brief Sleep statistics query
 */
typedef struct
{
    uint8_t sleepQualityScore;      ///< Sleep quality score
    uint16_t sleepTime;             ///< Sleep duration in minutes
    uint8_t wakeDuration;           ///< Wake duration
    uint8_t shallowSleepPercentage; ///< Shallow sleep duration percentage
    uint8_t deepSleepPercentage;    ///< Deep sleep duration percentage
    uint8_t timeOutOfBed;           ///< Time out of bed
    uint8_t exitCount;              ///< Exit count
    uint8_t turnOverCount;          ///< Turnover count
    uint8_t averageRespiration;     ///< Average respiration
    uint8_t averageHeartbeat;       /// Average heartbeat
    uint8_t apneaEvents;            ///< Apnea events

} sSleepStatistics;


namespace esphome {
namespace C1001 {

    typedef enum
    {
        eSleepMode = 0x02,
        eFallingMode = 0x01,
    } eWorkMode;

    enum RxStates {
        CMD_WHITE = 0,
        CMD_HEAD = 1,
        CMD_CONFIG = 2,
        CMD_CMD = 3,
        CMD_LEN_H = 4,
        CMD_LEN_L = 5,
        CMD_DATA = 6,
        CMD_END_H,
        CMD_END_L,
        CMD_DONE
    };

    enum ControlValues {
        CON_SYSTEM = 0x01,
        CON_WORKMODE = 0x02,
        CON_GETHUMAN = 0x80,
        CON_GETBREATHE = 0x81,
        CON_GETSLEEP = 0x84,
        CON_GETHEART = 0x85,
        CON_CONFIGSLEEP = 0x84
    };

    enum CommandValues {
        COM_HEARTBEAT = 0x01,
        COM_RESET = 0x02,

        COM_SETLEDFALL = 0x04,
        COM_SETLEDHP = 0x03,
        COM_GETLEDFALL = 0x84,
        COM_GETLEDHP = 0x83,

        COM_GETSLEEPCOMPOSITE = 0x8D,
        COM_GETSLEEPSTATISTICS = 0x8F,
        COM_SETSLEEPREPORT = 0x00,
        COM_GETSLEEPSTATUS = 0x82,
        COM_REPORTBEDOCCUPANCY = 0x01,
        COM_REPORTSLEEPSTATUS = 0x02,
        COM_REPORTWAKEDURATION = 0x03,
        COM_REPORTLIGHTSLEEPDURATION = 0x04,
        COM_REPORTDEEPSLEEPDURATION = 0x05,
        COM_REPORTSLEEPSCORE = 0x06,

        COM_GETBREATHEVALUE = 0x81,
        COM_GETBREATHESTATE = 0x82,
        COM_SETBREATHEREPORT = 0x00,
        COM_REPORTBREATHEVALUE = 0x02,

        COM_GETHEARTRATE = 0x82,
        COM_SETHEARTRATEREPORT = 0x00,
        COM_REPORTHEARTRATE = 0x02,
        
        COM_GETWORKMODE = 0xA8,
        COM_SETWORKMODE = 0x08
    };

    enum RadarStates {
        RDR_BOOT,
        RDR_BOOTDONE,
        RDR_WORKMODE,
        RDR_WORKMODEWAIT,
        RDR_RESET,
        RDR_RESETWAIT
    };
    
    class C1001Component : public PollingComponent,
                           public uart::UARTDevice {  // The class name must be the name defined by text_sensor.py
        #ifdef USE_BINARY_SENSOR
            SUB_BINARY_SENSOR(bed_occupied);
        #endif
        #ifdef USE_SENSOR
            SUB_SENSOR(sleep_state);
            SUB_SENSOR(wake_duration);
            SUB_SENSOR(light_sleep_duration);
            SUB_SENSOR(deep_sleep_duration);
            SUB_SENSOR(sleep_score);
            SUB_SENSOR(breath_rate);
            SUB_SENSOR(heart_rate);
        #endif

        public:
            C1001Component();
            float get_setup_priority() const override { return esphome::setup_priority::LATE; }
            void dump_config() override;
            void setup() override;
            void update() override;
            void loop() override;

        private:
            RxStates rxState;
            std::vector<uint8_t> rxBuf;
            uint8_t rxLen;
            uint8_t rxCount;
            RadarStates rdrState;
            bool commandPending;
            uint32_t timeoutStamp;

            void sendCommand(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData);
            void sendQuery(uint8_t con, uint8_t cmd);
            uint8_t computeChecksum(uint8_t len, uint8_t *buf);
            void processData();
    };
} // namespace C1001
} // namespace esphome
