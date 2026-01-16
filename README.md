
PC - ESP:


Cette partie permet de contrôler le volume des applications Windows individuellement (Spotify, Chrome, Discord...) en utilisant un **ESP32** à l'aide d'un potentiomètre physique infini (**AS5600**). 

Un ruban LED **WS2811B** intégré change de couleur automatiquement pour indiquer quel logiciel est en cours de contrôle (Vert pour Spotify, Jaune pour Chrome, etc.).


Le script **PCComm.py** fait l'interface entre Windows et l'ESP32. Au lancement, une interface s'ouvre pour nous permettre de voir l'état des logiciels detectés et du son en pourcentage.

Quand on tourne le potentiomètre, le volume change, dès qu'il arrive aux extrémités (0% ou 100%), on change le sens de tournage. Quand le potentiomètre est en contact avec l'encodeur en dessous avec un aimant, le code lui demande de changer de logiciel pour modifier le son. 
