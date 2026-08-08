#pragma once

#include <cstdint>

class IBusDevice
{
public:
    virtual ~IBusDevice() = default;

    virtual std::uint32_t read(std::uint32_t address) = 0;

    virtual void write(std::uint32_t address,
                       std::uint32_t value) = 0;
};