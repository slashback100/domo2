#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h" // Allows us to connect to, and publish to the MQTT broker

const char* ssid = "wifi_network";
const char* password = "wj7ds232pajmn";

const char* mqtt_server = "192.168.0.186";
const char* mqtt_username = "slashback";
const char* mqtt_password = "nimda";
const char* clientID = "esp_garage_mel_dist";
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
  client.subscribe("cmd/esp_garage_mel_dist/activate");
  
  client.loop();
  delay(100);
}
unsigned long starttime;

void setup() {
  activate = '0';
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  connect();
  client.publish("log/info/esp_garage_mel_dist", "connected");
  
  ArduinoOTA.setHostname("EspGarageDistance");
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
  
  client.publish("init/esp_garage_mel_dist/ready", "ready");
}

void subscribeReceive(char* topic, byte* payload, unsigned int length){
  // Print the message
  activate = char(payload[0]);
  if(activate == '1')
    client.publish("log/debug/esp_garage_mel_dist", "Activated");
   else
    client.publish("log/debug/esp_garage_mel_dist", "Disactivated");
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    reconnect();
    client.publish("log/error/esp_garage_mel_dist", "Mqtt reconnected.");
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
    readDistance();
    client.publish("cmd/esp_garage_mel_dist/distance", String(distance).c_str());
    delay(100);
  } 
  if(starttime + 1000 * 300 <= millis()){ //30 s
    client.publish("log/debug/esp_garage_mel_dist", "I am alive");
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
