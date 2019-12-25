#include  <TimerOne.h>          // Avaiable from http://www.arduino.cc/playground/Code/Timer1
#include <Wire.h> 
volatile int dimmerIndex=0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int freqStep = 75;    // This is the delay-per-brightness step in microseconds.
unsigned char AC_LOAD = 13;    // Output to Opto Triac pin
unsigned char ZERO_CROSS = 2; 
unsigned char i;
const int nbOutputs=1;
static int pins[nbOutputs] = {0}; // pins  used for output
static int dimmerValues[nbOutputs] = {50}; // dimmer of each output (0 -> 100)
static int found[nbOutputs] = {0}; // dimmer of each output (0 -> 100)
int done;
int x; 
// 1 pin for cross zero
// n pin in for switch on/off
// n pin out for dimmer

void setup() {
  // put your setup code here, to run once:
  done = nbOutputs;
  for (i = 0; i < nbOutputs; i++){
    found[i] = 0;
    pinMode(pins[i], OUTPUT);// Set AC Load pin as output
  }
  pinMode(ZERO_CROSS, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ZERO_CROSS), zero_cross_detect, RISING);
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);      
  Serial.begin(9600);
  Wire.begin(9); // start slave with address 9
  Wire.onReceive(receiveEvent);
  x = '-';
}
void receiveEvent(int howMany) {
  // expect message saying 4-100 or 3-025 for pin 3 25%
  int p = Wire.read(); // pin
  Wire.read(); //'-'
  int val=0;
  while (0 < Wire.available()) { // loop through all but the last
    val = val*10 + Wire.read();
  }
  Serial.println(p);
  Serial.println(val);
  dimmerValues[p] = val;
}

void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  dimmerIndex=0;
  for (i = 0; i < nbOutputs; i++){
    if(dimmerValues[i] > 5){
      digitalWrite(pins[i], LOW);       // turn off 
    }
  }
}                                 
void dim_check() {                   
  if(zero_cross == true) {              
    for (i = 0; i < nbOutputs; i++){
      if (dimmerIndex >= dimmerValues[i] && found[i] == 0) {
        done--;
        found[i] = 1;
        if (dimmerValues[i] <= 95){
          digitalWrite(pins[i], HIGH); // turn on light       
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

void loop() {
}

