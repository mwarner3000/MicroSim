#include "Simulator/Simulator.h"

Simulator::Simulator()
    : ram(1024),
      gpio(),
      timer(),
      cpu(bus)
{
    bus.attach(
        ram,
        0x00000000,
        0x000003FF
    );

    bus.attach(
        gpio,
        0x00001000,
        0x00001003
    );

    bus.attach(
        timer,
        0x00002000,
        0x00002003
    );

	clockables.push_back(&timer);
	clockables.push_back(&cpu);
}

Bus& Simulator::getBus()
{
    return bus;
}

RAM& Simulator::getRAM()
{
    return ram;
}

GPIO& Simulator::getGPIO()
{
    return gpio;
}

Clock& Simulator::getClock()
{
    return clock;
}

void Simulator::tick()
{
    clock.tick();

    for (IClockable* device : clockables)
    {
        device->tick(clock.getCycle());
    }
}

void Simulator::addClockable(IClockable& device)
{
    clockables.push_back(&device);
}

Timer& Simulator::getTimer()
{
    return timer;
}

SimpleCPU& Simulator::getCPU()
{
    return cpu;
}

RunResult Simulator::run(std::uint64_t maxCycles)
{
    std::uint64_t startCycle = clock.getCycle();

    while (!cpu.isHalted())
    {
        if (clock.getCycle() - startCycle >= maxCycles)
        {
            return RunResult::CycleLimitReached;
        }

        tick();
    }

    return RunResult::Halted;
}