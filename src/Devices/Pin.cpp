#include <stdexcept>

#include "Devices/Pin.h"

Pin::Pin()
    : direction(PinDirection::Input),
	  outputLatch(false),
	  externalVoltage(std::nullopt)
{
}

PinDirection Pin::getDirection() const
{
    return direction;
}

void Pin::setDirection(PinDirection newDirection)
{
    direction = newDirection;
}

bool Pin::getOutputLatch() const
{
	return outputLatch;
}
void Pin::setOutputLatch(bool value)
{
	outputLatch = value;
}

bool Pin::hasExternalVoltage() const
{
    return externalVoltage.has_value();
}

double Pin::getExternalVoltage() const
{
	if(!externalVoltage.has_value())
	{
		throw std::runtime_error(
			"Pin has no external voltage"
		);
	}
	
    return externalVoltage.value();
}

void Pin::setExternalVoltage(double voltage)
{
	externalVoltage = voltage;
}

void Pin::clearExternalVoltage()
{
	externalVoltage = std::nullopt;
}

std::optional<double> Pin::getEffectiveVoltage(
    double logicVoltage
) const
{
    if (direction == PinDirection::Output)
    {
        return outputLatch
            ? logicVoltage
            : 0.0;
    }

    return externalVoltage;
}