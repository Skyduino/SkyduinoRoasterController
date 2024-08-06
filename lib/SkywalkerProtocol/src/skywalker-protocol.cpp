#include "skywalker-protocol.h"

#ifndef WARN
#define WARN(...)
#endif
#ifndef WARNLN
#define WARNLN(...)
#endif

// Remote to Roster
#define BYTE_POS_AIR_FAN            0
#define BYTE_POS_FILTER_FAN         1
#define BYTE_POS_COOLING_FAN        2
#define BYTE_POS_DRUM               3
#define BYTE_POS_HEAT               4

// Roaster to Remote
#define BYTE_POS_TEMP_HIGH_MSB      0
#define BYTE_POS_TEMP_HIGH_LSB      1
#define BYTE_POS_TEMP_LOW_MSB       2
#define BYTE_POS_TEMP_LOW_LSB       3
#define BYTE_POS_DRUM_LOAD          5


#define SWPROT_TX_1_LENGTH_US       1500
#define SWPROT_TX_0_LENGTH_US       650
#define SWPROT_TX_PREAMBLE_LOW_US   7500UL
#define SWPROT_TX_PREAMBLE_HIGH_US  3800UL
// number of attempts to receive a preamble
#define SWPROT_RX_PREAMBLE_ATTEMPTS 60
#define SWPROT_RX_PREAMBLE_LOW_US   7000U
#define SWPROT_RX_1_LENGTH_US       1200U
#define MESSAGE_TIMEOUT_MS          3000UL


/*
 * Base Protocol constuctor: clear up the buffer
 */
_SWProtocolBase::_SWProtocolBase(uint8_t *buffer, size_t bufferSize) : buffer(buffer), bufferSize(bufferSize) {
    _clearBuffer();
};


/*
 * Calculate checksum of the buffer
 */
uint8_t _SWProtocolBase::calculateCRC() {
    uint8_t crc = 0;
    for (unsigned int i = 0; i < (bufferSize - 1); i++) {
        crc += buffer[i];
    }
    return crc;
}


/*
 * Zero out the buffer, aka shutdown
 */
void _SWProtocolBase::_clearBuffer() {
    for (unsigned int i = 0; i < bufferSize; i++) {
        buffer[i] = 0;
    }
};

void _SWProtocolBase::shutdown() {
    return _clearBuffer();
}


/*
 * Initialize TX protocol
 */
void _SWProtocolTx::begin(void) {
  pinMode(pin, OUTPUT);
}


/*
 * update checksum based on the buffer content
 */
void _SWProtocolTx::updateCRC() {
    buffer[bufferSize - 1] = calculateCRC();
}


/*
 * pulse the pin for 1 or 0
 */
void _SWProtocolTx::sendBit(uint8_t value) {
  //Assuming pin is HIGH when we get it
  digitalWrite(pin, LOW);
  delayMicroseconds(value ? SWPROT_TX_1_LENGTH_US : SWPROT_TX_0_LENGTH_US);
  digitalWrite(pin, HIGH);
  delayMicroseconds(SWPROT_TX_1_LENGTH_US >> 1);  //delay between bits
  //we leave it high
}


/*
 * send preamble
 */
void _SWProtocolTx::sendPreamble() {
    //Assuming pin is HIGH when we get it
    digitalWrite(pin, LOW);
    delayMicroseconds(SWPROT_TX_PREAMBLE_LOW_US);
    digitalWrite(pin, HIGH);
    delayMicroseconds(SWPROT_TX_PREAMBLE_HIGH_US);
    //we leave it high
}


/*
 * Send current buffer
 */
void _SWProtocolTx::sendMessage() {
    // Update crc, at the time of sending
    updateCRC();
    // send Preamble
    sendPreamble();

    // send Message
    for (uint8_t i = 0; i < bufferSize; i++) {
        for (uint8_t bit = 0; bit < 8; bit++) {
            sendBit(bitRead(buffer[i], bit));
        }
    }
}


/*
 * Set byte in buffer. returns true if successful
 */
bool _SWProtocolTx::setByte(uint8_t idx, uint8_t value) {
    if (idx > bufferSize) return false;

    buffer[idx] = value;
    return true;
}


/*
 * RX Constructor
 */
_SWProtocolRx::_SWProtocolRx(uint32_t rxpin, uint8_t *buffer, size_t bufferSize):
            _SWProtocolBase(buffer, bufferSize),
            pin(rxpin) {

    // Initialize instance
    lastSuccRx = 0;
    attemptCount = 0;
};


/*
 * verify CRC, return true of crc matches buffer content
 */
bool _SWProtocolRx::verifyCRC() {
    return buffer[bufferSize - 1] == calculateCRC();
}


/*
 * get byte from buffer. returns true if successful
 */
bool _SWProtocolRx::getByte(uint8_t idx, uint8_t *value) {
    if (idx > bufferSize) return false;

    *value = buffer[idx];
    return true;
}


/*
 * Return true when the reciever is in sync with the transmitter,
 * i.e. we successfuly recieved a frame not too long ago
 */
bool _SWProtocolRx::isSynchronized() {
    return (micros() - lastSuccRx) < (MESSAGE_TIMEOUT_MS * 1000);
}


/*
 * Recieve frame. Return true if successfully received
 */
