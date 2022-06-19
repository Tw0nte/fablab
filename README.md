# **<span style="text-decoration:underline;">Réalisation</span>**


# **<span style="text-decoration:underline;">d'un compteur pour mesurer la</span>**


# **<span style="text-decoration:underline;">fréquentation en bibliothèque</span>**


# **Le projet**

Pour répondre à la demande du client sur le besoin de détecteurs pour les entrées et sorties de la médiathèque, nous avons choisi des capteurs infrarouges E18-D50NK avec un ESP8266.

L'utilisation de capteur infrarouges permet de détecter le passage sur une trentaines de centimètre, il est adapté pour comptabiliser les entrées depuis une porte ou un portique de sécurité.

Les données sont envoyé sur un poste sur lequel un script python est chargé d’écouter sur un port les requète de l’ESP pour ensuite traiter les données


# **Procédure**

Nous avons choisi d’utiliser Arduino pour coder sur l’ESP8266. 

Pour commencer à utiliser l’ESP8266, il faut flasher le contrôleur. Avant de le flasher il faut installer les drivers nécessaires sur son PC. 

[https://cityos-air.readme.io/docs/1-usb-drivers-for-nodemcu-v10](https://cityos-air.readme.io/docs/1-usb-drivers-for-nodemcu-v10)

Bien vérifier que la version de son NodeMCU pour prendre le bon driver!

**Nous avons passé un bon moment à essayer de comprendre pourquoi il était impossible de voir l’ESP8266 avec notre PC même avec le bon pilote installé. Le problème venait du câble usb qui était un câble de chargeur de téléphone et non pas un câble data!**

Une fois que le NodeMCU est visible et connecté au PC, nous avons suivi la procédure suivante pour flasher le micro-contrôleur:

[https://create.arduino.cc/projecthub/pratikdesai/flash-firmware-on-esp8266-esp-01-module-e1f758](https://create.arduino.cc/projecthub/pratikdesai/flash-firmware-on-esp8266-esp-01-module-e1f758)

Une fois flashé, nous avons utilisé le logiciel Arduino IDE pour envoyer notre code sur l’ESP8266. Il a ensuite fallu intégrer manuellement l’ESP8266 au logiciel car il n’est inclus par défaut. 

Pour télécharger ARDUINO IDE: [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

Pour intégrer l’ESP8266 à Arduino IDE: [https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/). \
**Randomnerdtutorials** est un site très pratique car il recense différents projets avec des ESP.

Nous avons ensuite travaillé sur les codes pour connecter l’ESP8266 au Wi-Fi, récupérer les informations des capteurs et compiler les données dans un fichier qui est ensuite envoyé en .csv sur un serveur HTTP. A savoir qu’il est nécessaire que l’ESP récupère la date et l’heure via NTP pour faire l’envoi en HTTP.

Code Wi-Fi:

Code capteurs:

Code création et update fichier de données:

Code pour synchro NTP:

Envoi du fichier .csv en HTTP: 

La demande initiale était d’envoyer le fichier en SFTP. Après quelques recherches, nous avons découvert que SFTP n’est pas implémenté avec l’ESP8266.

Nous nous sommes ensuite intéressés au HTTPS. Après plusieurs essais, nous avons conclu que l’ESP8266 n’est pas assez puissant pour envoyer des données avec autre chose que du HTTPS en SHA-1. Cette méthode n’étant pas sécurisée, nous avons choisi d’envoyer le fichier en HTTP pour plus de simplicité.

Code HTTP:
