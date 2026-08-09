#include "Simulator/Clock.h"

Clock::Clock()
    : cycle(0)
{
}

void Clock::tick()
{
    ++cycle;
}

std::uint64_t Clock::getCycle() const
{
    return cycle;
}