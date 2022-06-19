# **<span style="text-decoration:underline;">Réalisation d'un compteur d'entrées/sorties afin de mesurer la fréquentation d'une bibliothèque</span>**

## **Le projet**
Pour répondre à la demande du client sur le besoin de détecteurs pour les entrées et sorties de la médiathèque, nous avons choisi des capteurs infrarouges E18-D50NK avec un ESP8266. L'utilisation de capteur infrarouges permettait de détecter le passage sur une trentaines de centimètre. Son utilisation était adapté afin de comptabiliser les entrées et sorties depuis une porte ou un portique de sécurité. 

La demande initiale était d’envoyer le fichier en SFTP directement depuis l'ESP. Après différentes recherches, nous avons découvert que l'implémentation du SFTP n’était pas possible sur les ESP par manque de puissance de calcul. Nous avons décidé de nous diriger vers une autre approche qui consistait à gérer cet envoi depuis un script python sur une machine distante. Le but étant d'envoyer la ligne à ajouter dans le fichier CSV de l'ESP vers le script python. Ainsi, nous avons donc créé un serveur Web python afin de receptionner des requêtes POST envoyées depuis l'ESP.

Après plusieurs essais, nous avons aussi remarqué que l’ESP8266 n’était pas assez puissant pour envoyer des données en HTTPS avec autre chose que du SHA-1. Cette méthode n’étant pas sécurisée, nous avons donc choisi d’envoyer les données en HTTP pour ne pas complexifier le code inutilement.

Voici le schéma de fonctionnement :

![This is an image](schema.png)

