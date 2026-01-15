#include "realTimeCommPC.h"
#include <Arduino.h>
#include "AS5600POTAR.h"

realTimeCommPC audioMixer(Serial);
AS5600POTAR magPotar(0x36, &Wire);

/**
 * @brief Tâche Gestion Hardware (capteurs, boutons, encodeur magnétique)
 * @param pvParameters Paramètres de la tâche (non utilisés)
 */
void taskHardware(void *pvParameters) {

  ControlMsg msg; //Structure pour recevoir les messages du PC.

  for (;;) {
    
    // Si la valeur du potentiomètre a changé, on envoie la nouvelle valeur de volume au PC
    if(magPotar.VolumeUpdated()) {
        audioMixer.sendEvent(SET_VOLUME, magPotar._currentVolume);
    }
    if(magPotar.buttonUpdated()){
        audioMixer.sendEvent(NEXT_SOFTWARE,magPotar._currentButtonState);
    }
    
    // Si la valeur du bouton a changé, on envoie l'événement au PC pour lui dire de changer le logiciel : 
    
    if (audioMixer.checkUpdate(&msg)) {
      // *! FAIRE LE CODE DE GESTION DES COMMANDES REÇUES ICI
    

    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void setup() {
  Serial.begin(921600);
  Wire.begin(); 
  Wire.setClock(400000);

  delay(1500);
  Serial.println("DEBUG: [SYSTEM] Démarrage matériel...");

  
  if (!audioMixer.begin()) {
    Serial.println("DEBUG: [ERREUR] Initialisation des Queues échouée.");
    while (1)
      ;
  }

  if (!audioMixer.init()) { // *! C'est ici qu'il y a le handshake avec le PC
    Serial.println("DEBUG: [ERREUR] Handshake PC introuvable. Reset...");
    ESP.restart();
  }

  audioMixer.sendMessageDebug("Connexion PC validée.");
  audioMixer.sendEvent(GET_SOFTWARE_LIST);
  // Initialisation du potentiomètre rotatif AS5600POTAR
    if (magPotar.beginPOTAR()!= AS5600_OK) {
        audioMixer.sendMessageDebug("ERREUR: Initialisation AS5600POTAR échouée.");
        while (1)
        ;
    }
    audioMixer.sendMessageDebug("AS5600POTAR initialisé avec succès.");


      xTaskCreatePinnedToCore(taskHardware, "TaskHW", 8192, NULL, 1, NULL, 1);
  audioMixer.sendMessageDebug("Tâche Hardware lancée.");
  audioMixer.sendMessageDebug("Setup terminé, audioMixer Fonctionnel.");

}

void loop() {
  // On tue la boucle loop pour libérer des ressources
  vTaskDelete(NULL);
}