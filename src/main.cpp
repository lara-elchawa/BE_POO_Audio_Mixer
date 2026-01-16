#include "realTimeCommPC.h"
#include <Arduino.h>
#include "AS5600POTAR.h"
#include "RGB_LED.h"
#include "LCD.h"


realTimeCommPC audioMixer(Serial);//objet qui communique en temps réel avec le PC, attention la task pour le core 0 est initialisée dedans
AS5600POTAR magPotar(0x36, &Wire);//potentiomètre magnétique
LcdDriver myLcd(21, 22);

// LEDS
CRGB ledsArray[3]; // Tableau de 4 LEDs
dynamicLEDS<3> leds(ledsArray); // Création de l'objet avec la taille 4
// boutons sur 18 vert et 19 blanc, INPUT_PULLDOWN, pour allumer le vert c'est le 16, 17 pour blanc







/**
 * @brief Tâche Gestion Hardware (capteurs, boutons, encodeur magnétique)
 * @param pvParameters Paramètres de la tâche (non utilisés)
 */
void taskHardware(void *pvParameters) {
const char* testCmds[] = {CMD_PAUSE_PLAY, CMD_NEXT_TRACK, CMD_PREV_TRACK, CMD_MUTE, CMD_VOLUME_UP, CMD_VOLUME_DOWN};
  uint8_t testRGB[3][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};

  for (int i = 0; i < 3; i++) {
    leds.updateFromPC(testRGB[i][0], testRGB[i][1], testRGB[i][2]);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  for (int i = 0; i < 6; i++) {
    myLcd.clear();
    myLcd.printText(0, 0, "TEST:");
    myLcd.printText(0, 15, testCmds[i]);
    myLcd.update();

    audioMixer.sendCommandToPC(testCmds[i]);
    
    leds.updateFromPC(255, 255, 255);
    vTaskDelay(pdMS_TO_TICKS(1000));
    leds.updateFromPC(0, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  myLcd.clear();
  myLcd.printText(0, 10, "TEST OK");
  myLcd.update();
  vTaskDelay(pdMS_TO_TICKS(1000));

  ControlMsg msg;
  bool refreshNeeded = true;


  for (;;) {
    if(magPotar.VolumeUpdated()) {//Détecte si le volume doit être changé
        audioMixer.sendEvent(SET_VOLUME, magPotar._currentVolume);
        refreshNeeded = true; //on refresh l'affichage pour mettre à jour la barre de son
    }

    if(magPotar.buttonUpdated()){
        audioMixer.sendEvent(NEXT_SOFTWARE);
        if (audioMixer._numSoftwares > 0) {
            audioMixer._selectedSoftwareIndex = (audioMixer._selectedSoftwareIndex + 1) % audioMixer._numSoftwares;//on revient au début grâce au modulo!
        }
        leds.updateFromPC(audioMixer._softwareColorRGB[0], 
                          audioMixer._softwareColorRGB[1], 
                          audioMixer._softwareColorRGB[2]);

        refreshNeeded = true;
    }

    // if (audioMixer.checkUpdate(&msg)) {
    //     leds.updateFromPC(audioMixer._softwareColorRGB[0], 
    //                       audioMixer._softwareColorRGB[1], 
    //                       audioMixer._softwareColorRGB[2]);
    //     refreshNeeded = true;
    // }

    if (refreshNeeded) {//Refresh de l'écran
        myLcd.clear();
        
        myLcd.drawVolumeBar(VOLUM_ZONE, magPotar._currentVolume);//barre de son
        
        if (audioMixer._numSoftwares > 0) {//nom du software
            String name = audioMixer._ListSoftwares[audioMixer._selectedSoftwareIndex];
            myLcd.printTextInZone(TEXT_ZONE, 0, 0, name.c_str());
        }

        //myLcd.drawImageInZone(IMAGE_ZONE, 0, 0, SpotifyIcon, 40, 40); //image par défaut
        
        myLcd.update(); // MAJ de l'écran, attention ça met du temps
        refreshNeeded = false;//flag down pour le refresh
    }

    vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}

void setup() {
  Serial.begin(921600);
  Serial.setRxBufferSize(2048);
  Wire.begin(); 
  Wire.setClock(400000);

  delay(1500);
  Serial.println("DEBUG: [SYSTEM] Démarrage matériel...");


  if (!myLcd.begin()) {
    audioMixer.sendMessageDebug("ERREUR: LCD non détecté.");
  } else {
    myLcd.clear();
    myLcd.printText(0, 0, "AudioMixer Ready");
    myLcd.update();
  }

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


    // *! init des LEDS : 
    leds.begin();
    
//Récupération des soft dispo et affichage : 
 
if (audioMixer.getSoftwaresList()) {
    audioMixer.sendMessageDebug("Liste recupérée : " + String(audioMixer._numSoftwares) + " softs.");
  } else {
    audioMixer.sendMessageDebug("ERREUR : Impossible de recup la liste.");
  }


pinMode(17,OUTPUT);
pinMode(16,OUTPUT);
  digitalWrite(17,LOW); 
   digitalWrite(16,LOW); 


     xTaskCreatePinnedToCore(taskHardware, "TaskHW", 8192, NULL, 1, NULL, 1);
}

void loop() {
  // On tue la boucle loop pour libérer des ressources
  vTaskDelete(NULL);
}