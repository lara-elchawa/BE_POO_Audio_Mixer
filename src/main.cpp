#include <Arduino.h>
#include "commPC.h"

commPC PCLink(Serial); 

void setup() {
    Serial.begin(115200);
    
    while(!PCLink.init()){
        delay(10);
    }

    PCLink.sendMessageDebug("Connexion OK. Demande de la liste...");
    if(PCLink.getSoftwaresList()){
        PCLink.sendMessageDebug("Liste reçue !");
        
        for(int i=0; i<PCLink._numSoftwares; i++){
            PCLink.sendMessageDebug("App " + String(i) + ": " + PCLink._ListSoftwares[i]);
        }
    } else {
        PCLink.sendMessageDebug("Erreur réception liste.");
    }

    PCLink.sendMessageDebug("--- DEBUT DU TEST AUTOMATIQUE DANS 3 SECONDES ---");
    delay(3000);
}

void loop() {


    PCLink.sendMessageDebug("TEST: Changement de logiciel (Next)");
    PCLink.sendCommandToPC(CMD_NEXT_SOFTWARE);
    delay(1500);

    PCLink.sendMessageDebug("TEST: Changement de logiciel (Next)");
    PCLink.sendCommandToPC(CMD_NEXT_SOFTWARE);
    delay(1500);



    PCLink.sendMessageDebug("TEST: Volume Down (x5)");
    for(int i=0; i<5; i++) {
        PCLink.sendCommandToPC(CMD_VOLUME_DOWN);
        delay(200); 
    }
    delay(1000);

    PCLink.sendMessageDebug("TEST: Volume Up (x5)");
    for(int i=0; i<5; i++) {
        PCLink.sendCommandToPC(CMD_VOLUME_UP);
        delay(200);
    }
    delay(1500);

    PCLink.sendMessageDebug("TEST: Mute (ON)");
    PCLink.sendCommandToPC(CMD_MUTE);
    delay(2000); 

    PCLink.sendMessageDebug("TEST: Mute (OFF)");
    PCLink.sendCommandToPC(CMD_MUTE);
    delay(1500);

    PCLink.sendMessageDebug("TEST: Set Volume 20%");
    PCLink.sendCommandToPC(String(CMD_SET_VOLUME) + ":20");
    delay(2000);

    PCLink.sendMessageDebug("TEST: Set Volume 80%");
    PCLink.sendCommandToPC(String(CMD_SET_VOLUME) + ":80");
    delay(2000);

    PCLink.sendMessageDebug("TEST: Play/Pause");
    PCLink.sendCommandToPC(PAUSE_PLAY); 
    delay(2000);    

    PCLink.sendMessageDebug("TEST: Mise à jour de la liste des logiciels");
    PCLink.getSoftwaresList();
    
    
    PCLink.sendMessageDebug("--- FIN DU CYCLE DE TEST (RESTART DANS 5s) ---");
    delay(5000);
}