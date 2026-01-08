#include "AS5600POTAR.h"

//Constructeur de la classe AS5600POTAR
AS5600POTAR::AS5600POTAR(uint8_t address, TwoWire *wire) : AS5600(wire) {
    _address = address;
}

//initialisation du potentiomètre
int AS5600POTAR::beginPOTAR(void) {
    if (!AS5600::begin()) {
        _error = AS5600_OK; // Code d'erreur pour échec de l'initialisation
        return _error; // Retourne -1 en cas d'erreur d'initialisation
    }
    else{

    setRotationRangeRevolutions(3.0f); // 
    setHysteresis(AS5600_HYST_LSB2); // Évite que la valeur saute entre deux points
    setSlowFilter(AS5600_SLOW_FILT_4X); // Lisse la lecture

    // Calibre la position actuelle comme 0%
    calibrateZero();

    return _error; // Retourne 0 si l'initialisation est réussie
    }
}

//Lecture de l'angle en degrés (0-360)
int AS5600POTAR::getAngleDeg(void) {
    int angle = readAngle();
    if (_error != AS5600_OK) {
        return _error; // Retourne _error en cas d'erreur de lecture
    }
    return map(angle, 0, 4095, 0, 360); // Conversion de l'angle en degrés
}

//Lecture de la valeur en pourcentage (0-100)
int AS5600POTAR::getValue(void) {

    // Utilise la position cumulative pour permettre plusieurs tours.
    int32_t pos = getCumulativePosition(true); // met à jour la lecture
    if (_error != AS5600_OK) {
        return _error; // Retourne _error en cas d'erreur de lecture
    }

    // delta = pos - base : distance parcourue depuis la référence
    int32_t delta = pos - _basePosition;

    // Si la position cumulative est inférieure ou égale à la référence, on considère 0% (pas de rotation positive)
    if (delta <= 0) {
        delta = 0;
        return 0;
    }

    uint32_t maxRaw = (uint32_t)round(_rotationRangeDegrees * AS5600_DEGREES_TO_RAW);
    if (maxRaw == 0) maxRaw = 4095*4;
    

int value = 0;

// On ne calcule la valeur que si maxRaw est supérieur à un seuil de sécurité
if (maxRaw > 0) {
    value = map((long)constrain(delta, 0L, (long)maxRaw), 0L, (long)maxRaw, 0, 100);
} 
else {
    // Si maxRaw est entre -600 et 0, ou simplement négatif
    value = 0; 
}
    return value; // Conversion de la position en pourcentage
} 


//Lecture de l'état du bouton poussoir magnétique
bool AS5600POTAR::isPressed(void) {

    int16_t magField = readMagnitude();
    if (_error != AS5600_OK) {
        return false; // Retourne false en cas d'erreur de lecture
    }
    else{
        if(magField >= _magFieldHighThreshold){
            return true; // Bouton pressé
        }
        else if(magField <= _magFieldLowThreshold){
            return false; // Bouton relâché
        }
    }


    return false; // État inchangé
}


// --- Calibration / sensibilité ---
void AS5600POTAR::setRotationRangeDegrees(float degrees)
{
    if (degrees <= 0) return;
    _rotationRangeDegrees = degrees;
}

float AS5600POTAR::getRotationRangeDegrees(void) const
{
    return _rotationRangeDegrees;
}

void AS5600POTAR::setRotationRangeRevolutions(float revs)
{
    setRotationRangeDegrees(revs * 360.0f);
}

void AS5600POTAR::calibrateZero(void)
{
    // mémorise la position cumulative actuelle comme 0%
    _basePosition = getCumulativePosition(true);
    if (_error != AS5600_OK) {
        _basePosition = 0; // fallback
    }
}


