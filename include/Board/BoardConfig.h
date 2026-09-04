#pragma once

#include <cstddef>
#include <cstdint>

#include "Devices/ADCConfig.h"

struct BoardConfig
{
    // Core hardware
    std::uint64_t clockHz = 16'000'000;
    std::size_t ramWords = 1024;

    std::size_t gpioPins = 8;

    // Electrical configuration
    double logicVoltage = 5.0;
    double digitalHighThreshold = 2.5;

    // Memory map
    std::uint32_t ramBase   = 0x00000000;
    std::uint32_t gpioBase  = 0x00001000;
    std::uint32_t timerBase = 0x00002000;
	std::uint32_t canBase = 0x00005000;
	std::uint32_t adcBase = 0x00003000;
	
	//ADC configuration
	ADCConfig adc;
};