bool _SWProtocolRx::receiveFrame() {
  unsigned long pulseDuration = 0;

  // clear buffer and reset checksum
  _clearBuffer();

  uint8_t attempts = 0;
  bool preambleDetected = false;
  do {
    pulseDuration = pulseIn(pin, LOW, SWPROT_TICK_INTERVAL_US);
    if (pulseDuration == 0) {
        return false;
    } else if ( pulseDuration >= SWPROT_RX_PREAMBLE_LOW_US) {
      preambleDetected = true;
      break;
    }
    attempts++;
  } while (attempts <= SWPROT_RX_PREAMBLE_ATTEMPTS);

  if (!preambleDetected) {
    WARN(F("Did not detect preamble after "));
    WARN(attempts);
    WARNLN(F(" attempts"));
    return false;
  }

  for (uint8_t i = 0; i < (bufferSize << 3); i++) {  //Read the proper number of bits..
    pulseDuration = pulseIn(pin, LOW);
    if (pulseDuration == 0) {
      WARN(F("Failed to get bit #"));
      WARNLN(i);
      return false;
    }
    //Bits are received in LSB order..
    if (pulseDuration > SWPROT_RX_1_LENGTH_US) {  // we received a 1
      buffer[i / 8] |= (1 << (i % 8));
    }
  }

  return true;
}


/*
 * receive the message
 */
bool _SWProtocolRx::getMessage() {
  if ( !( receiveFrame() and verifyCRC())) {
    // timeout receiving message or receiving it correctly
    attemptCount++;
    return false;
  }

  // received message and passed checksum verification

  if (attemptCount > 0) {
    WARN(F("[!] WARN: Took "));
    WARN(count);
    WARNLN(F(" tries to recieve a frame."));
  }
  attemptCount = 0;
  lastSuccRx = micros();

  return true;
}


/*
 * Initialize RX protocol
 */
void _SWProtocolRx::begin(void) {
  pinMode(pin, INPUT);
}


/**
 * @brief Get ADC value representing temperature for the higher temperature
 *        when there's an addition 2K 5V pull resistor is added to the existing
 *        20K pull up
 * 
 * @return returns current ADC value, bytes 0 & 1. 
 */
uint16_t _SWRoaster::getHighTempADC() {
    return bufMemory[BYTE_POS_TEMP_HIGH_MSB] << 8 
           | bufMemory[BYTE_POS_TEMP_HIGH_LSB];
}


/**
 * @brief Set ADC value for the higher temperature range
 */
void _SWRoaster::setHighTempADC(uint16_t adc) {
    // MSB order
    bufMemory[BYTE_POS_TEMP_HIGH_MSB] = adc >> 8;
    bufMemory[BYTE_POS_TEMP_HIGH_LSB] = adc & 0xFF;
}


/**
 * @brief Get ADC value representing temperature for the lower temperature range
 *        with just the 20K pull up resistor
 * 
 * @return returns current ADC value, bytes 2 & 3. 
 */
uint16_t _SWRoaster::getLowTempADC() {
    return bufMemory[BYTE_POS_TEMP_LOW_MSB] << 8
           | bufMemory[BYTE_POS_TEMP_LOW_LSB];
}


/**
 * @brief Set ADC value for the lower temperature range
 */
void _SWRoaster::setLowTempADC(uint16_t adc) {
    // MSB Order
    bufMemory[BYTE_POS_TEMP_LOW_MSB] = adc >> 8;
    bufMemory[BYTE_POS_TEMP_LOW_LSB] = adc & 0xFF;
}


/**
 * @brief Get drum load, byte 5
 */
uint8_t _SWRoaster::getDrumLoad() {
    return bufMemory[BYTE_POS_DRUM_LOAD];
}


/**
 * @brief Set drum load, byte 5
 */
void _SWRoaster::setDrumLoad(uint8_t load) {
    bufMemory[BYTE_POS_DRUM_LOAD] = load;
}


/**
 * @brief Get current air circulation fan setting
 *
 */
uint8_t _SWRemote::getAirFan() {
    return bufMemory[BYTE_POS_AIR_FAN];
}


/**
 * @brief set air circulation fan speed
 * @param value -- fan speed duty cycle 0-100
 */
void _SWRemote::setAirFan(uint8_t value) {
    if ( value > 100 ) {
        value = 100;
    }

    bufMemory[BYTE_POS_AIR_FAN] = value;
}


/**
 * @brief Get current filter fan setting
 *
 */
uint8_t _SWRemote::getFilterFan() {
    return bufMemory[BYTE_POS_FILTER_FAN];
}


/**
 * @brief set filter fan speed
 * @param value -- fan speed duty cycle 0-100
 */
void _SWRemote::setFilterFan(uint8_t value) {
    if ( value > 100 ) {
        value = 100;
    }

    bufMemory[BYTE_POS_FILTER_FAN] = value;
}


/**
 * @brief Get current cooling fan speed
 *
 */
uint8_t _SWRemote::getCoolingFan() {
    return bufMemory[BYTE_POS_FILTER_FAN];
}


/**
 * @brief set cooling fan speed
 * @param value -- fan speed duty cycle 0-100
 */
void _SWRemote::setCoolingFan(uint8_t value) {
    if ( value > 100 ) {
        value = 100;
    }

    bufMemory[BYTE_POS_FILTER_FAN] = value;
}


/**
 * @brief Get current drum speed
 *
 */
uint8_t _SWRemote::getDrumSpeed() {
    return bufMemory[BYTE_POS_DRUM];
}


/**
 * @brief set drum speed
 * @param value -- speed duty cycle 0-100
 */
void _SWRemote::setDrumSpeed(uint8_t value) {
    if ( value > 100 ) {
        value = 100;
    }

    bufMemory[BYTE_POS_DRUM] = value;
}


/**
 * @brief Get current heat setting
 *
 */
uint8_t _SWRemote::getHeat() {
    return bufMemory[BYTE_POS_HEAT];
}


/**
 * @brief set heating value
 * @param value -- heating duty cycle 0-100
 */
void _SWRemote::setHeat(uint8_t value) {
    if ( value > 100 ) {
        value = 100;
    }

    bufMemory[BYTE_POS_HEAT] = value;
}
