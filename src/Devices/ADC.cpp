#include "Devices/ADC.h"

#include <stdexcept>

ADC::ADC(const ADCConfig& config)
    : config(config)
{
	if (config.channelCount == 0)
	{
		throw std::invalid_argument(
			"ADC channel count must be greater than zero"
		);
	}
	
	if (config.resolutionBits < 1 || config.resolutionBits > 16)
	{
		throw std::invalid_argument(
			"ADC resolution must be between 1 and 16 bits"
		);
	}
	
	if (config.referenceVoltage <= 0.0)
	{
		throw std::invalid_argument(
			"ADC reference voltage must be greater than zero"
		);
	}
	
	if (config.conversionCycles ==0)
	{
		throw std::invalid_argument(
			"ADC conversion cycles must be greater than zero"
		);
	}
}