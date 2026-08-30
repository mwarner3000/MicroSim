#pragma once

#include <cstdint>
#include <cstddef>

#include "Bus/IBusDevice.h"
#include "Simulator/IClockable.h"
#include "Devices/ADCConfig.h"

class ADC : public IBusDevice, public IClockable
{
public:
	ADC(const ADCConfig& config);
	std::uint32_t read(std::uint32_t address) override;
	void write(std::uint32_t address, 
			   std::uint32_t value) override;
    void tick(std::uint64_t cycle) override;
	
private:
	ADCConfig config;
	
	std::size_t selectedChannel = 0;
	std::size_t activeChannel = 0;
	double sampledVoltage = 0.0;
	std::uint16_t result = 0;
	bool busy = false;
	bool complete = false;
	bool overrun = false;
	bool invalidChannel = false;
	bool resultUnread = false;
	bool interruptEnable = false;

	std::uint32_t cyclesRemaining = 0;
	
};