#pragma once

#include <cstddef>
#include <cstdint>

struct ADCConfig
{
	std::size_t channelCount = 8;
	std::uint8_t resolutionBits = 10;
	double referenceVoltage = 5.0;
	std::uint32_t conversionCycles = 20;
	
};