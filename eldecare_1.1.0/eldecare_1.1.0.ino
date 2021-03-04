#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM900
SoftwareSerial mySerial(7, 8); //SIM900 Tx & Rx is connected to Arduino #7 & #8

char remoteNumber[20]= "";  // enter the phone number of recipient

const int DOOR_SENSOR_PIN_1 = 13; // Arduino pin connected to door sensor's pin
const int DOOR_SENSOR_PIN_2 = 4;

const int SUMMARY_TIME = 86400000;  // summary interval (customizable) DEFAULT 24H
const int WARNING_TIME = 18000000;  // warning interval (customizable) DEFAULT 5H

// Variables for recording states of sensors:

int currentDoorState_1; // current state of door sensor
int lastDoorState_1;    // previous state of door sensor

int currentDoorState_2;
int lastDoorState_2;

// Variables for recording the number of sensor triggers:

int action_counter_1;   // counters, recording the number of times when some behaviour
int action_counter_2;   // was recorded


void setup() {
  Serial.begin(19200);
  mySerial.begin(19200);

  pinMode(DOOR_SENSOR_PIN_1, INPUT_PULLUP); // set arduino pin to input pull-up mode
  currentDoorState_1 = digitalRead(DOOR_SENSOR_PIN_1); // read state
  
  pinMode(DOOR_SENSOR_PIN_2, INPUT_PULLUP);
  currentDoorState_2 = digitalRead(DOOR_SENSOR_PIN_2);

  action_counter_1 = 0;
  action_counter_2 = 0;
}


void sendSummary(){
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println(String(String("AT+CMGS=\"") + String(remoteNumber) + String("\"")));
  updateSerial();
  mySerial.print(String(String("Summary of the activity:") + String("\nFridge - ") + String(action_counter_1) + String("\nBedroom - ") + String(action_counter_2))); //text content
  updateSerial();
  mySerial.write(26);
}

void sendWarning(String sensor){
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println(String(String("AT+CMGS=\"") + String(remoteNumber) + String("\"")));
  updateSerial();
  mySerial.print(String(String(sensor) + String(" - there was no activity detected for 5 hours"))); //text content
  updateSerial();
  mySerial.write(26);
}

void updateSerial(){

  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
  }
  
}


void loop() {
  int current_time_sum = millis();   // get the current time (for summary)
  int current_time_warn = millis();  // get the current time (for warning)
  
  lastDoorState_1 = currentDoorState_1;              // save the last state
  currentDoorState_1  = digitalRead(DOOR_SENSOR_PIN_1); // read new state
  
  if (lastDoorState_1 == LOW && currentDoorState_1 == HIGH && millis() - state_change_1 >= 3000) { // state change
      action_counter_1 ++;
      delay(3000);          // delay to avoid magnetic fluctuations
  }
    
  else if (lastDoorState_1 == HIGH && currentDoorState_1 == LOW && millis() - state_change_1 >= 3000){
      action_counter_1 ++;
      delay(3000);
  }

  lastDoorState_2 = currentDoorState_2;              // save the last state
  currentDoorState_2  = digitalRead(DOOR_SENSOR_PIN_2); // read new state

  if (lastDoorState_2 == LOW && currentDoorState_2 == HIGH && millis() - state_change_2 >= 3000) { // state change
      action_counter_2 ++;
      delay(3000);
  }
    
  else if (lastDoorState_2 == HIGH && currentDoorState_2 == LOW && millis() - state_change_2 >= 3000){
      action_counter_2 ++;
      delay(3000);
  }

  // Checking if the warning interval has passed and doors have not been opened

  if (current_time_warn == WARNING_TIME && action_counter_1 == 0){
    sendWarning("Fridge");
    current_time_warn = 0;
  }

  else if (current_time_warn == WARNING_TIME && action_counter_2 == 0){
    sendWarning("Bedroom");
    current_time_warn = 0;
  }

  // Checking if the specified summary interval has passed and passing information to the user:

  if (current_time_sum == SUMMARY_TIME){
    sendSummary();
    current_time_sum = 0;
    action_counter_1 = 0;
    action_counter_2 = 0;
  }

}
