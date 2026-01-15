#ifndef REALTIMECOMMPC_H
#define REALTIMECOMMPC_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "commPC.h"
#include "realTimeUtils.h"

class realTimeCommPC : public commPC {
public:
    /**
     * @brief Constructeur
     */
    realTimeCommPC(HardwareSerial& serial) 
        : commPC(serial), _queueToPC(NULL), _queueToESP(NULL) {}

    /**
     * @brief Initialise les queues et lance les tâches FreeRTOS sur le Core 0
     */
    bool begin();

    /**
     * @brief Envoie une commande vers le PC (Queue Sortante)
     */
    void sendEvent(CommandType type, int value = 0) {
        if (_queueToPC == NULL) return;
        ControlMsg msg;
        msg.type = type;
        msg.value = value;
        xQueueSend(_queueToPC, &msg, 0); 
    }

    /**
     * @brief Vérifie les mises à jour venant du PC (Queue Entrante)
     */
    bool checkUpdate(ControlMsg* msg) {
        if (_queueToESP == NULL) return false;
        return xQueueReceive(_queueToESP, msg, 0) == pdPASS;
    }

    // Méthodes de gestion des tâches
    void handleOutgoingMessagesTask();
    void handleIncomingMessagesTask();

    void sendMessageDebug(String message) {
    ControlMsg msg;
    msg.type = DEBUG_MSG; // Ajoute DEBUG_MSG à ton enum CommandType
    msg.value = 0;
    message.toCharArray(msg.rawData, 64);
    xQueueSend(_queueToPC, &msg, 0); 
}


private:
    QueueHandle_t _queueToPC;
    QueueHandle_t _queueToESP;

    // Wrappers statiques indispensables pour FreeRTOS en C++
    static void outgoingTaskStarter(void* pvParameters);
    static void incomingTaskStarter(void* pvParameters);

    void parseSoftwareList(String list);
    void parseRGBColor(String rgb);
};





#endif