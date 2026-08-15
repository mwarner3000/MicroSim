#pragma once

#include <cstdint>
#include <vector>

#include "Board/BoardConfig.h"
#include "Bus/Bus.h"
#include "CPU/SimpleCPU.h"
#include "Devices/GPIO.h"
#include "Devices/Timer.h"
#include "Memory/RAM.h"
#include "Simulator/Clock.h"
#include "Simulator/IClockable.h"

enum class RunResult
{
    Halted,
    CycleLimitReached
};

class Simulator
{
public:
    Simulator();
    explicit Simulator(const BoardConfig& config);

    Bus& getBus();
    RAM& getRAM();
    GPIO& getGPIO();
    Timer& getTimer();
    SimpleCPU& getCPU();
    Clock& getClock();

    const BoardConfig& getConfig() const;

    void tick();
    RunResult run(std::uint64_t maxCycles);

    void addClockable(IClockable& device);

private:
    BoardConfig config;

    Bus bus;
    RAM ram;
    GPIO gpio;
    Timer timer;
    SimpleCPU cpu;
    Clock clock;

    std::vector<IClockable*> clockables;
};