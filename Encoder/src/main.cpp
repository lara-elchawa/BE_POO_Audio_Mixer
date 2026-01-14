#include <Arduino.h>
#include "AS5600POTAR.h"
#include <Wire.h>

AS5600POTAR potar(0x36, &Wire);

void setup() {

  Serial.begin(115200);
  Wire.begin(21, 22);
  Serial.printf("Initialisation du potentiomètre rotatif AS5600POTAR...\n");

//Initialisation du potentiomètre rotatif
if(potar.beginPOTAR()!=AS5600_OK)
  {
    Serial.println("Erreur d'initialisation du potentiomètre rotatif AS5600POTAR.\n");
    while(1); // boucle infinie
  }
  Serial.println("Potentiomètre rotatif AS5600POTAR initialisé avec succès.\n");
}


void loop() {
// Test de lecture de l'angle et affichage 0-100 pour Teleplot
Serial.print(">Angle_Deg:");
Serial.println(potar.getAngleDeg());

Serial.print(">Valeur_0_100:");
Serial.println(potar.getValue());

// Pour le bouton (1 pour pressé, 0 pour relâché)
Serial.print(">Bouton:");
Serial.println(potar.isPressed() ? 1 : 0);

Serial.print("> Cumulative Position:");
Serial.println(potar.getCumulativePosition());

Serial.print("> magnetic Field:");
Serial.println(potar.readMagnitude());

Serial.print("Bouton état: ");
if (potar.isPressed()) {
    Serial.println("Pressé");
} else {
    Serial.println("Relâché");
}
delay(10);
}
