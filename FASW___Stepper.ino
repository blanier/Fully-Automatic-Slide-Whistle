#include "TimerOne.h"

#define UP 0
#define DOWN 1

#define STEP_PIN 12
#define DIR_PIN 13

int limits[2];
int limit_pins[2] = {2,3};

int current_position = 0;
int target_position = 0;
int current_direction = 0;
int observed_limit_pin = 0;

int current_dir_pin = HIGH;
int current_step_pin = HIGH;

int trigger_position = 0;
int trigger_direction = 0;
int triggered = false;

void setup()
{
  Serial.begin(115200);
  
  Timer1.initialize(80);        // initialize timer1, and set a 1/4 second period
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt

  for (int i=0; i<2; i++) {
    pinMode(limit_pins[i], INPUT);
  }
  
  pinMode(DIR_PIN, OUTPUT); 
  pinMode(STEP_PIN, OUTPUT);
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

void loop()
{
  delay(250);
  
  statusUpdate();
  
  if (current_position == target_position) {
    do {
      target_position = random(10001);
    } while (abs(current_position-target_position)<10);
  }
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
  
  if (!limits[current_direction]) {     
    current_step_pin = (current_step_pin==HIGH)?LOW:HIGH;
    
    digitalWrite(DIR_PIN, current_direction);
    digitalWrite(STEP_PIN, current_step_pin);
    
    current_position += offset;
  }
}
 
