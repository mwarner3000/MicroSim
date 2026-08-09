#pragma once

#include <vector>

#include "Simulator/IClockable.h"
#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Memory/RAM.h"
#include "Simulator/Clock.h"
#include "Devices/Timer.h"
#include "CPU/SimpleCPU.h"

class Simulator
{
public:
    Simulator();

    Bus& getBus();
    RAM& getRAM();
    GPIO& getGPIO();
    Clock& getClock();
	void tick();
	void addClockable(IClockable& device);
	Timer& getTimer();
	SimpleCPU& getCPU();

private:
    Bus bus;
	RAM ram;
	GPIO gpio;
	Timer timer;
	SimpleCPU cpu;
	Clock clock;
	std::vector<IClockable*> clockables;
};