#pragma once

#include <cstdint>

#include "Simulator/IClockable.h"

class TestClockable : public IClockable
{
public:
    TestClockable();

    void tick(std::uint64_t cycle) override;

    std::uint64_t getLastCycle() const;
    std::uint64_t getTickCount() const;

private:
    std::uint64_t lastCycle;
    std::uint64_t tickCount;
};