// #include <ESP8266WiFi.h>
// #include <espnow.h>
// #include <EEPROM.h>
// #include <Arduino.h>

// #define MOTOR_FORWARD_PIN 14
// #define MOTOR_BACKWARD_PIN 16
// #define POWER_OFF_PIN 5
// #define BUTTON_PIN 4  // Change to the pin connected to your button

// bool forward = false;
// bool inProgrammingMode = false;
// bool inSave = false;
// bool holdfor = false;



// // Define an enumeration for commands
// enum Command {
//   FORWARD = 1,
//   BACKWARD,
//   PROGRAMMING,
//   DIRECTION,
//   SAVE,
//   HOLD_FOR,
//   HOLD_BACK,
//   RESTART
// };

// //EEprom adresses
// const int addrForwardTime = 0;
// const int addrBackwardTime = 4;
// const int addrDirection = 8;
// const int addrMotorRunning = 9;
// unsigned long forwardTime = 0;
// unsigned long accumlate = 0;

// // void saveMotorState() {
// //   EEPROM.put(addrForwardTime, accumlatedForwardTime);
// //   EEPROM.put(addrBackwardTime, accumlatedBackwardTime);
// //   EEPROM.put(addrDirection, directionForward);
// //   EEPROM.put(addrMotorRunning, motorRunning);
// //   EEPROM.commit();
// // }

// // void stopMotor() {
// //   digitalWrite(MOTOR_FORWARD_PIN, LOW);
// //   digitalWrite(MOTOR_BACKWARD_PIN, LOW);
// //   motorRunning = false;
// //   saveMotorState();
// // }

// void runMotor(bool forward) {
//   if (forward) {   
//     digitalWrite(MOTOR_FORWARD_PIN, HIGH);
//     digitalWrite(MOTOR_BACKWARD_PIN, LOW);
//   } else {
//     digitalWrite(MOTOR_FORWARD_PIN, LOW);
//     digitalWrite(MOTOR_BACKWARD_PIN, HIGH);
//   }
// }


// void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
//   Command receivedCommand;
//   memcpy(&receivedCommand, data, sizeof(receivedCommand));
//   Serial.print("Command received: ");
//   Serial.println(receivedCommand);

//   switch (receivedCommand) {
//     case FORWARD:
//         forward = true;
//       break;
//     case BACKWARD:
//       Serial.println("Backward");
//       break;
//     case PROGRAMMING:
//       inProgrammingMode = true;
//       break;
//     case DIRECTION:
//       Serial.println("DIRECTION");
//       break;
//     case SAVE:
//       inSave = true;
//       break;
//     case HOLD_FOR:
//       holdfor = true;
//       break;
//     case HOLD_BACK:
//       Serial.println("Hold Back");
//       break;
//       case RESTART:
//       Serial.println("RESTART");
//       break;
//     default:
//       break;
//   }
// }



// void setup ()
// {
//      Serial.begin(115200);
//       WiFi.mode(WIFI_STA);
//     if (esp_now_init() != 0) {
//     Serial.println("ESP-NOW initialization failed");
//     return;
//     }
//     esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
//     esp_now_register_recv_cb(onDataRecv);

//     pinMode(MOTOR_FORWARD_PIN, OUTPUT);
//     pinMode(MOTOR_BACKWARD_PIN, OUTPUT);
//     pinMode(POWER_OFF_PIN, INPUT_PULLUP);
//     digitalWrite(MOTOR_FORWARD_PIN, LOW);
//     digitalWrite(MOTOR_BACKWARD_PIN, LOW);

//     // EEPROM.begin(512);
//     // EEPROM.get(addrForwardTime, accumlatedForwardTime);
//     // EEPROM.get(addrBackwardTime, accumlatedBackwardTime);
//     // EEPROM.get(addrDirection, directionForward);
//     // EEPROM.get(addrMotorRunning, motorRunning);
// }

// void loop ()
// {
//   if (inProgrammingMode == true)
//   {
//     if (holdfor)
//       {
//         digitalWrite(MOTOR_BACKWARD_PIN , HIGH);
//         Serial.println("RElay HIGH");
//           forwardTime  = millis();
//       }
//       if (forward)
//         {
//           forward = false;
//           holdfor = false;
//           inProgrammingMode = false;
//           digitalWrite(MOTOR_FORWARD_PIN , LOW);
//            Serial.println("RElay OFF");
//            accumlate = millis() - forwardTime;
//            Serial.println(accumlate);
//            //eeprom save
//         }
//   }
// }





/*****************************************/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>

// Define GPIO pins for the receiver
#define RELAY_FORWARD_PIN 14  // Forward relay pin
#define RELAY_BACKWARD_PIN 16 // Backward relay pin
#define POWER_PIN 4          // Power detection pin

// Define an enumeration for commands (same as on the sender side)
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

// Variables to store relay activation duration
unsigned long holdForwardDuration = 0;
unsigned long holdBackwardDuration = 0;

// Flags to indicate current states
bool programmingMode = false;
bool forwardActive = false;
bool backwardActive = false;
bool holdForwardActive = false;
bool holdBackwardActive = false;
unsigned long startTime = 0;
bool powerRestored = false;
unsigned long relayResumeTime = 0;


