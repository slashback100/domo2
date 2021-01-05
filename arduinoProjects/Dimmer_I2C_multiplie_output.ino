//#include  <TimerOne.h>          // Avaiable from http://www.arduino.cc/playground/Code/Timer1
#include <Wire.h> 
volatile int dimmerIndex=0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
//int freqStep = 75;    // This is the delay-per-brightness step in microseconds.
//unsigned char AC_LOAD = 3;    // Output to Opto Triac pin
unsigned char ZERO_CROSS = 2; 
unsigned char i;
#define etage1A

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
const int nbOutputs=1;
static int pins[nbOutputs] = {3}; // pins  used for output
static int dimmerValues[nbOutputs] = {100}; // dimmer of each output (0 -> 100) (0 = ON, 100 = OFF)
static int dimmerState[nbOutputs] = {0}; // 200 = OFF, 250 = ON
static int found[nbOutputs] = {0};
static int sortedNb[nbOutputs] = {0};
#endif

#ifdef twoOut
const int nbOutputs=2;
static int pins[nbOutputs] = {3, 4}; // pins  used for output
static int dimmerValues[nbOutputs] = {100, 100}; // dimmer of each output (0 -> 100) (0 = ON, 100 = OFF)
static int dimmerState[nbOutputs] = {0, 0}; // 200 = OFF, 250 = ON
static int found[nbOutputs] = {0, 0};
static int sortedNb[nbOutputs] = {0, 1};
#endif


int done;
//#define debug
// int x; 
// 1 pin for cross zero
// n pin in for switch on/off
// n pin out for dimmer

unsigned long starttime;
void setup() {
  done = nbOutputs;
  for (i = 0; i < nbOutputs; i++){
    found[i] = 0;
    pinMode(pins[i], OUTPUT);// Set AC Load pin as output
  }
  pinMode(ZERO_CROSS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS), zero_cross_int, RISING);
  //Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  //Timer1.attachInterrupt(dim_check, freqStep);  
#ifdef debug    
  Serial.begin(9600);
  Serial.println("coucou");
#endif
  Wire.begin(9); // start slave with address 9
  Wire.onReceive(receiveEvent);
  starttime = millis();
}
void orderOutputs(){
  int tmpTab[nbOutputs];
  //copy in tmp
  for (i = 0; i < nbOutputs; i++){
    tmpTab[i] = dimmerValues[i];
  }
  for (int k = 0; k < nbOutputs; k++){
    int minVal = 101;
    int minIndex = -1;
    for (i = 0; i < nbOutputs; i++){
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
  for (int k = 0; k < nbOutputs; k++){
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
/*
void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  dimmerIndex=0;
  for (i = 0; i < nbOutputs; i++){
    if(dimmerValues[i] > 5){
      digitalWrite(pins[i], LOW);       // turn off 
    }
  }
} */ 
void zero_cross_int() { // function to be fired at the zero crossing to dim the light
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us
  
  int previousWait = 0;
  int index = 0;
  for(int i = 0; i < nbOutputs; i++){
    index = sortedNb[i];
#ifdef debug
//    Serial.print("index is ");
//    Serial.println(index);
#endif
    
    if (dimmerValues[index] > 95 || dimmerState[index] == 0) {
      digitalWrite(pins[index], LOW);    // triac Off
    } else if (dimmerValues[index] < 5 ){
      digitalWrite(pins[index], HIGH);   // triac On
    } else {
      int dimtime = 100*dimmerValues[index]-previousWait;    // For 60Hz =>65   
#ifdef debug
/*    Serial.print("dimmerValues ") ;
    Serial.print(dimmerValues[index]) ;
    Serial.print(" dimtime ") ;
    Serial.print(dimtime) ;
    Serial.print(" previouswait ") ;
    Serial.println(previousWait) ;
*/
#endif
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
  /*
  delayMicroseconds(10); 
  for(i = 0; i < nbOutputs; i++){
    if (dimmerValues[i] >= 5 ){
      digitalWrite(pins[i], LOW);    // triac Off
    }
  }*/
  
}   
/*                            
void dim_check() {                   
  if(zero_cross == true) {              
    for (i = 0; i < nbOutputs; i++){
      if(dimmerState[0] == 0){
        done--;
        found[i] = 1;
      } else {
        if (dimmerIndex >= dimmerValues[i] && found[i] == 0) {
          done--;
          found[i] = 1;
          if (dimmerValues[i] <= 95){
            digitalWrite(pins[i], HIGH); // turn on light       
          }
        }
      }
    } 
    if (done == 0){  
      dimmerIndex = 0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
      done = nbOutputs;
      for (i = 0; i < nbOutputs; i++){
        found[i] = 0;
      }
    }
    dimmerIndex++; // increment time step counter                     
  }                                  
}  
*/
void loop() {
  if(starttime + 1000 * 5 <= millis()){ //30 s
    Serial.println("I'm alive");
    starttime = millis();
  }
}
