#include "Simulator/TestClockable.h"

TestClockable::TestClockable()
    : lastCycle(0),
      tickCount(0)
{
}

void TestClockable::tick(std::uint64_t cycle)
{
    lastCycle = cycle;
    ++tickCount;
}

std::uint64_t TestClockable::getLastCycle() const
{
    return lastCycle;
}

std::uint64_t TestClockable::getTickCount() const
{
    return tickCount;
}