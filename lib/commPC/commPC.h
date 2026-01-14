#ifndef COMMPC_H
#define COMMPC_H

//Objectif : Pouvoir communiquer entre un PC et une carte ESP via le port série USB pour controler les pistes audio du PC via le mixer audio physique.
//Le PC envoie la liste de noms de logiciels pouvant être controlés, puis l'ESP envoie la sélection du logiciel, puis les commandes de contrôle (volume up, volume down, mute, prochaine musique, musique précédente, logiciel suivant, etc).

#include <Arduino.h>
#include <String.h>


#define MAX_SOFTWARES 10 // nombre maximum de logiciels pouvant être gérés par l'AudioMixer
// Commandes à envoyer au PC pour controler les audios des logiciels : 
#define CMD_VOLUME_UP "VOLUME_UP"
#define CMD_VOLUME_DOWN "VOLUME_DOWN"
#define CMD_MUTE "MUTE"
#define CMD_NEXT_TRACK "NEXT_TRACK"
#define CMD_PREV_TRACK "PREV_TRACK"
#define CMD_NEXT_SOFTWARE "NEXT_SOFTWARE"
#define CMD_SET_VOLUME "SET_VOLUME" // suivi du niveau de volume (0-100) qui sera récupéré par l'encoder
#define GET_SOFTWARE_LIST "GET_SOFTWARE_LIST"
#define PAUSE_PLAY "PAUSE_PLAY"
#define NVIC_SYSTEM_RESET "NVIC_SYSTEM_RESET" // commande spéciale pour redémarrer l'ESP via le PC
class commPC {
public:

    /**
     * @brief Constructeur de la classe
     * 
     * @param serial Référence à l'objet HardwareSerial pour la communication série
     */
    commPC(HardwareSerial& serial); 

    /**
     * @brief Initialisation de la communication série avec le PC
     * 
     * @return true si l'initialisation réussit, false sinon
     */
    bool init(void);

    /**
     * @brief Getter pour récupérer la liste des logiciels manipulables du PC par l'AudioMixer 
     */
    bool getSoftwaresList(void);

    /**
     * @brief Envoi d'un message de debug au PC via la liaison série (utilisé pour le développement et le dépannage)
     * 
     * @param message Message à envoyer 
     */
    void sendMessageDebug(String message); // Envoyer un message de debug au PC via la liaison série

    /**
     * @brief Envoi d'une commande au PC via la liaison série
     * 
     * @param command Commande à envoyer 
     */
    void sendCommandToPC(String command); // Envoyer une commande au PC via la liaison série

    String _ListSoftwares[MAX_SOFTWARES]; // Tableau pour stocker les noms des logiciels
    int _selectedSoftwareIndex; // Index du logiciel sélectionné
    int _currentVolume;        // Volume actuel du logiciel sélectionné
    int _setVolume;            // Volume à définir pour le logiciel sélectionné
    int _isMuted;              // État de mise en sourdine du logiciel sélectionné
    int _numSoftwares;         // Nombre de logiciels dans la liste
    uint8_t _softwareColorRGB[3]; // Tableau pour stocker les valeurs RGB du logiciel sélectionné

private:
    HardwareSerial& _serial;
    
    



};

#endif