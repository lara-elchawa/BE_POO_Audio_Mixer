#include "realTimeCommPC.h"

bool realTimeCommPC::begin() {
    _queueToPC = xQueueCreate(20, sizeof(ControlMsg));
    _queueToESP = xQueueCreate(20, sizeof(ControlMsg));

    if (_queueToPC == NULL || _queueToESP == NULL) return false;

    // Lancement des tâches de communication sur le CORE 0
    xTaskCreatePinnedToCore(outgoingTaskStarter, "TaskOutPC", 4096, this, 1, NULL, 0);
    xTaskCreatePinnedToCore(incomingTaskStarter, "TaskInPC", 4096, this, 1, NULL, 0);

    return true;
}

void realTimeCommPC::outgoingTaskStarter(void* pvParameters) {
    ((realTimeCommPC*)pvParameters)->handleOutgoingMessagesTask();
}

void realTimeCommPC::incomingTaskStarter(void* pvParameters) {
    ((realTimeCommPC*)pvParameters)->handleIncomingMessagesTask();
}

void realTimeCommPC::handleOutgoingMessagesTask() {
    ControlMsg msg;
    for (;;) {
        if (xQueueReceive(_queueToPC, &msg, portMAX_DELAY)) {
            
            if (msg.type == DEBUG_MSG) {
                _serial.print("DEBUG: ");
                _serial.println(msg.rawData);
            } 
            else {
                // Ici, c'est le SEUL endroit du programme qui appelle sendCommandToPC
                switch (msg.type) {
                    case VOLUME_UP:         sendCommandToPC(CMD_VOLUME_UP); break;
                    case VOLUME_DOWN:       sendCommandToPC(CMD_VOLUME_DOWN); break;
                    case MUTE:              sendCommandToPC(CMD_MUTE); break;
                    case NEXT_TRACK:        sendCommandToPC(CMD_NEXT_TRACK); break;
                    case PREV_TRACK:        sendCommandToPC(CMD_PREV_TRACK); break;
                    case PAUSE_PLAY:        sendCommandToPC(CMD_PAUSE_PLAY); break;
                    case NEXT_SOFTWARE:     sendCommandToPC(CMD_NEXT_SOFTWARE); break;
                    case GET_SOFTWARE_LIST: sendCommandToPC(CMD_GET_SOFTWARE_LIST); break;
                    
                    case SET_VOLUME:
                        sendCommandToPC(String(CMD_SET_VOLUME) + ":" + String(msg.value));
                        break;

                    case NVIC_SYSTEM_RESET:
                        sendCommandToPC(CMD_NVIC_SYSTEM_RESET);
                        vTaskDelay(pdMS_TO_TICKS(10));
                        ESP.restart();
                        break;
                }
            }
        }
    }
}

void realTimeCommPC::handleIncomingMessagesTask() {
    _serial.setTimeout(100);
    for (;;) {
        if (_serial.available() > 0) {
            String input = _serial.readStringUntil('\n');
            input.trim();
            ControlMsg notifyMsg;
            bool shouldNotify = false;

            if (input.startsWith("VOL:")) {
                _currentVolume = input.substring(4).toInt();
                notifyMsg.type = SET_VOLUME;
                notifyMsg.value = _currentVolume;
                shouldNotify = true;
            }
            else if (input.startsWith("LIST:")) {
                parseSoftwareList(input.substring(5));
                notifyMsg.type = GET_SOFTWARE_LIST;
                shouldNotify = true;
            }
            else if (input.startsWith("SET_COLOR:")) {
                parseRGBColor(input.substring(10));
                notifyMsg.type = NEXT_SOFTWARE; 
                shouldNotify = true;
            }
            else if (input.startsWith("MUTE:")) {
                _isMuted = input.substring(5).toInt();
                notifyMsg.type = MUTE;
                notifyMsg.value = _isMuted;
                shouldNotify = true;
            }

            if (shouldNotify) xQueueSend(_queueToESP, &notifyMsg, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void realTimeCommPC::parseSoftwareList(String list) {
    int count = 0, startIdx = 0;
    int endIdx = list.indexOf(';');
    while (endIdx != -1 && count < MAX_SOFTWARES) {
        _ListSoftwares[count++] = list.substring(startIdx, endIdx);
        startIdx = endIdx + 1;
        endIdx = list.indexOf(';', startIdx);
    }
    if (count < MAX_SOFTWARES) _ListSoftwares[count++] = list.substring(startIdx);
    _numSoftwares = count;
}

void realTimeCommPC::parseRGBColor(String rgb) {
    int firstComma = rgb.indexOf(',');
    int secondComma = rgb.indexOf(',', firstComma + 1);
    if (firstComma != -1 && secondComma != -1) {
        _softwareColorRGB[0] = rgb.substring(0, firstComma).toInt();
        _softwareColorRGB[1] = rgb.substring(firstComma + 1, secondComma).toInt();
        _softwareColorRGB[2] = rgb.substring(secondComma + 1).toInt();
    }
}