#include "skywalker-protocol.h"

#ifndef WARN
#define WARN(...)
#endif
#ifndef WARNLN
#define WARNLN(...)
#endif

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