#ifndef AS5600LIB_H
#define AS5600LIB_H

#include "AS5600.h"

//Classe fille de AS5600 pour gestion potentiometre rotatif en full I2C
class AS5600POTAR : public AS5600 {

public : 

/**
 * @brief Constructeur de la classe AS5600POTAR pour transformer le capteur AS5600 en potentiomètre magnétique rotatif.
 * 
 * @param address Adresse I2C du capteur (par défaut 0x36)
 * @param wire    Pointeur vers l'objet TwoWire utilisé pour la communication I2C (par défaut Wire)
 */

AS5600POTAR(uint8_t address, TwoWire *wire);


/** 
 * @brief Méthode pour initialiser le potentiomètre. 
 * @return int Code de retour (0 si succès, -1 si erreur)
 */
int beginPOTAR(void);


/**
 * @brief Récupère l'angle mesuré par le capteur AS5600 en degrés.
 * 
 * @return valeur entière représentant l'angle en degrés (0-360) 
 */
int getAngleDeg(void);

/**
 * @brief Récupère la valeur entre 1-100 correspondant à la position du potentiomètre pour convertir en % de volume sur le PC. 
 * 
 * @return Valeur ∈ [0;100]
 */
int getValue(void);

/**
 * @brief Définit la plage de rotation (en degrés) nécessaire pour atteindre 100%.
 *        Par défaut 360° (une révolution). Exemples :
 *          setRotationRangeDegrees(720); // deux tours pour atteindre 100%
 */
void setRotationRangeDegrees(float degrees);
float getRotationRangeDegrees(void) const;
void setRotationRangeRevolutions(float revs);

/**
 * @brief Calibre la position actuelle comme 0% (réinitialise la référence).
 */
void calibrateZero(void);

/**
 * @brief Lit l'état du bouton poussoir magnétique en fonction du champ magnétique détecté.
 * 
 * @return true 
 * @return false 
 */
bool isPressed(void);

/**
 * @brief Définit le seuil haut de déclenchement du bouton poussoir magnétique.
 * 
 * @param threshold Valeur flottante représentant le seuil haut en mT
 * @return int Code de retour (0 si succès, -1 si erreur)
 */
int setMagFieldHighThreshold(float threshold); //Définition du seuil haut de déclenchement du bouton

/**
 * @brief Définit le seuil bas de déclenchement du bouton poussoir magnétique.
 * 
 * @param threshold Valeur flottante représentant le seuil bas en mT
 * @return int Code de retour (0 si succès, -1 si erreur)
 */
int setMagFieldLowThreshold(float threshold); //Définition du seuil bas de déclenchement du bouton


private :
        uint8_t _address; //Adresse I2C du capteur par défaut : 0x36
        float _magFieldHighThreshold = 2000; //Seuil haut de déclenchement du bouton
        float _magFieldLowThreshold = 1000; //Seuil bas de déclenchement du bouton

        // Calibration de la plage du potentiomètre (défaut = 360° pour 0->100%)
        float _rotationRangeDegrees = 360.0f;
        int32_t _basePosition = 0; // position de référence (raw cumulative) pour 0%


};




#endif // AS5600LIB_H