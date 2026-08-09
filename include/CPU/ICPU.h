#pragma once

#include <cstdint>

#include "Simulator/IClockable.h"

class ICPU : public IClockable
{
public:
    virtual ~ICPU() = default;

    virtual void reset() = 0;

    virtual std::uint32_t getProgramCounter() const = 0;
};