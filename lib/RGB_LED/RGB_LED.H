#ifndef RGB_LED_H
#define RGB_LED_H

#include <FastLED.h>

// --- CONFIGURATION MATÉRIELLE ---
#define LED_PIN      4
#define LED_TYPE     WS2811
#define COLOR_ORDER  GRB    // <--- FIXÉ SUR GRB COMME DEMANDÉ
// --------------------------------

template<size_t SIZE> 
class dynamicLEDS {
private:
    CRGB (&_leds)[SIZE];

public:
    dynamicLEDS(CRGB (&ledsArray)[SIZE]) : _leds(ledsArray) {}

    /**
     * @brief Initialisation du contrôleur FastLED
     */
    void begin() {
        // Le template utilise COLOR_ORDER pour savoir comment envoyer les bits
        FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(_leds, SIZE);
        FastLED.setBrightness(255);
        clear();
    }

    /**
     * @brief Applique la couleur RGB reçue du PC
     */
    void updateFromPC(uint8_t r, uint8_t g, uint8_t b) {
        // On crée un objet CRGB(R,G,B), FastLED le convertira en GRB en interne
        fill_solid(_leds, SIZE, CRGB(r, g, b));
        FastLED.show();
    }

    void clear() {
        fill_solid(_leds, SIZE, CRGB::Black);
        FastLED.show();
    }
};

#endif