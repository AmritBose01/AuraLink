#include <Arduino.h>

// Define pins
const int fingerPins[] = {32, 33, 34, 35, 36};
const char* fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) {
    pinMode(fingerPins[i], INPUT);
  }
}

void loop() {
  Serial.print(">>> GESTURE DATA: ");
  for (int i = 0; i < 5; i++) {
    int val = analogRead(fingerPins[i]);
    // Map 0-4095 (ESP32 ADC) to 0-100% for easier logic
    int percent = map(val, 0, 4095, 0, 100);
    
    Serial.print(fingerNames[i]);
    Serial.print(": ");
    Serial.print(percent);
    Serial.print("% | ");
  }
  Serial.println();
  delay(200); 
}
