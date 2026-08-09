#pragma once

#include <cstdint>

#include "Bus/IBusDevice.h"
#include "Simulator/IClockable.h"

class Timer : public IBusDevice, public IClockable
{
public:
    Timer();

    std::uint32_t read(std::uint32_t address) override;

    void write(std::uint32_t address,
               std::uint32_t value) override;

    void tick(std::uint64_t cycle) override;

private:
    std::uint16_t counter;
    std::uint16_t period;

    bool enabled;
    bool expired;
};