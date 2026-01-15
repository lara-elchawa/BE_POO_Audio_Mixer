#include "commPC.h"

commPC::commPC(HardwareSerial &serial) : _serial(serial) {}

bool commPC::init() {
  _serial.setTimeout(5000); // Très important pour ne pas bloquer readStringUntil
  delay(500);

  sendMessageDebug("Attente du signal PC...");

  unsigned long startTime = millis();
  
  const unsigned long timeout = 5000; // 5 secondes

  // Timeout
  while (millis() - startTime < timeout) {

    if (_serial.available() > 0) {
      String message = _serial.readStringUntil('\n');
      message.trim();

      if (message == "BEGIN_AUDIO_MIXER") {
        sendMessageDebug("AUDIO_MIXER_STARTED");
        return true; // Tout se passe bien

      } else {
        sendMessageDebug("Message reçu incorrect...");
        //  continue de chercher
      }
    }
  }

  // Si on arrive ici, c'est que les 5 secondes sont passées sans succès
  sendMessageDebug("Timeout lors de l'initialisation de la communication.");
  return false;
}

// On suppose que dans ton .h tu as défini : #define MAX_SOFTWARES 10

bool commPC::getSoftwaresList(
    void) { // On retire le paramètre car on utilise _ListSoftwares
  while (_serial.available()) {
    _serial.read();
  }

  _serial.println("GET_SOFTWARE_LIST");

  bool listParsed = false;
  bool ackReceived = false;
  bool colorReceived = false;

  unsigned long startTime = millis();
  const unsigned long timeout = 1000;

  while (millis() - startTime < timeout) {
    if (listParsed && ackReceived && colorReceived) {
      // MISE À JOUR DE L'INDEX : On reset l'index à 0 pour éviter de pointer
      // sur un logiciel qui n'existe plus dans la nouvelle liste
      _selectedSoftwareIndex = 0;

      sendMessageDebug("Synchronisation complete.");
      return true;
    }

    if (_serial.available() > 0) {



      String response = _serial.readStringUntil('\n');
      response.trim();

      // INTERCEPTION PRIORITAIRE DU RESET
      if (response == "NVIC_SYSTEM_RESET") {
        sendMessageDebug("REDÉMARRAGE MATÉRIEL EN COURS...");
        delay(500);         // Laisse le temps au message debug de partir
        ESP.restart(); // Redémarrage matériel
      }
      // CAS A : ACK
      if (response.indexOf("GET_SOFTWARE_LIST_ACK") != -1) {
        ackReceived = true;
        sendMessageDebug("ACK valide.");
      }

      // CAS B : Couleur -> Utilisation de la variable membre _softwareColorRGB
      else if (response.startsWith("SET_COLOR:")) {
        String rgbValues = response.substring(10);
        for (int i = 0; i < 3; i++) {
          int commaIndex = rgbValues.indexOf(',');
          if (commaIndex != -1) {
            _softwareColorRGB[i] =
                (uint8_t)rgbValues.substring(0, commaIndex).toInt();
            rgbValues = rgbValues.substring(commaIndex + 1);
          } else {
            _softwareColorRGB[i] = (uint8_t)rgbValues.toInt();
          }
        }
        colorReceived = true;
        sendMessageDebug("Couleur software mise a jour.");
      }

      // CAS C : Liste -> Utilisation de la variable membre _ListSoftwares
      else if (response.indexOf(',') != -1 &&
               !response.startsWith("SET_COLOR:")) {
        int currentCase = 0;
        int startIndex = 0;
        int endIndex = response.indexOf(',');

        while (endIndex != -1 && currentCase < MAX_SOFTWARES - 1) {
          // On remplit directement le tableau de la classe
          _ListSoftwares[currentCase] =
              response.substring(startIndex, endIndex);
          _ListSoftwares[currentCase].trim();

          startIndex = endIndex + 1;
          endIndex = response.indexOf(',', startIndex);
          currentCase++;
        }

        _ListSoftwares[currentCase] = response.substring(startIndex);
        _ListSoftwares[currentCase].trim();

        // Mise à jour du nombre de logiciels
        _numSoftwares = currentCase + 1;
        listParsed = true;
        sendMessageDebug("Tableau couleur RGB rempli.");
      }
    }
  }

  if (!listParsed)
    sendMessageDebug("Erreur : Liste non recue.");
  return listParsed;
}

void commPC::sendMessageDebug(String message) {
  _serial.println("DEBUG: " + message);
}

void commPC::sendCommandToPC(String command) {

  uint8_t maxAttempts = 3; // nombre de tentatives de renvoi de la commande,
                           // format uint8_t pour économiser de la mémoire!

  for (int i = 0; i < maxAttempts; i++) {
    while (_serial.available()) {
      _serial.read();
    }

    _serial.println(command); // *! ENVOI DE LA COMMANDE AU PC

    // Initialisation du timeout
    unsigned long startTime = millis();
    const unsigned long timeout = 100; // 1 seconde max d'attente

    bool ackReceived = false; // Pour vérifier la réception de l'ACK
    bool needsColor =
        (command == "NEXT_SOFTWARE" ||
         command ==
             "GET_SOFTWARE_LIST"); // Les deux commandes nécessitent la couleur
    bool colorReceived = false;

    while (millis() - startTime < timeout) {

      if (ackReceived && (!needsColor || colorReceived))
        return; // Si on a tout reçu ou qu'on a pas besoin d'attendre la
                // couleur, on sort de la fonction!

      if (_serial.available() > 0) {



        // traitement du texte reçu
        String response = _serial.readStringUntil('\n');
        response.trim();

        // INTERCEPTION PRIORITAIRE DU RESET
        if (response == "NVIC_SYSTEM_RESET") {
          sendMessageDebug("REDÉMARRAGE MATÉRIEL EN COURS...");
          ESP.restart(); // Redémarrage matériel
        }

        if (response == (command + "_ACK")) { // ACK RECU POUR LA COMMANDE
          ackReceived = true;
          //sendMessageDebug("ACK recu pour " + command);
        }

        // Cas B : La Couleur (Utilisation de la variable membre
        // _softwareColorRGB)
        else if (response.startsWith(
                     "SET_COLOR:")) { // Détection du début de la chaine de
                                      // reception RGB
          String rgbValues = response.substring(
              10); // Extraction des valeurs RGB de la chaine principale
          for (int i = 0; i < 3; i++) { // Boucle pour R, G, B
            int commaIndex =
                rgbValues.indexOf(','); // Recherche de la position de la
                                        // virgule et extraction de la valeur
            if (commaIndex != -1) {
              _softwareColorRGB[i] =
                  (uint8_t)rgbValues.substring(0, commaIndex)
                      .toInt(); // Conversion en entier 8 bits! ça permet de
                                // stocker 0-255 peu importe la valeur reçue,
                                // dans le pire des cas, la couleur sera
                                // mauvaise
              rgbValues = rgbValues.substring(commaIndex + 1);
            } else {
              _softwareColorRGB[i] = (uint8_t)rgbValues.toInt();
            }
          }
          colorReceived = true;
          sendMessageDebug("Couleur du logiciel mise a jour.");
        }
      }

      delay(1);
    }

    if (!ackReceived) {

      sendMessageDebug("ERREUR : Pas d'ACK pour " + command +
                       " Renvoi de la commande...");
      sendMessageDebug("Tentative " + String(i + 1) + " sur " +
                       String(maxAttempts)); // String() pour convertir les
                                             // entiers en chaînes de caractères
    }
  }
}