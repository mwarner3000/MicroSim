#pragma once

#include <vector>

#include "Simulator/IClockable.h"
#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Memory/RAM.h"
#include "Simulator/Clock.h"
#include "Devices/Timer.h"
#include "CPU/SimpleCPU.h"

enum class RunResult
{
    Halted,
    CycleLimitReached
};

class Simulator
{
public:
    Simulator();

    Bus& getBus();
    RAM& getRAM();
    GPIO& getGPIO();
    Timer& getTimer();
    SimpleCPU& getCPU();
    Clock& getClock();

    void tick();
	RunResult run(std::uint64_t maxCycles);

    void addClockable(IClockable& device);

private:
    Bus bus;
	RAM ram;
	GPIO gpio;
	Timer timer;
	SimpleCPU cpu;
	Clock clock;
	std::vector<IClockable*> clockables;
};