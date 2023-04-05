#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define DHTTYPE DHT11 

const char* ssid = "********";
const char* password =  "********";

const char* mqttServer = "192.168.0.x";
const int mqttPort = 1883;
const char* mqtt_username = "******";
const char* mqtt_password = "******";
const char* clientID = "esp_sport_tempHumid";

#define cinema

#ifdef sport
#define TOPIC "esp_sport"
#define HOSTNAME "EspSportTempHumd"
#endif

#ifdef cinema
#define TOPIC "esp_cinema"
#define HOSTNAME "EspCinemaTempHumd"
#endif

#define INFO_TOPIC "log/info/" TOPIC
#define ERROR_TOPIC "log/error/" TOPIC
#define INIT_TOPIC "init/" TOPIC "/ready"
#define TEMP_TOPIC "cmd/" TOPIC "/temp"
#define HUMID_TOPIC "cmd/" TOPIC "/temp"

// DHT Sensor
uint8_t DHTPin = 5; //D1 
DHT dht(DHTPin, DHTTYPE);                
float Temperature;
float Humidity;

WiFiClient espClient;
PubSubClient client(espClient);

void connect() {
  //IPAddress ip(192,168,0,223);   
  //IPAddress gateway(192,168,0,1);   
  //IPAddress subnet(255,255,255,0);   
  //WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  wifiCheck();
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  client.setServer(mqttServer, mqttPort);
  mqtt();
}
void reconnect(){
  WiFi.reconnect();
  wifiCheck();
  mqtt();
}
void wifiCheck(){
  int tryIt = 30;
//  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  while (WiFi.status() != WL_CONNECTED && tryIt > 0) {
    delay(500);
    tryIt--;
  }
  if(tryIt <= 0){
    ESP.restart();
  }
}
void mqtt(){
  if (!client.connect(clientID, mqtt_username, mqtt_password)) {
    ESP.restart();
  }
  // callback
  // subscribe
  
  client.loop();
  delay(100);
}
unsigned long starttime;

void setup() { 
  Serial.begin(9600);
  Serial.println("setup");
  pinMode(DHTPin, INPUT);
  
  connect();
  Serial.println("connected");
  ArduinoOTA.setHostname(HOSTNAME);
  Serial.println("hostname set");
  
  client.publish(INFO_TOPIC, "connected");
  client.loop();
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
  });
  ArduinoOTA.begin();  
  
  client.publish(INIT_TOPIC, "ready");
  client.loop();
  dht.begin();    
}

void loop() {  
  if(WiFi.status() != WL_CONNECTED){
    reconnect();
    client.publish(ERROR_TOPIC, "Mqtt reconnected.");
  }
  if (!client.connected()) {
    mqtt();
  }
  ArduinoOTA.handle();
  client.loop();
  doTheJob();
  
  client.loop();
}
void delayWithOTA(int nbSec){
  starttime = millis();
  while(starttime + 1000 * nbSec > millis()){ //10 m
    ArduinoOTA.handle();
    delay(100);
  }
}
void doTheJob(){
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  char string_to_send[10];
  sprintf(string_to_send, "%d", (int)Temperature);
  client.publish(TEMP_TOPIC, string_to_send, true);
  sprintf(string_to_send, "%d", (int)Humidity);  
  client.publish(HUMID_TOPIC, string_to_send, true); 
  
  delayWithOTA(60*10);//10 m
}
