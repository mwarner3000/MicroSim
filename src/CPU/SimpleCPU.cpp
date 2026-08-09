#include "CPU/SimpleCPU.h"

SimpleCPU::SimpleCPU(Bus& bus)
    : bus(bus),
      programCounter(0)
{
}

void SimpleCPU::reset()
{
    programCounter = 0;
}

void SimpleCPU::tick(std::uint64_t /*cycle*/)
{
    // CPU execution will be implemented next.
}

std::uint32_t SimpleCPU::getProgramCounter() const
{
    return programCounter;
}