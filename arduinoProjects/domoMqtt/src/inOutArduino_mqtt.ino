#include <SPI.h>                  // For networking
#include <Ethernet.h>             // For networking
#include <PubSubClient.h>         // For MQTT
#include <TimerOne.h>         // For MQTT

//#define DEBUG
//#define DEBUGMQTT
#define ETAGE1A

#ifdef ETAGE0A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
IPAddress ip(192,168,0,203);
String arduinoId = "etage0a";
#endif
#ifdef ETAGE1A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,200);
String arduinoId = "etage1a";
#endif
#ifdef ETAGE2A
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ip(192,168,0,201);
String arduinoId = "etage2a";
#endif
#ifdef ETAGE2B
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress ip(192,168,0,202);
String arduinoId = "etage2b";
#endif
#ifdef ETAGE0B
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xF0 };
IPAddress ip(192,168,0,204);
String arduinoId = "etage0b";
#endif

/*--------------------------- Network ------------------------------*/
/* Network Settings */
EthernetClient ethclient;

/*--------------------------- MQTT ------------------------------*/
IPAddress broker(192,168,0,186);
PubSubClient client(ethclient);
boolean messageReceived = false;
String receivedTopic;
String receivedPayload;
char buffer[50];
char topicBuffer[30];
char messBuffer[100];
/* ------------- button management -------------------------------- */
long lastActivityTime = 0;

const int nbOutput = 14;
const int nbInput = 33;
const int nbPir = 1;
static int lastButtonState[nbInput] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int lastButtonLevel[nbInput] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH,HIGH,HIGH, HIGH};
static int buttonArray[nbInput] = {14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,46,47};
static int lastPirState[nbPir] = {0};
static int pirArray[nbPir] = {44};
static int outputArray[nbOutput] = {1,2,3,4,5,6,7,8,9,11,12,13,48,49};
int lastStatus = 0;
#define DEBOUNCE_DELAY 50
//#define WATCHDOG 21600000
#define WATCHDOG 360000

/* ------------- MQTT in -------------------------------- */
void callback(char* topic, byte* payload, unsigned int l){
//      sendMessage("log/info/"+arduinoId, "callback");
  if(!messageReceived){
    messageReceived=true;
    receivedTopic = String(topic);
    receivedPayload = "";
    for (unsigned int i=0;i<l;i++) {
      receivedPayload = receivedPayload + String((char)payload[i]);
    }
  }
}

void sendMessage(String topic, String message){
  topic.toCharArray(topicBuffer, topic.length() + 1);
  message.toCharArray(messBuffer, message.length() + 1);
  client.publish(topicBuffer, messBuffer);
}

void sendCallback(String t){
    sendMessage("callback",  t);
}

