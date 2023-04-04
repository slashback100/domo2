#include <SPI.h>                  // For networking
#include <Ethernet.h>             // For networking
#include <PubSubClient.h>         // For MQTT
//#include <TimerOne.h>             // For Watchdog
#include <Wire.h>              // For I2C (variator)


//#define DEBUGSERIAL
#define ETAGE1A
#define idLength 7
#ifdef ETAGE0A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
IPAddress ip(192,168,0,203);
const char* arduinoId = "etage0a";
/* ------------- button management -------------------------------- */
const unsigned int nbOutput = 16;
const unsigned int nbInput = 40;
const unsigned int nbPir = 0;
static boolean lastButtonState[nbInput] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
static unsigned int lastButtonLevel[nbInput] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH, HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//A0 is 54, A1 is 55, ...
static unsigned int buttonArray[nbInput] = {14,15,16,17,18,19,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46,47,A0,A1,A2,A3,A4,A5,A7,A8,A9};//20 & 21 are for i2c (SDL & SDA)
static unsigned int watchdogResetPin = A10;
//static boolean lastPirState[nbPir]; // = {0};
static unsigned int pirArray[nbPir]; // = {44};
static unsigned int outputArray[nbOutput] = {1,2,3,4,5,6,7,8,9,11,12,13,48,49,A14,A15}; // A14 = 68
#endif
#ifdef ETAGE1A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,200);
const char* arduinoId = "etage1a";
/* ------------- button management -------------------------------- */
const unsigned int nbOutput = 18;
const unsigned int nbInput = 42;
const unsigned int nbPir = 0;
static boolean lastButtonState[nbInput] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
static unsigned int lastButtonLevel[nbInput] = {HIGH,  HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH, HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//A0 is 54, A1 is 55, ...
static unsigned int buttonArray[nbInput] = {1,2,3,4,5,6,7,8,9,14,15,16,17,18,19,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
//0 is RX, 1 is TX
//10 cant be used,
//13 is led
//20 & 21 are for i2c (SDL & SDA),
//49 for arduino reset
//50 -> 53 is for ethernet
static unsigned int watchdogResetPin = 49;
static unsigned int pirArray[nbPir];
static unsigned int outputArray[nbOutput] = {11, 12, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};
#endif
#ifdef ETAGE2A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ip(192,168,0,201);
const char* arduinoId = "etage2a";
/* ------------- button management -------------------------------- */
const unsigned int nbOutput = 16;
const unsigned int nbInput = 40;
const unsigned int nbPir = 0;
static boolean lastButtonState[nbInput] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
static unsigned int lastButtonLevel[nbInput] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH, HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//A0 is 54, A1 is 55, ...
static unsigned int buttonArray[nbInput] = {14,15,16,17,18,19,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46,47,A0,A1,A2,A3,A4,A5,A7,A8,A9};//20 & 21 are for i2c (SDL & SDA)
static unsigned int watchdogResetPin = A10;
//static boolean lastPirState[nbPir]; // = {0};
static unsigned int pirArray[nbPir]; // = {44};
static unsigned int outputArray[nbOutput] = {1,2,3,4,5,6,7,8,9,11,12,13,48,49,A14,A15}; // A14 = 68
#endif
#ifdef ETAGE2B
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress ip(192,168,0,202);
const char* arduinoId = "etage2b";
/* ------------- button management -------------------------------- */
const unsigned int nbOutput = 16;
const unsigned int nbInput = 40;
const unsigned int nbPir = 0;
static boolean lastButtonState[nbInput] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
static unsigned int lastButtonLevel[nbInput] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH, HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//A0 is 54, A1 is 55, ...
static unsigned int buttonArray[nbInput] = {14,15,16,17,18,19,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46,47,A0,A1,A2,A3,A4,A5,A7,A8,A9};//20 & 21 are for i2c (SDL & SDA)
static unsigned int watchdogResetPin = A10;
//static boolean lastPirState[nbPir]; // = {0};
static unsigned int pirArray[nbPir]; // = {44};
static unsigned int outputArray[nbOutput] = {1,2,3,4,5,6,7,8,9,11,12,13,48,49,A14,A15}; // A14 = 68
#endif
#ifdef ETAGE3A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xFA };
IPAddress ip(192,168,0,204);
const char* arduinoId = "etage3a";
/* ------------- button management -------------------------------- */
const unsigned int nbOutput = 16;
const unsigned int nbInput = 40;
const unsigned int nbPir = 0;
static boolean lastButtonState[nbInput] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
static unsigned int lastButtonLevel[nbInput] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH, HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH};
//A0 is 54, A1 is 55, ...
static unsigned int buttonArray[nbInput] = {14,15,16,17,18,19,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46,47,A0,A1,A2,A3,A4,A5,A7,A8,A9};//20 & 21 are for i2c (SDL & SDA)
static unsigned int watchdogResetPin = A10;
//static boolean lastPirState[nbPir]; // = {0};
static unsigned int pirArray[nbPir]; // = {44};
static unsigned int outputArray[nbOutput] = {1,2,3,4,5,6,7,8,9,11,12,13,48,49,A14,A15}; // A14 = 68
#endif

/*--------------------------- Variator -----------------------------*/
volatile unsigned int val; // variable used by master to send data to slave

/*--------------------------- Network ------------------------------*/
/* Network Settings */
EthernetClient ethclient;

/*--------------------------- MQTT ------------------------------*/
IPAddress broker(192,168,0,186);
PubSubClient client(ethclient);
boolean messageReceived = false;
char receivedTopic[50];
char receivedPayload[50];
char buffer[50];
char buffer2[100];
char topic[50];
/*-------------------------- log mngmnt ----------------------------*/
boolean debug = false;
void logError(char* msg){
    sprintf(topic, "log/%s/error", arduinoId);
    sendMessage(topic, msg);
}
void logInfo(char* msg){
    sprintf(topic, "log/%s/info", arduinoId);
    sendMessage(topic, msg);
}
void logDebug(char* msg){
    sprintf(topic, "log/%s/debug", arduinoId);
    sendMessage(topic, msg);
}
void log(char* level, char* msg){
    sprintf(topic, "log/%s/%s", arduinoId, level);
    sendMessage(topic, msg);
}

long lastActivityTime = 0;

boolean watchdogStatus;
#define DEBOUNCE_DELAY 50 // button push delay

/* ------------- MQTT in -------------------------------- */
void callback(char* topic, byte* payload, unsigned int l){
  if(!messageReceived){
    messageReceived=true;
    strcpy(receivedTopic, topic);
    strncpy(receivedPayload, (char*) payload, l);
    receivedPayload[l] = '\0';
  }
}

void sendMessage(char* topic, char* message){
  client.publish(topic, message);
}

void sendCallback(char* t){
    sprintf(topic, "callback");
    sendMessage(topic,  t);
}

/* ------------- MQTT connection -------------------------------- */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef DEBUGSERIAL
    Serial.print(F("Attempting MQTT connection..."));
    #endif
    // Attempt to connect
    //strcpy(buffer, "Reconnecting Arduino-");
    //strcat(buffer; Ethernet.localIP());

    //String clientString = "Reconnecting Arduino-" + String(Ethernet.localIP());
    //clientString.toCharArray(buffer, clientString.length()+1);

    if (client.connect(arduinoId, "slashback", "nimda")) {
      #ifdef DEBUGSERIAL
      Serial.println(F("connected"));
      #endif
      // Once connected, publish an announcement...
      // ... and resubscribe;

      sprintf(topic, "cmd/%s/out/#", arduinoId);
      client.subscribe(topic);

      sprintf(topic, "cmd/%s/resetWatchdog", arduinoId);
      client.subscribe(topic);

      sprintf(topic, "cmd/%s/debug", arduinoId);
      client.subscribe(topic);

      sprintf(topic, "init/%s/ready", arduinoId);
      sprintf(buffer2, "ready");
      sendMessage(topic, buffer2);
    } else {
      #ifdef DEBUGSERIAL
      Serial.print(F("failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));
      #endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* -------------------- Input ------------------------*/
/*
void processPir(int pirId){
  unsigned int sensorReading = digitalRead(pirArray[pirId]) == HIGH ? 1 : 0;
  //if((sensorReading == 0 && lastPirState[pirId] == 1) || // was high
  //  (sensorReading == 1 && lastPirState[pirId] == 0)) {  // change of state
  if(sensorReading ^ lastPirState[pirId]){ // XOR
      if (sensorReading == 1) {
        sendMessage("cmd/"+arduinoId+"/in/"+pirArray[pirId], "ON");
      } else {
        sendMessage("cmd/"+arduinoId+"/in/"+pirArray[pirId], "OFF");
      }
      lastPirState[pirId] = sensorReading;
  }
}*/
void processButtonDigital(int buttonId){
  unsigned int sensorReading = digitalRead(buttonArray[buttonId]);
  if (sensorReading != lastButtonLevel[buttonId]){
    //if(debug) log("info", "sensor!=lastbutton (" + String(buttonId)+")");
    if(debug) {
      sprintf(buffer2, "sensor!=lastbutton (%d)", buttonId);
      logDebug(buffer2);
    }
    lastActivityTime = millis();
  }
  if((sensorReading == LOW && !lastButtonState[buttonId]) /*was high*/ ||
    (sensorReading == HIGH && lastButtonState[buttonId])) {  // potential change of state
    //if(debug) log(F("info"), "Change of state? (" + String(buttonId)+")");
    if(debug) {
      sprintf(buffer2, "Change of state? (%d)", buttonId);
      logDebug(buffer2);
    }
    if((millis() - lastActivityTime) > DEBOUNCE_DELAY){  // Proceed if we haven't seen a recent event on this button
      if(debug) {
        sprintf(buffer2, "Change of state! (%d)", buttonId);
        logDebug(buffer2);
      }
      //if(debug) log(F("info"), "Change of state! (" + String(buttonId)+")");
      lastActivityTime = millis();

      sprintf(topic, "cmd/%s/in/%d", arduinoId, buttonArray[buttonId]);
      sprintf(buffer, sensorReading == LOW ? "ON" : "OFF");
      sendMessage(topic, buffer);
      lastButtonState[buttonId] = sensorReading == LOW;
    }
  }
  lastButtonLevel[buttonId] = sensorReading;
}

/**
 * Initial configuration
 */
void setup(){
  /* ----------- I2C --------*/
  Wire.begin ();
  /* -------------- network ------------------- */
  #ifdef DEBUGSERIAL
  Serial.begin(9600);
  Serial.print(F("Using static MAC address: "));
  #endif
  char tmpBuf[17];
  sprintf(tmpBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #ifdef DEBUGSERIAL
  Serial.println(tmpBuf);
  #endif
  Ethernet.begin(mac, ip);  // Use static address defined above
  #ifdef DEBUGSERIAL
  Serial.print(F("My IP: http://"));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    if( thisByte < 3 ){
      Serial.print(".");
    }
  }
  Serial.println();
  Serial.println(Ethernet.localIP());
  #endif

  /* --------- prepare MQTT connection ------------- */
  #ifdef DEBUGSERIAL
  Serial.println("connecting...");
  #endif
  client.setServer(broker, 1883);
  client.setCallback(callback);
  #ifdef DEBUGSERIAL
  Serial.println("Ready.");
  #endif

  /* --------- IO ------------- */
  for(unsigned int i = 0; i < nbInput; i++){
    pinMode(buttonArray[i], INPUT_PULLUP);
  }
  for(unsigned int i = 0; i < nbPir; i++){
    pinMode(pirArray[i], INPUT);
  }
  for(unsigned int i = 0; i < nbOutput; i++){
    pinMode(outputArray[i], OUTPUT);
    digitalWrite(outputArray[i], HIGH);
  }
  /* --------- watchdog -------- */
  pinMode(watchdogResetPin, OUTPUT);
  digitalWrite(watchdogResetPin, LOW);
  delayMicroseconds(100); // reset the watchdog on the esp when booting up
  digitalWrite(watchdogResetPin, HIGH);
  watchdogStatus=HIGH;
}

void loop(){

  /* --------- Connect to MQTT broker ------------- */
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  /* ---------------- input pins -------------------*/
  for(unsigned int i = 0; i < nbInput; i++) {
      processButtonDigital(i);
  }

  /* ---------- mqtt message received --------------*/
  if(messageReceived){
      // cmd/etagexx/out/
      strcpy(buffer, receivedTopic);
      buffer[idLength+9]='\0'; //substring

      sprintf(buffer2, "cmd/%s/out/", arduinoId);

      if(strcmp(buffer, buffer2) == 0){
          // get the pin nbr, extrating from the topic
          strcpy(buffer, receivedTopic +sizeof(char)*(idLength+9));
          int pin = atoi(buffer);
          if(pin >= 0 /* 0 & 1 are for tx rx, can be used iif serial is disabled */  && pin <= 70 /* 50 -> 53 are for SPI */){
            if(debug){
              sprintf(buffer2,"out on pin %d, msg %s", pin, receivedPayload);
              logDebug(buffer2);
            }
            if(strcmp(receivedPayload, "ON") == 0){
              digitalWrite(pin, LOW);
              if(debug) {
                sprintf(buffer2, "successfully treated on message %s for arduino %s pin %d",receivedTopic, arduinoId, pin);
                logDebug(buffer2);
              }
            } else if(strcmp(receivedPayload,"OFF") == 0){
              digitalWrite(pin, HIGH);
              if(debug){
                 sprintf(buffer2, "successfully treated off message %s for arduino %s pin %d",receivedTopic, arduinoId, pin);
                 logDebug(buffer2);
              }
            } else if(strcmp(receivedPayload,"VON") == 0){
              Wire.beginTransmission (9);
              Wire.write(pin);
              Wire.write(250); // 250 for on, 200 for off
              Wire.endTransmission ();
              if(debug){
                 sprintf(buffer2, "successfully treated von message %s for arduino %s pin %d",receivedTopic, arduinoId, pin);
                 logDebug(buffer2);
              }
            } else if(strcmp(receivedPayload,"VOF") == 0){
              Wire.beginTransmission (9);
              Wire.write(pin);
              Wire.write(200); // 250 for on, 200 for off
              Wire.endTransmission ();
              if(debug){
                 sprintf(buffer2, "successfully treated vof message %s for arduino %s pin %d",receivedTopic, arduinoId, pin);
                 logDebug(buffer2);
              }
            } else {
              //means it is a percentage, to send in I2C
              val = atoi(receivedPayload);
              if(debug){
                 sprintf(buffer2, "Percentage received %d", val);
                 logDebug(buffer2);
              }
              Wire.beginTransmission (9);
              Wire.write(pin);
              Wire.write(val);
              Wire.endTransmission ();
              if(debug){
                 sprintf(buffer2, "successfully treated perentage message %s for arduino %s pin %d",receivedTopic, arduinoId, pin);
                 logDebug(buffer2);
              }
            }
            if(debug){
               sprintf(buffer2, "cmd: %s pin: %d", receivedTopic, pin);
               logDebug(buffer2);
            }
            sendCallback(receivedTopic);
          }
      } else {
          //cmd/etagexx/resetWatchdog
          sprintf(topic, "cmd/%s/resetWatchdog", arduinoId);
          if(strcmp(receivedTopic, topic) == 0){
            watchdogStatus=1-watchdogStatus;
            digitalWrite(watchdogResetPin, watchdogStatus); //toggle the pin
            if(debug){
               sprintf(buffer2, "Watchdog reset ok");
               logDebug(buffer2);
               sprintf(buffer2, "free rem %d", freeRam());
               logDebug(buffer2);
            }
          } else {
              // cmd/etagexx/debug
              sprintf(topic, "cmd/%s/debug", arduinoId);
              if(strcmp(receivedTopic, topic) == 0){
                sprintf(buffer, "true");
                if(strcmp(receivedPayload, buffer) == 0){
                  debug=true;
                  sprintf(buffer, "Log mode activated");
                  logDebug(buffer);
                } else {
                  debug=false;
                  sprintf(buffer, "Log mode desactivated");
                  logDebug(buffer);
                }
              } else {
                  logError(receivedTopic);
              }
          }
      }
      messageReceived = false;
  }
}
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
