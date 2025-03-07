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

#include "C1001.h"
#include "esphome/core/log.h"

namespace esphome {
namespace C1001 {
    static const char *const TAG = "C1001";

    C1001Component::C1001Component()
    {

    };

    void C1001Component::dump_config() {
        ESP_LOGCONFIG(TAG, "MR60BHA2:");
        #ifdef USE_BINARY_SENSOR
        LOG_BINARY_SENSOR(" ", "People Exist Binary Sensor", this->bed_occupied_binary_sensor_);
        #endif
        #ifdef USE_SENSOR
        LOG_SENSOR(" ", "Sleep State Sensor", this->sleep_state_sensor_);
        LOG_SENSOR(" ", "Wake Duration Sensor", this->wake_duration_sensor_);
        LOG_SENSOR(" ", "Deep Sleep Duration Sensor", this->deep_sleep_duration_sensor_);
        LOG_SENSOR(" ", "Sleep Score Sensor", this->sleep_score_sensor_);
        LOG_SENSOR(" ", "Breath Rate Sensor", this->breath_rate_sensor_);
        LOG_SENSOR(" ", "Heart Rate Sensor", this->heart_rate_sensor_);
        #endif
    }

    void C1001Component::setup() {
        rxState = RxStates::CMD_WHITE;
        commandPending = false;
        rdrState = RadarStates::RDR_BOOT;
        timeoutStamp = millis();
    }

    void C1001Component::loop() {
        /*
        */

        uint8_t led = 1;
        switch (rdrState)
        {
            case RadarStates::RDR_BOOTDONE:
                ESP_LOGD(TAG, "Radar booted");
                rdrState = RadarStates::RDR_WORKMODE;
                break;
            case RadarStates::RDR_WORKMODE:
                ESP_LOGD(TAG, "Radar check workmode");
                sendQuery(ControlValues::CON_WORKMODE, CommandValues::COM_GETWORKMODE);
                rdrState = RadarStates::RDR_WORKMODEWAIT;
                break;
            case RadarStates::RDR_WORKMODEWAIT:
                if(!commandPending) {
                    ESP_LOGD(TAG, "Radar workmode set");
                    rdrState = RadarStates::RDR_RESET;
                }
                break;
            case RadarStates::RDR_RESET:
                ESP_LOGD(TAG, "Radar reset");
                sendQuery(ControlValues::CON_SYSTEM, CommandValues::COM_RESET);
                rdrState = RadarStates::RDR_RESETWAIT;
                timeoutStamp = millis();
                break;
            default:
                break;
        }
        
        processData();
    }

    void C1001Component::update() {
        if(!commandPending)
            sendQuery(ControlValues::CON_GETSLEEP, CommandValues::COM_GETSLEEPSTATUS);
    }

