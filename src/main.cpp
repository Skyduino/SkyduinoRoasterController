#include <Arduino.h>
#include <filterRC.h>
#ifndef __DEBUG__
#include <IWatchdog.h>
#endif
#include <status-led.h>

#include "roaster.h"
#include "eeprom_settings.h"
#include "state.h"
#include "logging.h"
#include "commands.h"
#include "safemon.h"
#include "skywalker_remote_comms.h"

// pidProfiles: kP, kI, kD, pmode, dmode, iAwMode, chan, fanSetpointError, ConservProfileSwitchingThresholdC, CycleTimeMS
#define DEFAULT_PID_PROFILE {PID_KP, PID_KI, PID_KD, PID_PMODE, PID_DMODE, PID_AWMODE, PID_CHAN, PID_FAN_ERR_C, PID_CONSERV_ERR, PID_CYCLE_TIME_MS}

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
    {
        0, // power on resets
        0, // Watchdog resets
        0, // Software resets
        0  // safetyTriggers
    },
    0, // pidCurrentProfile
    0, // pidConservProfile
    2, // pidFanProfile
    // pidProfiles: kP, kI, kD, pmode, dmode, iAwMode, chan, fanSetpointError, CycleTimeMS
    {
        DEFAULT_PID_PROFILE,
        DEFAULT_PID_PROFILE,
        DEFAULT_PID_PROFILE,
        DEFAULT_PID_PROFILE,
        DEFAULT_PID_PROFILE
    },
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
SafetyMonitor safeMon = SafetyMonitor(
                            &state,
                            nvmSettings.settings.maxSafeTempC,
                            std::bind(&EepromSettings::incSafetyCounter, &nvmSettings));
SkywalkerRemoteComm skwRemoteComm = SkywalkerRemoteComm( &state );

void setup() {
  StatusLed.begin();
  StatusLed.turnOn();
  Serial.begin(115200);
  Serial.setTimeout(100);
  Serial.println(F(VERSION));

  if ( __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) ) {
      nvmSettings.settings.counters.powerOnResets++;
      nvmSettings.markDirty();
  } else if ( __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) ) {
      nvmSettings.settings.counters.watchdogResets++;
      nvmSettings.markDirty();
      Serial.println(F("Watchdog timer triggered, what's going on"));
  } else if ( __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) ) {
      nvmSettings.settings.counters.softResets++;
      nvmSettings.markDirty();
  }
  __HAL_RCC_CLEAR_RESET_FLAGS();

  safeMon.begin();
  skwRemoteComm.begin();

#ifndef __DEBUG__
  IWatchdog.begin( WATCHDOG_TIMEOUT_MS * 1000 );
#endif

  while ( !(state.begin()) ) {
    Serial.println(F("Failed to initialize"));
    delay(100);
  };

  setupCommandHandlers();



}

void loop() {
  // for loop timing statistics
  state.stats.loopStart();
  StatusLed.turnOff();

#ifndef __DEBUG__
  IWatchdog.reload();
#endif

  // Check Serial Communication
  commandsLoopTick();

  state.loopTick();

  safeMon.loopTick();

#ifndef __DEBUG__
  IWatchdog.reload();
#endif
  skwRemoteComm.loopTick();

  state.stats.loopEnd();
}

extern "C" {
void HardFault_Handler(void) {
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_3);        // Turn on the status led
    uint32_t *stack_pointer = (uint32_t *)__get_MSP(); // Get Main Stack Pointer
    [[maybe_unused]] uint32_t fault_address = stack_pointer[6];         // Program Counter at fault
    [[maybe_unused]] uint32_t r0 = stack_pointer[0];   // Register R0
    [[maybe_unused]] uint32_t r1 = stack_pointer[1];   // Register R1
    [[maybe_unused]] uint32_t r2 = stack_pointer[2];   // Register R2
    [[maybe_unused]] uint32_t r3 = stack_pointer[3];   // Register R3
    [[maybe_unused]] uint32_t r12 = stack_pointer[4];  // Register R12
    [[maybe_unused]] uint32_t lr = stack_pointer[5];   // Link Register (return address)
    [[maybe_unused]] uint32_t psr = stack_pointer[7];  // Program Status Register
    while (1) {
    }
}
}