## **Procédure**
Nous avons choisi d’utiliser Arduino pour coder sur l’ESP8266. Pour commencer à utiliser l’ESP8266, il fallait flasher le contrôleur. Avant de le flasher nous devions installer les drivers nécessaires sur nos machines : [https://cityos-air.readme.io/docs/1-usb-drivers-for-nodemcu-v10](https://cityos-air.readme.io/docs/1-usb-drivers-for-nodemcu-v10)

Il était important de bien vérifier la version du NodeMCU utilisée par l'ESP afin d'installer le bon driver. A noter que nous avons passé un bon moment à essayer de comprendre pourquoi il était impossible de voir l’ESP sur les ports COM de nos machines même avec le bon pilote d'installé. Le problème venait du câble USB qui était un câble de chargeur de téléphone et non pas un câble DATA.

Une fois que le NodeMCU était visible et connecté au PC, nous avons suivi la procédure suivante pour flasher le micro-contrôleur: [https://create.arduino.cc/projecthub/pratikdesai/flash-firmware-on-esp8266-esp-01-module-e1f758](https://create.arduino.cc/projecthub/pratikdesai/flash-firmware-on-esp8266-esp-01-module-e1f758)

Ensuite, nous avons utilisé le logiciel Arduino IDE pour envoyer notre code sur l’ESP. Nous avons dû l'intégrer manuellement au logiciel car il n’était pas inclus par défaut. Nous avons suivi ce tutoriel : [https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/). **Randomnerdtutorials** est un site très pratique car il recense différents projets avec des ESP.

Pour la partie code nous avons tout d'abord travaillé sur celui de l'ESP afin de le connecter au Wi-Fi. Ensuite, nous avons travaillé sur la récupération des informations des capteurs et enfin sur l'envoi des données récupérés par grâce aux capteurs vers notre serveur Web python. A savoir, qu’il était nécessaire que l’ESP récupère la date et l’heure correct via un serveur NTP afin d'effectuer les envois périodiquement. 

En seconde partie nous avons travaillé sur la création du serveur Web python en HTTP, puis sur le traitement des données reçues avec nottament la création du fichier CSV et enfin sur l'envoi vers le serveur SFTP.

## **Code de l'ESP8266**
Code Wi-Fi:
```cpp
// Initialize Serial Monitor
Serial.begin(115200);

// Connect to Wi-Fi
WiFi.begin("SSID", "PASSWORD");
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

Serial.println();
Serial.print("Connected, IP address: ");
Serial.println(WiFi.localIP());
```

Code pour synchro NTP:
```cpp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.fr.pool.ntp.org");

// Update time with NTP
timeClient.begin();
timeClient.setTimeOffset(7200);
timeClient.update();
```

Code capteurs:
```cpp
int sensorValueEntrer= digitalRead(Entrer);
int sensorValueSortie = digitalRead(Sortie);
int firstDetectedTime;

// If someone is in front both sensor at the same time it restarts the loop
if(sensorValueEntrer == 0 and sensorValueSortie == 0){
  delay(500);
  loop();
}
else {

// If first sensor is triggered first, it means that someone can enter
if(sensorValueEntrer == 0){
  firstDetectedTime = millis();

  // It tries to detect during 1 second if the second sensor is triggered
  do {
    sensorValueSortie = digitalRead(Sortie);
    if(sensorValueSortie == 0){
      Serial.println("Entrée");
      break;
    }
  } while (firstDetectedTime + 1000 > millis());

  delay(500);
  loop();
}

// If second sensor is triggered first, it means that someone can go out
if(sensorValueSortie == 0){
  firstDetectedTime = millis();

  // It tries to detect during 1 second if the first sensor is triggered
  do {
    sensorValueEntrer= digitalRead(Entrer);
    if(sensorValueEntrer == 0){
      Serial.println("Sortie");
      break;
    }
  } while (firstDetectedTime + 1000 > millis());
  delay(500);
  loop();
}
delay(100);
```

Code pour le HTTP POST vers le serveur Web python :
```cpp
time_t epochTime = timeClient.getEpochTime();
struct tm *ptm = gmtime ((time_t *)&epochTime); 

String formattedTime = timeClient.getFormattedTime();
String currentDate = String(ptm->tm_year+1900) + "-" + String(ptm->tm_mon+1) + "-" + String(ptm->tm_mday);
String weekDay = weekDays[timeClient.getDay()];
  
// Enter the if statement only if the hours are between 8h and 19h and every 30 minutes
if (((timeClient.getHours() >= 8) || (timeClient.getHours() < 19)) && ((timeClient.getMinutes() == 30) || (timeClient.getMinutes() == 0)) && timeClient.getSeconds() == 00){
  String csv_entry = currentDate + ',' + formattedTime + ',' + 0 + ',' + 0 + ',' + "ALLI1,Counter A,RDCentre,Alliance,ALLI";

  // Check if we are connected to the wifi
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    // The IP address and the port of the web server to connect
    http.begin(client, "http://192.168.43.133:8080/");

    // Specify the content-type header
    http.addHeader("Content-Type", "text/plain");

    // Send an HTTP POST request with the csv string
    int httpResponseCode = http.POST(csv_entry);

    // Check if the request have been send successfully
    if (httpResponseCode != 200)
    {
      Serial.println("Can't send POST request to the web server");
    }

    else {
      Serial.println("POST request send to the web server");
    }

    // Free resources
    http.end();
  } 

  else {
    Serial.println("WiFi Disconnected");
  }
delay(1000);
}
```
## **Code du serveur Web**

Serveur HTTP :
```python
from http.server import BaseHTTPRequestHandler, HTTPServer

class handler(BaseHTTPRequestHandler):
  def do_POST(self):
    message = "OK"
    self.wfile.write(message.encode("utf-8"))

    self.send_response(200)
    self.send_header('Content-type','text/html')
    self.end_headers()

with HTTPServer(('192.168.43.133', 8080), handler) as server:
  server.serve_forever()
```

Création du fichier CSV et envoi vers le serveur SFTP : 
```python
from ftplib import FTP_TLS, error_perm
import datetime

try:
  ftp = FTP_TLS('192.168.43.254', user='client', passwd='')
  ftp = ftp.prot_p()

except error_perm as e:
  print(e)
  self.send_response(403)

print("Connected to FTP server")

mydate = datetime.datetime.now()
filename = mydate.strftime("%d_%m_%Y_counterdata-1.csv")

with open(filename, "a") as f:
  f.write(post_body)

file = open(filename, 'rb')
ftp.storbinary(f'STOR {filename}', file)
file.close()
ftp.quit()
```

### **Utilisation du serveur Web**
```python
python server_detector.py
```
