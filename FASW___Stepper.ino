#include "TimerOne.h"

#define STEP_PIN 12
#define DIR_PIN 13
#define SOL_PIN 4

int limits[2];
int limit_pins[2] = {3,2};

long current_position = 0;
long target_position = 0;
int current_direction = 0;
int observed_limit_pin = 0;

int current_dir_pin = HIGH;
int current_step_pin = HIGH;

void setup() {
  Serial.begin(115200);
  
  Timer1.initialize(160);        // initialize timer1, and set a 80 uSec second period
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt

  for (int i=0; i<2; i++) {
    pinMode(limit_pins[i], INPUT);
  }
  
  pinMode(DIR_PIN, OUTPUT); 
  pinMode(STEP_PIN, OUTPUT);
  pinMode(SOL_PIN, OUTPUT);
  
  //userInterlock();
  recalibrate();
}

void loop() {
  delay(250);
  
  statusUpdate();
  readInput();
  toggleSolenoid();
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
  Serial.print(current_direction);
  Serial.print(" ");
  Serial.print(current_step_pin);
  Serial.print(" ");
  
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
  current_position = 0;
}

void readInput() {
  if (Serial.available()) {
    
    target_position = Serial.parseInt();
    Serial.read();  // add an extra read.  Sender qill add a space.  This allows parseInt to end fast (rather than timing out)
  }
}

int solProxy = HIGH;
void toggleSolenoid() {
  solProxy = (solProxy==HIGH)?LOW:HIGH;
  digitalWrite(SOL_PIN, solProxy);
}

void callback() {
  for (int i=0;i<2;i++) {
    limits[i] = digitalRead(limit_pins[i]);
  }
  
  int offset = 0;
  
  if (current_position < target_position) {
    offset = 1;
    current_direction = HIGH;
  }
  if (current_position > target_position) {
    offset = -1;
    current_direction = LOW;
  }
  
  if (offset && !limits[current_direction]) {     
    current_step_pin = (current_step_pin==HIGH)?LOW:HIGH;
    
    digitalWrite(DIR_PIN, current_direction);
    digitalWrite(STEP_PIN, current_step_pin);
    
    current_position += offset;
  }
}
 
