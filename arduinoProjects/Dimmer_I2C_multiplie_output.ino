/**
  Fonctionne plutôt bien, sauf que parfois en allumant au min, elle s'allume d'abord 100% un bref instant
  Pareil que Dimmer_I2C_multiple_output_workingWithOneFlash mais sans le code superflux
*/
#include <Wire.h> 
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero

#define ZERO_CROSS 2
#define etage2A

#ifdef etage0A
#define oneOut
#endif

#ifdef etage1A
#define twoOut
#endif

#ifdef etage2A
#define twoOut
#endif

#ifdef etage2B
#define oneOut
#endif

#ifdef oneOut
#define PINS 1
static int pins[PINS] = {3}; // pins  used for output
#endif

#ifdef twoOut
#define PINS 2
static int pins[PINS] = {3, 4}; // pins  used for output
#endif

volatile int sortedNb[PINS];
volatile int dimmerState[PINS];
volatile int dimmerValues[PINS];

// debug mode fout la merde !
//#define debug

void setup() {
  for (int i = 0; i < PINS; i++){
    pinMode(pins[i], OUTPUT);// Set AC Load pin as output
    dimmerState[i] = 0;
    dimmerValues[i] = 100;
  }
  orderOutputs();
  pinMode(ZERO_CROSS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS), zero_cross_int, RISING);
#ifdef debug    
  Serial.begin(9600);
  Serial.println("Starting");
#endif
  Wire.begin(9); // start slave with address 9
  Wire.onReceive(receiveEvent);
}
void orderOutputs(){
  int tmpTab[PINS];
  for (int i = 0; i < PINS; i++){
    tmpTab[i] = dimmerValues[i];
  }
  for (int k = 0; k < PINS; k++){
    int minVal = 101;
    int minIndex = -1;
    for (int i = 0; i < PINS; i++){
      if(minVal > tmpTab[i]){
        minVal = tmpTab[i];
        minIndex = i;
      }
    }
    tmpTab[minIndex] = 101;
    sortedNb[k] = minIndex;
  }
#ifdef debug
  Serial.println("Order outputs results");
  for (int k = 0; k < PINS; k++){
    Serial.print("k: ");
    Serial.print(k);
    Serial.print(" val: ");
    Serial.println(sortedNb[k] );
  }
#endif
}
void receiveEvent(int howMany) {
#ifdef debug
  Serial.print("received ");
  Serial.print(howMany);
  Serial.println(" bytes");
  
  Serial.print(Wire.available());
  Serial.println(" bytes available");
#endif
  
  int p = 0;
  if (2 <= Wire.available()) { // si deux octets disponibles TO TEST
    int p = Wire.read(); //first byte is output index. Should be >= 0 & < sizeOf(pins)
    int val = Wire.read(); //second byte is state/luminosity 
#ifdef debug
    Serial.print("index ");
    Serial.println(p);
    Serial.print("value ");
    Serial.println(val);
#endif
    if(val == 200){ //OFF
      dimmerState[p] = 0;
    } else if(val == 250){//ON
      dimmerState[p] = 1;
    } else { // in theory < 100
      dimmerValues[p] = 100-val; // ->brake stuf
      orderOutputs();
    }
  }
  while(Wire.available() >= 1){
    Wire.read();
  }
}
void zero_cross_int() { // function to be fired at the zero crossing to dim the light
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us
  
  int previousWait = 0;
  int index = 0;
  for(int i = 0; i < PINS; i++){
    index = sortedNb[i];
    if (dimmerValues[index] > 95 || dimmerState[index] == 0) {
      digitalWrite(pins[index], LOW);    // triac Off
    } else if (dimmerValues[index] < 5 ){
      digitalWrite(pins[index], HIGH);   // triac On
    } else {
      int dimtime = 100*dimmerValues[index]-previousWait;    // For 60Hz =>65   
      previousWait = 100*dimmerValues[index];
      // Replace with a timer to avoid wasting time
      delayMicroseconds(dimtime);    // Off cycle
      digitalWrite(pins[index], HIGH);   // triac firing
      delayMicroseconds(10);         // triac On propogation delay (for 60Hz use 8.33)
    // to try: remove the delay 10 and add the turn off at the very begining or the zero_cross_int OR at the end of the for  
    /*
     * The Triac will switch off again at the following zero crossing, but we are going to already 
     * write a low on the TRIAC pin to avoid accidental ignition in the next cycle. 
     * We need to wait a bit however to know for sure the TRIAC is on, so we wait 10us
     */
      digitalWrite(pins[index], LOW);    // triac Off
    }
  }  
}   

void loop() {}
