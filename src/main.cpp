#include <Arduino.h>
#include <filterRC.h>

#include "roaster.h"
#include "eeprom_settings.h"
#include "state.h"
#include "logging.h"
#include "commands.h"
#include "safemon.h"
#include "skywalker_remote_comms.h"


// NVM container & default settings
PROGMEM const static t_Settings nvmSettingsStorage = {
#ifdef USE_STEPPER_DRUM
    STEPPER_STEPS_PER_REV, // stepsPerRevolution;
    STEPPER_MAX_RPM, // Max RPM for stepper drum driver
#endif  // USE_STEPPER_DRUM
    MAX_SAFE_TEMP_C,
    PWM_FREQ_COOL,
    PWM_FREQ_DRUM,
    PWM_FREQ_EXHAUST,
    PWM_FREQ_HEAT,
    PWM_FREQ_LED,
    // Counters
    0, // power on resets
    0, // Watchdog resets
    0, // safetyTriggers
    EEPROM_SETTINGS_MAGIC, // EEPROM MAGIC number
    0 // CRC
};


/*
 * Until this is replaced by an Class, this structure
 * contains the entire state, status, config, reported
 * and commanded roaster status
 */
EepromSettings nvmSettings = EepromSettings( &nvmSettingsStorage );
State state = State( &nvmSettings );
SafetyMonitor safeMon = SafetyMonitor( &state, nvmSettings.settings.maxSafeTempC );
SkywalkerRemoteComm skwRemoteComm = SkywalkerRemoteComm( &state );

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println(F(VERSION));

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST)) {
      nvmSettings.settings.counters.powerOnResets++;
      nvmSettings.markDirty();
  }
  __HAL_RCC_CLEAR_RESET_FLAGS();

  safeMon.begin();
  skwRemoteComm.begin();

  while ( !(state.begin()) ) {
    Serial.println(F("Failed to initialize"));
    delay(100);
  };

  setupCommandHandlers();

}

void loop() {
  // for loop timing statistics
  state.stats.loopStart();

  // Check Serial Communication
  commandsLoopTick();

  state.loopTick();

  safeMon.loopTick();

  skwRemoteComm.loopTick();

  state.stats.loopEnd();
}