    void C1001Component::processData() {
        //if(available() > 0) ESP_LOGD(TAG, "Processing");
        while (available() > 0 && rxState != RxStates::CMD_DONE)
        {
            uint8_t data;
            read_byte(&data);
            //ESP_LOGD(TAG, "Got %x", data);
            
            switch (rxState)
            {
            case RxStates::CMD_WHITE:
                if (data == 0x53)
                {
                    rxBuf.push_back(data);
                    rxState = RxStates::CMD_HEAD;
                    rxCount = 0; // Reset count
                }
                break;
            case RxStates::CMD_HEAD:
                if (data == 0x59)
                {
                    rxState = RxStates::CMD_CONFIG;
                    rxBuf.push_back(data);
                }
                else
                {
                    rxState = RxStates::CMD_WHITE;
                    ESP_LOGW(TAG, "Radar bad packet");
                    rxBuf.clear();
                }
                break;
            case RxStates::CMD_CONFIG:
                rxState = RxStates::CMD_CMD;
                rxBuf.push_back(data);
                break;
            case RxStates::CMD_CMD:
                rxState = RxStates::CMD_LEN_H;
                rxBuf.push_back(data);
                break;
            case RxStates::CMD_LEN_H:
                rxBuf.push_back(data);
                rxLen = data << 8;
                rxState = RxStates::CMD_LEN_L;
                break;
            case RxStates::CMD_LEN_L:
                rxBuf.push_back(data);
                rxLen |= data;
                rxState = RxStates::CMD_DATA;
                // DBG(_len);
                break;
            case RxStates::CMD_DATA:
                if (rxCount < rxLen)
                {
                    rxBuf.push_back(data);
                    rxCount++;
                }
                else
                {
                    if (data == computeChecksum(6 + rxLen, rxBuf.data()))
                    {
                        rxBuf.push_back(data);
                        rxState = RxStates::CMD_END_H;
                    }
                    else
                    {
                        rxState = RxStates::CMD_WHITE;
                        ESP_LOGW(TAG, "Radar bad packet %x,%x!", rxBuf[RxStates::CMD_CONFIG], rxBuf[RxStates::CMD_CMD]);
                        rxBuf.clear();
                    }
                }
                break;
            case RxStates::CMD_END_H:
                rxBuf.push_back(data);
                rxState = RxStates::CMD_END_L;
                break;
            case RxStates::CMD_END_L:
                rxBuf.push_back(data);
                rxState = RxStates::CMD_DONE;
                ESP_LOGV(TAG, "Radar sent a response packet, %x,%x!", rxBuf[RxStates::CMD_CONFIG], rxBuf[RxStates::CMD_CMD]);
                commandPending = false;
                break;
            default:
                break;
            }
        }
        
        if(rxState == RxStates::CMD_DONE) {
            rxState = RxStates::CMD_WHITE;
            switch (rxBuf[RxStates::CMD_CONFIG])
            {
            case ControlValues::CON_SYSTEM:
                if(rxBuf[RxStates::CMD_CMD] == CommandValues::COM_HEARTBEAT){
                    ESP_LOGD(TAG, "Radar sent heartbeat!");
                }
                break;
            case ControlValues::CON_WORKMODE:
                if(rxBuf[RxStates::CMD_CMD] == CommandValues::COM_GETWORKMODE){
                    ESP_LOGD(TAG, "Radar sent work mode %d", rxBuf[RxStates::CMD_DATA]);
                    if(rxBuf[RxStates::CMD_DATA] != eWorkMode::eSleepMode) {
                        ESP_LOGD(TAG, "Radar in wrong work mode!");
                        sendQuery(ControlValues::CON_WORKMODE, CommandValues::COM_SETWORKMODE);
                    }
                }
                break;
            case ControlValues::CON_GETSLEEP:
                switch (rxBuf[RxStates::CMD_CMD])
                {
                case CommandValues::COM_REPORTBEDOCCUPANCY:
                    ESP_LOGD(TAG, "Radar sent occupancy");
                    if(this->bed_occupied_binary_sensor_ != nullptr) {
                        switch (rxBuf[RxStates::CMD_DATA]) {
                            case 0:
                                this->bed_occupied_binary_sensor_->publish_state(false);
                                break;
                            case 1:
                                this->bed_occupied_binary_sensor_->publish_state(true);
                                break;
                        }
                    }
                    break;
                case CommandValues::COM_GETSLEEPSTATUS:
                case CommandValues::COM_REPORTSLEEPSTATUS:
                    ESP_LOGD(TAG, "Radar sent sleep status");
                    if(this->sleep_state_sensor_ != nullptr) {
                        this->sleep_state_sensor_->publish_state(rxBuf[RxStates::CMD_DATA]);
                    }
                    break;
                case CommandValues::COM_REPORTWAKEDURATION:
                    ESP_LOGD(TAG, "Radar sent wake duration");
                    if(this->wake_duration_sensor_ != nullptr) {
                        this->wake_duration_sensor_->publish_state(rxBuf[RxStates::CMD_DATA] << 8 | rxBuf[RxStates::CMD_DATA+1]);
                    }
                    break;
                case CommandValues::COM_REPORTLIGHTSLEEPDURATION:
                ESP_LOGD(TAG, "Radar sent light sleep duration");  
                    if(this->light_sleep_duration_sensor_ != nullptr) {
                        this->light_sleep_duration_sensor_->publish_state(rxBuf[RxStates::CMD_DATA] << 8 | rxBuf[RxStates::CMD_DATA+1]);
                    }
                    break;
                case CommandValues::COM_REPORTDEEPSLEEPDURATION:
                    ESP_LOGD(TAG, "Radar sent deep sleep duration");
                    if(this->deep_sleep_duration_sensor_ != nullptr) {
                        this->deep_sleep_duration_sensor_->publish_state(rxBuf[RxStates::CMD_DATA] << 8 | rxBuf[RxStates::CMD_DATA+1]);
                    }
                    break;
                case CommandValues::COM_REPORTSLEEPSCORE:
                    ESP_LOGD(TAG, "Radar sent sleep score");
                    if(this->sleep_score_sensor_ != nullptr) {
                        this->sleep_score_sensor_->publish_state(rxBuf[RxStates::CMD_DATA]);
                    }
                    break;
                default:
                    ESP_LOGD(TAG, "Radar sent unknown sleep 0x%x", rxBuf[RxStates::CMD_CMD]);
                    break;
                }
                break;
            case ControlValues::CON_GETBREATHE:
                if(rxBuf[RxStates::CMD_CMD] == CommandValues::COM_REPORTBREATHEVALUE)
                    ESP_LOGD(TAG, "Radar sent breathe value");
                    if(this->breath_rate_sensor_ != nullptr) {
                        this->breath_rate_sensor_->publish_state(rxBuf[RxStates::CMD_DATA]);
                    }
                break;

            case ControlValues::CON_GETHEART:
                if(rxBuf[RxStates::CMD_CMD] == CommandValues::COM_REPORTHEARTRATE)
                    ESP_LOGD(TAG, "Radar sent heart rate");
                    if(this->heart_rate_sensor_ != nullptr) {
                        this->heart_rate_sensor_->publish_state(rxBuf[RxStates::CMD_DATA]);
                    }
                break;
            default:
                ESP_LOGD(TAG, "Unknown packet 0x%x, 0x%x", rxBuf[RxStates::CMD_CONFIG], rxBuf[RxStates::CMD_CMD]);
                break;
            }
            rxBuf.clear();
        }
    }

    void C1001Component::sendQuery(uint8_t con, uint8_t cmd) {
        uint8_t data = 0x0F;
        sendCommand(con, cmd, 1, &data);
    }

    void C1001Component::sendCommand(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData) {
        uint8_t cmdBuf[20];
        cmdBuf[0] = 0x53;
        cmdBuf[1] = 0x59;
        cmdBuf[2] = con;
        cmdBuf[3] = cmd;
        cmdBuf[4] = (len >> 8) & 0xff;
        cmdBuf[5] = len & 0xff;
        memcpy(&cmdBuf[6], senData, len);
        cmdBuf[6 + len] = computeChecksum(6 + len, cmdBuf);
        cmdBuf[7 + len] = 0x54;
        cmdBuf[8 + len] = 0x43;

        write_array(cmdBuf, 9+len);
        commandPending = true;
    }

    uint8_t C1001Component::computeChecksum(uint8_t len, uint8_t *buf)
    {
        uint8_t data = 0;
        uint8_t *_buf = buf;
        for (uint8_t i = 0; i < len; i++)
        {
            data += _buf[i];
        }
        return data;
    }

} // namespace C1001
}    