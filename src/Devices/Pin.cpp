#include "Devices/Pin.h"

Pin::Pin()
    : voltage(0.0),
      direction(PinDirection::Input)
{
}

double Pin::getVoltage() const
{
    return voltage;
}

void Pin::setVoltage(double volts)
{
    voltage = volts;
}

PinDirection Pin::getDirection() const
{
    return direction;
}

void Pin::setDirection(PinDirection newDirection)
{
    direction = newDirection;
}