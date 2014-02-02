#include "TimerOne.h"

#define STEP_PIN 12            // the "step" pin of the Easy Driver
#define DIR_PIN 13             // the "dir" pin of the Easy Driver
#define SOL_PIN 4              // the solenoid pin

int limits[2];                 // these are the states of the limit pins, HIGH if triggered.  They are
                               // written to by the state machine
                             
int limit_pins[2] = {3,2};     // the limit pins themselves

long current_position = 0;     // used by the state machine to keep track of where the stepper is

long target_position = 0;      // used by the state machine to keep track of where we want to go.
                               // if current_position == target position, there's nothing to do
                             
int direction_proxy = 0;       // used by the state machine to track which direction we are travelling
                               // that way we can honor travel requests in the direction where the limit switch
                               // has not been tripped, and disalow them the other direction.

int step_proxy = HIGH;         // Used by the state machine to enable the toggling behavior of the step pin

int solenoid_proxy = LOW;      // Used by the state machine to set the state of the solenoid.

void setup() {
  Serial.begin(115200);              // initialize serial communication at a very high rate
  
  Timer1.initialize(160);            // initialize timer1, and set a 160 uSec second period
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt

  for (int i=0; i<2; i++) {         
    pinMode(limit_pins[i], INPUT);   // set the limit pins to INPUT mode since we will be reading them in the state machine
  }
  
  pinMode(DIR_PIN, OUTPUT);          // set up the pins we will be writing to 
  pinMode(STEP_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);
  
  recalibrate();                     // establish where 0 is on the stepper
}

void loop() {
  delay(250);
  
  statusUpdate();
  readInput();
  // toggleSolenoid();
}

void userInterlock() {
  Serial.println("Please send me an X");
  while (Serial.read() != 'X') {
  }
}
  
void statusUpdate() {
  Serial.print(limits[0]);
  Serial.print(" ");
  Serial.print(limits[1]);
  Serial.print(" ");
  
  Serial.print(current_position);
  Serial.print(" ");
  Serial.print(target_position);
  Serial.print(" ");
  Serial.print(direction_proxy);
  Serial.print(" ");
  Serial.print(step_proxy);
  Serial.print(" ");
  Serial.print(solenoid_proxy);
  
  Serial.print("\n");
}

// we have roughly 600000 steps.  Make 0 near 1 edge and 600000 near the other
void recalibrate() {
  
  target_position = 100000;
  delay(1000);
  
  long range = -600000;
  
  Serial.print("Setting target to ");
  Serial.println(range);
  target_position = range;
  while (limits[0]== LOW) {
    statusUpdate();
  }
  Serial.println("MIN found");
  current_position = target_position = 0;
}

long delay_until = 0; 

void readInput() {
  if (Serial.available() && (millis() > delay_until)) {
    
    char cmd = Serial.read();
    long value = Serial.parseInt();

    Serial.print(cmd);
    Serial.print(" ");
    Serial.println(value);
    
    switch (cmd) {
      case 'g':  // goto position
        target_position = value;
        break;
      
      case 's': // solenoid on/off
        solenoid_proxy = (value)?HIGH:LOW;
        break;
      
      case 'd': // delay
        delay_until = millis() + value;
        break;
      
      default:
        Serial.println("What you talkin' 'bout, Willis");
        break;
    }
    
    Serial.read();  // add an extra read.  Sender can add a space, which allows parseInt to end fast (rather than timing out)
  }
}


void toggleSolenoid() {
  solenoid_proxy = (solenoid_proxy==HIGH)?LOW:HIGH;
  digitalWrite(SOL_PIN, solenoid_proxy);
}

void callback() {
  // read the limit switches
  for (int i=0;i<2;i++) {
    limits[i] = digitalRead(limit_pins[i]);
  }
  
  // figure out what we want to do to the stepper
  int offset = 0;
  
  if (current_position < target_position) {
    offset = 1;
    direction_proxy = HIGH;
  }
  if (current_position > target_position) {
    offset = -1;
    direction_proxy = LOW;
  }
  
  // if permitted and needed, move the stepper and keep track of the position
  if (offset && !limits[direction_proxy]) {     
    step_proxy = (step_proxy==HIGH)?LOW:HIGH;
    
    digitalWrite(DIR_PIN, direction_proxy);
    digitalWrite(STEP_PIN, step_proxy);
    
    current_position += offset;
  }
  
  // write the solenoid
  digitalWrite(SOL_PIN, solenoid_proxy);
}
 
