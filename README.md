# Partie LCD 128x32 px

La librairie (LCD.h et LCD.cpp) permet d'afficher, avec deux couleurs (noir/blanc) :
- du texte
- une barre de volume
- une image

Il est possible de scinder l'écran en 3 zones :
1. zone de texte  "TEXT_ZONE"*  -  en haut de l'écran : 128x9 px
2. zone de volume "VOLUM_ZONE"* - a gauche de l'écran :   9x23px
3. zone d'image   "IMAGE_ZONE"* -    reste de l'écran : 118x23px
** sont les paramètres à utiliser pour les champs "LCDzone zone" des fonctions.

Pour cela, des fonctions dédiés sont disponibles :
0. clearZone(...)       -> affiche un rectangle noir dans la zone désigné
1. printTextInZone(...) -> affiche du texte dans la zone de text (pas de vérification de taille max)
2. drawVolumeBar(...)   -> affiche la barre de volume dans la zone de volume, en pourcentage  (0 à 100%, écrété si dépasse ces seuils)
3. drawImageInZone(...) -> affiche une image stockée dans le .h sous forme de bitmap (static const unsigned char []).

Attention à ne pas oublier d'utiliser la méthode update() pour appliquer les changements. 
-

Il est possible d'afficher du texte, une barre de volume ou des images sans scinder l'écran avec les foncions : printText(...), printVolumeBar(...) et drawImage(...).
