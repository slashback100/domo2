####################################################################
#  HC-SR04  <->   NodeMCU
#  Vcc   -------- VV (5v)
#  Trig  -------- D1
#  Echo  -------- D3
#  Gnd   -------- Gnd
#
####################################################################



#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h" // Allows us to connect to, and publish to the MQTT broker

const char* ssid = "******";
const char* password = "******";

const char* mqtt_server = "192.168.0.x";
const char* mqtt_username = "******";
const char* mqtt_password = "******";

//#define mel
#define chris

#ifdef mel
#define HOSTNAME "EspGarageMelDistance"
#define TOPIC "esp_garage_mel_dist"
#endif

#ifdef chris
#define HOSTNAME "EspGarageChrisDistance"
#define TOPIC "esp_garage_christ_dist"
#endif

#define ACTIVATE_TOPIC "cmd/" TOPIC "/activate"
#define READY_TOPIC "init/" TOPIC "/ready"
#define MAIN_TOPIC "cmd/" TOPIC "/distance"

#define INFO_TOPIC "log/info/" TOPIC
#define DEBUG_TOPIC "log/debug/" TOPIC
#define ERROR_TOPIC "log/error/" TOPIC

const char* clientID = TOPIC;
const int trigPin = 5; // D1
const int echoPin = 4; // D2
long duration;
int distance;
char activate;

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker

void connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  wifiCheck();
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  mqtt();
}
void reconnect(){
  WiFi.reconnect();
  wifiCheck();
  mqtt();
}
void wifiCheck(){
  int tryIt = 30;
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
  
  client.setCallback(subscribeReceive);
  client.subscribe(ACTIVATE_TOPIC);
  
  client.loop();
  delay(100);
}
unsigned long starttime;

void setup() {
  activate = '0';
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  connect();
  client.publish(INFO_TOPIC, "connected");
  
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
  });

  ArduinoOTA.begin();
  starttime = millis();
  
  client.publish(READY_TOPIC, "ready");
}

void subscribeReceive(char* topic, byte* payload, unsigned int length){
  // Print the message
  activate = char(payload[0]);
  if(activate == '1'){
    client.publish(DEBUG_TOPIC , "Activated");
    distance = 0; // to force a resend even if oldDistance == distance
  } else {
    client.publish(DEBUG_TOPIC, "Disactivated");
  }
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    reconnect();
    client.publish(ERROR_TOPIC, "Mqtt reconnected.");
  }
  if(!client.connected()){
    mqtt();
  }
  doTheJob();
 
  ArduinoOTA.handle();
  client.loop();
}

void doTheJob(){
 if(activate == '1'){
    int oldDistance;
    oldDistance = distance;
    readDistance();
    if(distance != oldDistance && distance <= 2000){ // send >= 2060 if no feedback is detected
    //if(distance <= 2000){ // send >= 2060 if no feedback is detected
      client.publish(MAIN_TOPIC, String(distance).c_str());
    }
    delay(100);
  } 
  if(starttime + 1000 * 300 <= millis()){ //30 s
    client.publish(DEBUG_TOPIC, "I am alive");
    starttime = millis();
  }
}
void readDistance() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH); 
  // Calculating the distance
  distance = duration * 0.034 / 2;
}
