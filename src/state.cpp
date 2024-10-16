#include "roaster.h"
#include "state.h"


Config::Config() {
    isMetric = false;
}

Status::Status() {
    tcStatus = 1;
}


/**
 * @brief emergency shutdown and control lockout
 */
void State::abort() {
    this->pid.abort();
    this->commanded.abort();
}


bool State::begin()
{
    bool isSuccess = true;

    isSuccess &= commanded.begin();
    isSuccess &= reported.begin();
    isSuccess &= pid.begin();
    pid.addGetLogicalChantTempC(
        std::bind(
            &Reported::getLogicalChanTemp,
            &reported,
            std::placeholders::_1
        )
    );

    return isSuccess;
}

bool State::loopTick() {
    bool isSuccess = true;

    isSuccess &= nvmSettings->loopTick();
    isSuccess &= commanded.loopTick();
    isSuccess &= reported.loopTick();

    return true;
    return isSuccess;
}


/**
 * Outputs to serial the current roaster state in response to READ command
 * Print out comma separated values for ambient temp, channel temps, OT1, OT2
*/
void State::printState() {
    reported.printState();
    commanded.printState();
    if ( pid.isOn() ) {
        float spC = pid.getSetPoint();
        Serial.println(
            cfg.isMetric ? spC : CONVERT_C_TO_F( spC )
        );
    } else {
        Serial.println(0);
    }
}

/**
 * @brief print all the available statistics
 */
void State::printStatistics() {
    this->nvmSettings->print();
    this->reported.printStatistics();
    this->stats.print();
}
