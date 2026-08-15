#pragma once

#include <cstddef>
#include <cstdint>

struct BoardConfig
{
	
    // Default generic board configuration.
    // Users may override any value before constructing a Simulator.	
    std::uint64_t clockHz = 16'000'000;
    std::size_t ramWords = 1024;

    // We'll make these functional later.
    std::size_t gpioPins = 8;
    std::size_t timerCount = 1;
	
	double logicVoltage = 5.0;
	double digitalHighThreshold = 2.5;	
};