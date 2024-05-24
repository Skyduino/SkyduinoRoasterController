#include <Arduino.h>
#define LED PA3

void setup() {
  pinMode(LED, OUTPUT);
  SystemClock_Config();
}

void loop() {
  while (true) {
    digitalWrite(LED, HIGH);
    delay(300);

    digitalWrite(LED, LOW);
    delay(300);

  }
}