/* ------------- MQTT connection -------------------------------- */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
    #endif
    // Attempt to connect
    String clientString = "Reconnecting Arduino-" + String(Ethernet.localIP());
    clientString.toCharArray(buffer, clientString.length()+1);
    
    if (client.connect(buffer, "slashback", "nimda")) {
      #ifdef DEBUG
      Serial.println("connected");
      #endif
      // Once connected, publish an announcement...
      // ... and resubscribe;
      String initString = "cmd/"+arduinoId+"/out/#";
      initString.toCharArray(buffer, initString.length()+1);
      client.subscribe(buffer);

      initString = "cmd/"+arduinoId+"/reset";
      initString.toCharArray(buffer, initString.length()+1);
      client.subscribe(buffer);
      
      sendMessage("log/info/"+arduinoId, "connected");
      sendMessage("init/"+arduinoId+"/ready", "ready");
    } else {
      #ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      #endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* -------------------- Input ------------------------*/
void processPir(int pirId){
  int sensorReading = digitalRead(pirArray[pirId]) == HIGH ? 1 : 0;
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
}
void processButtonDigital(int buttonId){
  int sensorReading = digitalRead(buttonArray[buttonId]);
  if (sensorReading != lastButtonLevel[buttonId]){
    #ifdef DEBUG
    Serial.println("sensor!=lastButton");
    #endif
    lastActivityTime = millis();
  }
  if((sensorReading == LOW && lastButtonState[buttonId] == 0) /*was high*/ ||
    (sensorReading == HIGH && lastButtonState[buttonId] == 1)) {  // potential change of state
    #ifdef DEBUG
    //Serial.println("change of state");
    #endif
    if((millis() - lastActivityTime) > DEBOUNCE_DELAY){  // Proceed if we haven't seen a recent event on this button
      #ifdef DEBUG
      //Serial.println("> debounce delay");
      #endif
      lastActivityTime = millis();

      if (sensorReading == LOW) {
        sendMessage("cmd/"+arduinoId+"/in/"+buttonArray[buttonId], "ON");
      } else {
        sendMessage("cmd/"+arduinoId+"/in/"+buttonArray[buttonId], "OFF");
      }
      lastButtonState[buttonId] = sensorReading == LOW ? 1 : 0;
    }
  }
  lastButtonLevel[buttonId] = sensorReading;
}
void(* resetFunc) (void) = 0;
//last time the WDT was ACKd by the application
unsigned long lastUpdate=0;
 
//time, in ms, after which a reset should be triggered
unsigned long timeout=WATCHDOG;
 
void longWDT(void)
{
  if((millis()-lastUpdate) > timeout)
  {
    //enable interrupts so serial can work
  //#ifdef DEBUG
    sei();
  //#endif
 
    //detach Timer1 interrupt so that if processing goes long, WDT isn't re-triggered
    Timer1.detachInterrupt();
 
    //flush, as Serial is buffered; and on hitting reset that buffer is cleared
  #ifdef DEBUG
    Serial.println("WDT triggered");
    Serial.flush();
  #endif
 
    //call to bootloader / code at address 0
    resetFunc();
  }
}
/**
 * Initial configuration
 */
void setup(){
  /* -------------- network ------------------- */
  #ifdef DEBUG
  Serial.begin(9600); 
  Serial.print(F("Using static MAC address: "));
  #endif
  char tmpBuf[17];
  sprintf(tmpBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #ifdef DEBUG
  Serial.println(tmpBuf);
  #endif
  Ethernet.begin(mac, ip);  // Use static address defined above
  #ifdef DEBUG
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
  #ifdef DEBUG
  Serial.println("connecting...");
  #endif
  client.setServer(broker, 1883);
  client.setCallback(callback);
  #ifdef DEBUG
  Serial.println("Ready.");
  #endif
  
  /* --------- IO ------------- */
  for(int i = 0; i < nbInput; i++){
    pinMode(buttonArray[i], INPUT_PULLUP);
  }
  for(int i = 0; i < nbPir; i++){
    pinMode(pirArray[i], INPUT);
  }
  for(int i = 0; i < nbOutput; i++){
    pinMode(outputArray[i], OUTPUT);
    digitalWrite(outputArray[i], HIGH);
  }
  /* ----------- watchdog ------------ */
  //allow 30s without update; and seed an update
  timeout = 30000;
  lastUpdate=millis();
 
//  timeout = 21600000;
 /* if (!client.connected()) {
    reconnect();
  }
  client.loop();  
 */ 
  Timer1.initialize(1000000); //1 second pulses
  Timer1.attachInterrupt(longWDT); //code to execute
  //sendMessage("log/debug/"+arduinoId, "Attached");
 
  //long running setup code of 6 minutes (6 * 60 * 60 * 1000)
  timeout = WATCHDOG;
}

void loop(){
  
  /* --------- Connect to MQTT broker ------------- */
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  
  
  /* ---------------- input pins -------------------*/
  for(int i = 0; i < nbInput; i++) {
      processButtonDigital(i);
  }

  /* ---------------- pir pins -------------------*/
  /*for(int i = 0; i < nbPir; i++) {
      processPir(i);
  }*/
  
  /* ---------- mqtt message received --------------*/
  if(messageReceived){
    // cmd/arduinoId/[in|out]/pinId
      if(receivedTopic.substring(0, ("cmd/"+arduinoId+"/out/").length()) == "cmd/"+arduinoId+"/out/"){
          int pin = receivedTopic.substring(("cmd/"+arduinoId+"/out/").length()).toInt(); //remove prefix
          if(pin >= 1 /* 0 & 1 are for tx rx */  && pin <= 49 /* 50 -> 53 are for SPI */){
            if(receivedPayload == "ON"){
              digitalWrite(pin, LOW);
              #ifdef DEBUG
              sendMessage("log/debug/"+arduinoId, "successfully treated message "+receivedTopic+" for arduino "+arduinoId+" pin "+String(pin));
              #endif
            } else if(receivedPayload == "OFF"){
              digitalWrite(pin, HIGH);
              #ifdef DEBUG
              sendMessage("log/debug/"+arduinoId, "successfully treated message "+receivedTopic+" for arduino "+arduinoId+" pin "+String(pin));
              #endif
            } else {
              sendMessage("log/error/"+arduinoId, "failed to treat message "+receivedTopic+" for arduino "+arduinoId+" pin "+String(pin)+" message "+receivedPayload);
            }
            #ifdef DEBUG
            sendMessage("log/cmd",receivedTopic);
            sendMessage("log/pin",String(pin));
            #endif
            sendCallback(receivedTopic);
          }
     // } else if(receivedTopic.substring(0, ("cmd/"+arduinoId+"/wdg").length()) == "cmd/"+arduinoId+"/wdg"){
     //   sendMessage("log/debug/"+arduinoId, "Reset of the watchdog");
	//lastUpdate=millis(); 
	//Timer1.restart();
      } else if(receivedTopic.substring(0, ("cmd/"+arduinoId+"/reset").length()) == "cmd/"+arduinoId+"/reset"){
        sendMessage("log/debug/"+arduinoId, "Reset of the watchdog");
	//Timer1.restart();
	lastUpdate=millis(); 
	//resetFunc();
      } else {
        sendMessage("log/error/"+arduinoId, receivedTopic);
      }
      messageReceived = false;
  }
}
