#ifndef __ROASTER_H
#define __ROASTER_H

#define VERSION "Skyduino version " SKYDUINO_VERSION " " SKYDUINO_COMMIT

// TC4 protocol communication timeout in milliseconds
#define TC4_COMM_TIMEOUT_MS                 10000UL
// physical channel for thermocouple temperature
#define TEMPERATURE_CHANNEL_THERMOCOUPLE    0
// physical channel for temperature reported by roaster
#define TEMPERATURE_CHANNEL_ROASTER         1

#define TEMPERATURE_ROASTER(x) x[TEMPERATURE_CHANNEL_ROASTER]
#define TEMPERATURE_TC(x)      x[TEMPERATURE_CHANNEL_THERMOCOUPLE]
#define CONVERT_C_TO_F(C)      (C * 9.0f / 5.0f + 32.0f)
#define CONVERT_F_TO_C(F)      (5.0f * ( F - 32.0f ) / 9.0f)

// PIN Definitions
#define PIN_LED           PA3
#define PIN_EXHAUST       PA8
#define PIN_DATA_FROM_RMT PA9
#define PIN_DATA_TO_RMT   PA10
#define PIN_DRUM          PA15
#define PIN_HEAT_RELAY    PB8
#define PIN_HEAT          PB9_ALT1
#define PIN_NTC           PC5
#define PIN_COOL          PC8

#define SPI_BTCS          PA4

// Allow use of DRV8855 stepper motor for drum driving
#ifdef USE_STEPPER_DRUM

#ifndef PIN_STEPPER_STEP
#define PIN_STEPPER_STEP PB6
#endif  // PIN_STEPPER_STEP

#ifndef PIN_STEPPER_EN
#define PIN_STEPPER_EN   PB7
#endif  // PIN_STEPPER_EN

#define STEPPER_STEPS_PER_REV 1600
#define STEPPER_MAX_RPM       60

#endif  // USE_STEPPER_DRUM


// PWM frequencies Hz
#define PWM_FREQ_COOL       60
#define PWM_FREQ_DRUM       60
#define PWM_FREQ_EXHAUST    60
#define PWM_FREQ_HEAT       1
#define PWM_FREQ_LED        120

#define EEPROM_SETTINGS_MAGIC 0xbeefbeef
#define EEPROM_SAVE_TIME_MS   5000
#define EEPROM_SETTINGS_ADDR  0


// PID definitions
#define PID_KP              5.00f
#define PID_KI              0.15f
#define PID_KD              0.00f
#define PID_CHAN            2
#define PID_CYCLE_TIME_MS   1000U
#define PID_PMODE           QuickPID::pMode::pOnError
#define PID_DMODE           QuickPID::dMode::dOnMeas
#define PID_AWMODE          QuickPID::iAwMode::iAwCondition
#define PID_NUM_PROFILES    4
#endif  // __ROASTER_H