bool stopForceForward = false;
bool stopForceBackward = false;
bool forwardComplete = false ;
bool backwardComplete = false ;

// Function to handle received data

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  Command receivedCommand;
  memcpy(&receivedCommand, incomingData, sizeof(receivedCommand));

  switch (receivedCommand) {
    case FORWARD:
      if (!programmingMode && !forwardComplete) {
        forwardComplete = true;
        forwardActive = true;
        startTime = millis();
        digitalWrite(RELAY_FORWARD_PIN, HIGH);
      }
      else {
        stopForceForward = true;
      }
      break;
    case BACKWARD:
      if (!programmingMode && !backwardComplete) {
        backwardComplete = true;
        backwardActive = true;
        startTime = millis();
        digitalWrite(RELAY_BACKWARD_PIN, HIGH);
      }
      else{
        stopForceBackward = true;
      }
      break;
    case PROGRAMMING:
      programmingMode = true;
      break;
    case SAVE:
      if (programmingMode) {
        EEPROM.put(6, holdForwardDuration);
        EEPROM.put(40, holdBackwardDuration);
        EEPROM.commit();
        programmingMode = false; // Exit programming mode after saving
        Serial.println("Data from Eeprom");
        Serial.println(holdForwardDuration);
        Serial.println(holdBackwardDuration);
      }
      break;
    case HOLD_FOR:
      if (programmingMode) {
        holdForwardActive = true;
        startTime = millis();
        digitalWrite(RELAY_FORWARD_PIN, HIGH);
      }
      break;
    case HOLD_BACK:
      if (programmingMode) {
        holdBackwardActive = true;
        startTime = millis();
        digitalWrite(RELAY_BACKWARD_PIN, HIGH);
      }
      break;
    case RESTART:
      ESP.restart();
      break;
    default:
      break;
  }
}

void savePosition() {
  // Save the current state and duration times to EEPROM
  // unsigned long currentTime = millis();
  EEPROM.put(2, holdForwardDuration);
  EEPROM.put(4, holdBackwardDuration);
  // EEPROM.write(10, programmingMode);
  // EEPROM.write(12, holdForwardActive);
  // EEPROM.write(14, holdBackwardActive);
  // EEPROM.write(16, startTime);
  // EEPROM.write(18, currentTime);
  EEPROM.commit();
}

void loadPosition() {
  // unsigned long storedTime;
  EEPROM.get(6, holdForwardDuration);
  EEPROM.get(40, holdBackwardDuration);
  // EEPROM.get(10, programmingMode);
  // EEPROM.get(12, holdForwardActive);
  // EEPROM.get(14, holdBackwardActive);
  // EEPROM.get(16, startTime);
  // // EEPROM.get(18, storedTime);
  // relayResumeTime = millis() - storedTime + startTime;
}

void setup() {
  
  digitalWrite(RELAY_FORWARD_PIN, LOW);  // Ensure relay is off initially
  digitalWrite(RELAY_BACKWARD_PIN, LOW); // Ensure relay is off initially
  Serial.begin(115200);

  // Set GPIO pins
  pinMode(RELAY_FORWARD_PIN, OUTPUT);
  pinMode(RELAY_BACKWARD_PIN, OUTPUT);
  pinMode(POWER_PIN, INPUT_PULLUP);


  // Initialize EEPROM
  EEPROM.begin(512);

  // Initialize WiFi in station mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for receiving data
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);

  // Load the previous state from EEPROM
  loadPosition();
  Serial.println("Data from Eeprom");
  Serial.println(holdForwardDuration);
  Serial.println(holdBackwardDuration);
}

void loop() {
  // Check if the power is going down
  int a = digitalRead(POWER_PIN);
   if (a == LOW) {
  //   savePosition();  // Save the position and duration time in EEPROM
  //   while (digitalRead(POWER_PIN) == LOW);  // Wait until power is restored
  //   powerRestored = true;
    Serial.println("POWER OFF DETECT");
   }

  // if (powerRestored) {
  //   if (holdForwardActive) {
  //     digitalWrite(RELAY_FORWARD_PIN, HIGH);
  //     startTime = relayResumeTime;   
  //     powerRestored = false;
  //   }

  //   if (holdBackwardActive) {
  //     digitalWrite(RELAY_BACKWARD_PIN, HIGH);
  //     startTime = relayResumeTime;
  //     powerRestored = false;
  //   }
  // }

  // Handle relay timing without blocking
  if (programmingMode) {
    if ( stopForceForward){
      stopForceForward = false;
      digitalWrite(RELAY_FORWARD_PIN , LOW);
      holdForwardDuration = millis() - startTime;
      
    }

    if (stopForceBackward) {
      stopForceBackward = false;
      digitalWrite(RELAY_BACKWARD_PIN , LOW);
      holdBackwardDuration = millis() - startTime;
    }
  } else {
    if (forwardActive && (millis() - startTime >= holdForwardDuration)) {
      forwardActive = false;        
      digitalWrite(RELAY_FORWARD_PIN, LOW);
      backwardComplete = false;
    }

    if (backwardActive && (millis() - startTime >= holdBackwardDuration)) {
      backwardActive = false;
      digitalWrite(RELAY_BACKWARD_PIN, LOW);
      forwardComplete = false ;
    }
  }
}
