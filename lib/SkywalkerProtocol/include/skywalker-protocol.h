#ifndef __SKYWALKER_PROTOCOL_H
#define __SKYWALKER_PROTOCOL_H

#include <Arduino.h>

#define MESSAGE_LENGTH_ROASTER      7U
#define MESSAGE_LENGTH_REMOTE       6U
#define SWPROT_TICK_INTERVAL_US     200000UL


class _SWProtocolBase {
    protected:
        uint8_t *buffer;
        size_t   bufferSize;
        uint32_t tickInterval, lastTick;
        uint8_t calculateCRC();
        // constructor for ProtocolTx/Rx child classes
        _SWProtocolBase() {};
        _SWProtocolBase(uint8_t *buffer, size_t bufferSize);
        void _clearBuffer();
        virtual void tickIntervalHandler() {};
    public:
        virtual void begin() =0;
        virtual void loopTick();
        void setTickInterval(uint32_t interval);
        void shutdown();
};


class _SWProtocolTx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        _SWProtocolTx(uint32_t txpin, uint8_t *buffer, size_t bufferSize):
            _SWProtocolBase(buffer, bufferSize), pin(txpin) {};
        void updateCRC();
        void sendBit(uint8_t value);
        void sendPreamble();
        void tickIntervalHandler();
    public:
        void begin();
        bool setByte(uint8_t idx, uint8_t value);
        void sendMessage();
};


class _SWProtocolRx: public _SWProtocolBase {
    protected:
        uint32_t pin;
        uint32_t lastSuccRx;
        uint32_t attemptCount;
        _SWProtocolRx(uint32_t rxpin, uint8_t *buffer, size_t bufferSize);
        bool receiveFrame();
        bool verifyCRC();
        void tickIntervalHandler();
    public:
        void begin();
        bool getByte(uint8_t idx, uint8_t *value);
        bool getMessage();
        bool isSynchronized();
};


/**
 * @brief Receive data from the roaster AKA pretend to be a remote on the
 *        receiving end. Roaster controller sends data to the remote on
 *        USB D+ (Green) line.
 *        Roaster sends (remote receives) 7 bytes:
 *        bytes[0:1] -- ADC Temp 1
 *        bytes[2:3] -- ADC Temp 2
 *        bytes[4] -- uknown, always 0
 *        bytes[5] -- Drum motor current/load
 *        bytes[6] -- CRC
 * @param pin: PA10 -- pin used for communication (receive)
 */
class SWRoasterRx: public _SWProtocolRx {
    private:
        uint8_t bufMemory[MESSAGE_LENGTH_ROASTER];
    public:
        SWRoasterRx(uint8_t pin):
            _SWProtocolRx(pin, bufMemory, MESSAGE_LENGTH_ROASTER) {};
};


/**
 * @brief Send roaster data AKA pretend to be a roaster on the transmitting end.
 *        Roaster controller sends data to the remote on USB D+ (Green) line.
 *        Roaster sends (remote receives) 7 bytes:
 *        bytes[0:1] -- ADC Temp 1
 *        bytes[2:3] -- ADC Temp 2
 *        bytes[4] -- uknown, always 0
 *        bytes[5] -- Drum motor current/load
 *        bytes[6] -- CRC
 * @param pin: PA10 -- pin used for communication (transmit)
 */
class SWRoasterTx: public _SWProtocolTx {
    private:
        uint8_t bufMemory[MESSAGE_LENGTH_ROASTER];
    public:
        SWRoasterTx(uint8_t pin):
            _SWProtocolTx(pin, bufMemory, MESSAGE_LENGTH_ROASTER) {};
};


/**
 * @brief Send "remote" data AKA pretend to be remote and control the roaster.
 *        Roaster remote uses USB D- (White) line for communication with the 
 *        roaster controller.
 *        Message consist of 6 bytes:
 *        bytes[0] -- Fan duty cycle, 0 -- 100
 *        bytes[1] -- Filter fan, appears to be 0, 1, 2, 3, 4
 *        bytes[2] -- Bean Cooling FAN, 0 or 100
 *        bytes[3] -- Drum, 0 or 100
 *        bytes[4] -- Heater duty, 0 -- 100
 *        bytes[5] -- CRC
 * @param pin: PA9 -- pin used for communication (receive)
 */
class SWRemoteTx: public _SWProtocolTx{
    protected:
        uint8_t bufMemory[MESSAGE_LENGTH_REMOTE];
    public:
        SWRemoteTx(uint8_t pin):
            _SWProtocolTx(pin, bufMemory, MESSAGE_LENGTH_REMOTE) {};
};


/**
 * @brief Receive "remote" data AKA pretend to be a reoaster and receive a
 *        control message from the remote.
 *        Roaster remote uses USB D- (White) line for communication with the 
 *        roaster controller.
 *        Message consist of 6 bytes:
 *        bytes[0] -- Fan duty cycle, 0 -- 100
 *        bytes[1] -- Filter fan, appears to be 0, 1, 2, 3, 4
 *        bytes[2] -- Bean Cooling FAN, 0 or 100
 *        bytes[3] -- Drum, 0 or 100
 *        bytes[4] -- Heater duty, 0 -- 100
 *        bytes[5] -- CRC
 * @param pin: PA9 -- pin used for communication (transmit)
 */
class SWRemoteRx: public _SWProtocolRx{
    protected:
        uint8_t bufMemory[MESSAGE_LENGTH_REMOTE];
    public:
        SWRemoteRx(uint8_t pin):
            _SWProtocolRx(pin, bufMemory, MESSAGE_LENGTH_REMOTE) {};
};


#endif  // __SKYWALKER_PROTOCOL_H