#include "commPC.h"

commPC::commPC(HardwareSerial &serial) : _serial(serial) {}

bool commPC::init() {
  _serial.setTimeout(5000); //  important pour ne pas bloquer readStringUntil
  delay(500);

  sendMessageDebug("Attente du signal PC...");

  unsigned long startTime = millis();
  const unsigned long timeout = 5000; // 5 secondes pour le handshake sinon on timeout

  while (millis() - startTime < timeout) {
    if (_serial.available() > 0) {
      String message = _serial.readStringUntil('\n');
      message.trim();

      if (message == "BEGIN_AUDIO_MIXER") {
        sendMessageDebug("AUDIO_MIXER_STARTED");
        return true;// tout va bien on est contents!
      } else {
        sendMessageDebug("Message reçu incorrect...");
      }
    }
  }

  sendMessageDebug("Timeout lors de l'initialisation de la communication.");
  return false;
}

bool commPC::getSoftwaresList(void) { 
  while (_serial.available()) { _serial.read(); }

  _serial.println("CMD_GET_SOFTWARE_LIST"); // Ajout du CMD_ pour le PC

  bool listParsed = false, ackReceived = false, colorReceived = false;
  unsigned long startTime = millis();

  while (millis() - startTime < 3000) { // On laisse 3s pour la grosse liste
    if (listParsed && ackReceived && colorReceived) {
      _selectedSoftwareIndex = 0;
      sendMessageDebug("Synchronisation complete.");
      return true;
    }

    if (_serial.available() > 0) {
      String response = _serial.readStringUntil('\n');
      response.trim();

      if (response == "NVIC_SYSTEM_RESET") { ESP.restart(); }

      // CAS A : ACK (Vérification souple du nom)
      if (response.indexOf("GET_SOFTWARE_LIST_ACK") != -1) {
        ackReceived = true;
        sendMessageDebug("ACK valide.");
      }
      // CAS B : Couleur
      else if (response.startsWith("SET_COLOR:")) {
        String rgbValues = response.substring(10);
        if (sscanf(rgbValues.c_str(), "%hhu,%hhu,%hhu", &_softwareColorRGB[0], &_softwareColorRGB[1], &_softwareColorRGB[2]) == 3) {
            colorReceived = true;
            sendMessageDebug("Couleur recue.");
        }
      }
      // CAS C : La Liste (Découpage plus robuste)
      else if (response.length() > 0 && !response.endsWith("_ACK")) {
        int count = 0;
        int lastPos = 0;
        int commaPos = response.indexOf(',');

        while (commaPos != -1 && count < MAX_SOFTWARES - 1) {
          _ListSoftwares[count] = response.substring(lastPos, commaPos);
          _ListSoftwares[count].trim();
          lastPos = commaPos + 1;
          commaPos = response.indexOf(',', lastPos);
          count++;
        }
        _ListSoftwares[count] = response.substring(lastPos); // Le dernier nom
        _ListSoftwares[count].trim();
        _numSoftwares = count + 1;
        
        listParsed = true;
        sendMessageDebug("Tableau rempli : " + String(_numSoftwares) + " softs.");
      }
    }
  }
  return listParsed;
}

void commPC::sendMessageDebug(String message) {
  _serial.println("DEBUG: " + message);
}

void commPC::sendCommandToPC(String command) {
  uint8_t maxAttempts = 3; 

  for (int i = 0; i < maxAttempts; i++) {
    while (_serial.available()) { _serial.read(); }

    _serial.println(command); // *! ENVOI AU PC ATTENTION

    unsigned long startTime = millis();
    const unsigned long timeout = 50; // 50ms seconde d'attente réelle

    bool ackReceived = false; 
    bool needsColor = (command == "NEXT_SOFTWARE" || command == "GET_SOFTWARE_LIST"); 
    bool colorReceived = false;

    while (millis() - startTime < timeout) {
      if (ackReceived && (!needsColor || colorReceived)) return; 

      if (_serial.available() > 0) {
        String response = _serial.readStringUntil('\n');
        response.trim();

        if (response == "NVIC_SYSTEM_RESET") {
          ESP.restart();
        }

        if (response == (command + "_ACK")) { 
          ackReceived = true;
        }
        else if (response.startsWith("SET_COLOR:")) {
          String rgbValues = response.substring(10);
          for (int i = 0; i < 3; i++) {
            int commaIndex = rgbValues.indexOf(',');
            if (commaIndex != -1) {
              _softwareColorRGB[i] = (uint8_t)rgbValues.substring(0, commaIndex).toInt();
              rgbValues = rgbValues.substring(commaIndex + 1);
            } else {
              _softwareColorRGB[i] = (uint8_t)rgbValues.toInt();
            }
          }
          colorReceived = true;
        }
      }
      delay(1);
    }

    if (!ackReceived) {
      sendMessageDebug("ERREUR : Pas d'ACK pour " + command + ". Tentative " + String(i + 1));
    }
  }
}