#pragma once

#include <cstdint>

class IClockable
{
public:
    virtual ~IClockable() = default;

    virtual void tick(std::uint64_t cycle) = 0;
};