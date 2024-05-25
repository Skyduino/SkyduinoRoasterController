#include <Arduino.h>
#define LED PA3

#include "roaster.h"
#include "logging.h"
#include "tick-timer.h"
#include "commands.h"


/*
 * Until this is replaced by an Class, this structure
 * contains the entire state, status, config, reported
 * and commanded roaster status
 */
t_State state = {
  // t_StateCommanded
  {0, 0, 0, 0, 0},
  // t_StateReported
  {0, 0, 0, 0},
  // t_Config
  {
    // chanMapping
    {TEMPERATURE_CHANNEL_ROASTER+1, TEMPERATURE_CHANNEL_THERMOCOUPLE+1, 0, 0},
    'F',
  },
  // t_Status
  {
    TimerMS(TC4_COMM_TIMEOUT_MS),      // tc4ComTimeOut
    1       // tcStatus
  }
};


static bool is_led_on = false;
static TimerMS timer_led_toggle = TimerMS(300);


void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial.setTimeout(100);

  setupCommandHandlers();
  Serial.println(F(VERSION));
}

void loop() {
  // LED Heartbeat
  if (timer_led_toggle.hasTicked()) {
    is_led_on = !is_led_on;
    is_led_on ? digitalWrite(LED, HIGH) : digitalWrite(LED, LOW);
    timer_led_toggle.reset();
  }

  // Check Serial Communication
  commandsLoopTick();
}