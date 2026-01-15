#ifndef REAL_TIME_UTILS_H
#define REAL_TIME_UTILS_H

#include <Arduino.h>

enum CommandType {
    VOLUME_UP,
    VOLUME_DOWN,
    MUTE,
    NEXT_TRACK,
    PREV_TRACK,
    NEXT_SOFTWARE,
    SET_VOLUME,
    GET_SOFTWARE_LIST,
    PAUSE_PLAY,
    NVIC_SYSTEM_RESET,
    DEBUG_MSG
};

//Structure pour les messages échangés entre les deux coeurs
struct ControlMsg {
    CommandType type;   
    int value;          
    char rawData[128];   // Augmenté pour contenir des messages de debug
};
//extern veut dire que la variable est définie ailleurs (dans le main)
extern QueueHandle_t qPCtoESP; // Commandes venant du PC (Core 0 -> Core 1)
extern QueueHandle_t qESPtoPC; // Événements matériel (Core 1 -> Core 0)

#endif