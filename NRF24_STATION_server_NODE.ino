#define NO_OF_TRAINS 5 // maximum number of train ids saved at a time
#define TIME_LIMIT_DISCONNECT 1000 // if message not received for this time the train is taken as left the station
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <RH_NRF24.h>

const char* ssid = "Dialog 4G 885";
const char* password = "kk=@QW12XX";
const char* serverName = "http://trainalert-api.herokuapp.com/departure";

int trainIDs[NO_OF_TRAINS];
long timeStamp[NO_OF_TRAINS];
boolean stored[NO_OF_TRAINS];
long lastTime;

RH_NRF24 nrf24(2,4);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

   WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  if (!nrf24.init()) {
    Serial.println("init failed");
  }

  if (!nrf24.setChannel(1)) {
    Serial.println("setChannel failed");
  }

  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("setRF failed");
  }
  for(int i=0;i<NO_OF_TRAINS;i++){
    stored[i] = false;
  }
  lastTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - lastTime > 300){
    for(int i=0;i<NO_OF_TRAINS;i++){
      if(stored[i]== true && millis() - timeStamp[i] > TIME_LIMIT_DISCONNECT){
        stored[i] = false;
        trainLeft(trainIDs[i]);
      }
    }
    lastTime = millis();
  }
  if (nrf24.available())
  {  
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len)){
      Serial.print("got request: ");
      int recID = buf[0] << 8 | buf[1];
      Serial.println(recID);
      boolean updated = false;
      for(int i=0;i<NO_OF_TRAINS;i++){
        if(stored[i]==true && trainIDs[i] == recID){
          timeStamp[i]= millis();
          updated = true;
        }
        if(updated == false){
          for(int i=0;i<NO_OF_TRAINS;i++){
            if(stored[i]== false){
              trainIDs[i]= recID;
              timeStamp[i]= millis();
              trainArrived(recID);
              stored[i] = true;
              break;
            }
          }
        }
      }
    }
    else{
      Serial.println("receive failed");
    }
  }
}

void trainArrived(int trainID){
  Serial.print("Arrived: ");
  Serial.println(trainID);

   if(WiFi.status()== WL_CONNECTED){
  
      HTTPClient http;
      http.begin(serverName);
      String id="3";
      String location="Panadura";
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"id\":\"" + id + "\",\"location\":\"" + location + "\"}");
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

 
 }else{
    Serial.println("Error in WiFi connection");
 }
  
}

void trainLeft(int trainID){
  Serial.print("Left: ");
  Serial.println(trainID);

  
}
