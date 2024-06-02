#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
// #include <ezButton.h>
#include "Button2.h"

#define GPIO_0_PIN 0
#define GPIO_2_PIN 2
#define GPIO_3_PIN 3
#define GPIO_4_PIN 1
Button2 buttonA, buttonB, buttonC;
byte x = 1;

const int interval = 150;
int ledState = HIGH;
int blinkCounter = 0;
unsigned long previousMillis2 = 0;
unsigned long lastButtonPressMillis2 = 0;
const unsigned long inactivityTimeout = 5 * 60 * 1000; // 5 minutes in milliseconds


uint8_t receiverMac[] = {0x84, 0xCC, 0xA8, 0xA4, 0x0B, 0xE0};

// Define an enumeration for commands
enum Command {
  FORWARD = 1,
  BACKWARD,
  PROGRAMMING,
  DIRECTION,
  SAVE,
  HOLD_FOR,
  HOLD_BACK,
  RESTART
};


void resetInactivityTimer() {
  lastButtonPressMillis2 = millis();
}

void sendCommand(Command command) {
  esp_now_send(receiverMac, (uint8_t *)&command, sizeof(command));
}

void handlerA(Button2& btn) {
  resetInactivityTimer();
    switch (btn.getType()) {
        case single_click:{
          x = 1 ;
        sendCommand(FORWARD); }                   
            break;
        case triple_click:{
          x = 3 ;
            sendCommand(HOLD_FOR);}            
            break;
    }
}
void handlerB(Button2& btn) {
  resetInactivityTimer();
    switch (btn.getType()) {
        case single_click:{
          x = 1 ;
          sendCommand(BACKWARD);}          
            break;
        case triple_click: {
          x = 3 ;           
            sendCommand(HOLD_BACK);}            
            break;
    }
}
void handlerC(Button2& btn) {
  resetInactivityTimer();
    switch (btn.getType()) {
        case single_click:{
          x = 1;
          sendCommand(PROGRAMMING);}        
            break;
        case double_click:{
          x = 2;
            sendCommand(DIRECTION);}            
            break;
        case triple_click:{
          x = 3;
            sendCommand(SAVE);}            
            break;
    }
}


void setup ()
{
  

  pinMode(GPIO_4_PIN , OUTPUT);
  digitalWrite(GPIO_4_PIN , HIGH);

    WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(receiverMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  buttonA.begin(GPIO_0_PIN);
  buttonA.setClickHandler(handlerA);  // this will only be called upon detection
  buttonA.setTripleClickHandler(handlerA);

  buttonB.begin(GPIO_2_PIN);
  buttonB.setClickHandler(handlerB);
  buttonB.setTripleClickHandler(handlerB);
  
  buttonC.begin(GPIO_3_PIN);
  buttonC.setClickHandler(handlerC);
  buttonC.setDoubleClickHandler(handlerC);
  buttonC.setTripleClickHandler(handlerC);
  sendCommand(RESTART);
  resetInactivityTimer();
  
}



void loop () {

  buttonA.loop();
  buttonB.loop();
  buttonC.loop();

unsigned long currentMillis2 = millis(); 
if (currentMillis2 - previousMillis2 >= interval) {
    previousMillis2 = currentMillis2;  // Save the current time

    // If the LED is off, turn it on and vice versa
    if (ledState == HIGH) {
      ledState = LOW; // Turn the LED on (active low)
      blinkCounter++; // Increment the blink counter when the LED turns on
    } else {
      ledState = HIGH; // Turn the LED off (active low)
    }
    digitalWrite(GPIO_4_PIN, ledState); 
      if (blinkCounter > x) {
      // Turn off the LED and stop further execution
      digitalWrite(GPIO_4_PIN, HIGH);
      blinkCounter = 0;
      x = 0; // Ensure the LED is off
     
    }
  }
 
  if (millis() - lastButtonPressMillis2 >= inactivityTimeout) {
    // Go to deep sleep
    ESP.deepSleep(0);
  }

}

