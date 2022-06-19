#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const int Entrer = 5;
const int Sortie = 4;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.fr.pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup() {
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

  // Update time with NTP
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  timeClient.update();
}

void loop() {
  
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  
  String formattedTime = timeClient.getFormattedTime();
  String currentDate = String(ptm->tm_year+1900) + "-" + String(ptm->tm_mon+1) + "-" + String(ptm->tm_mday);
  String weekDay = weekDays[timeClient.getDay()];
  
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

  delay(100);
  }
}
