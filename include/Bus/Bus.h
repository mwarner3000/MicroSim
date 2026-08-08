#pragma once

#include <cstdint>
#include <vector>

#include "Bus/IBusDevice.h"

class Bus
{
public:
    void attach(IBusDevice& device,
                std::uint32_t startAddress,
                std::uint32_t endAddress);

    std::uint32_t read(std::uint32_t address);

    void write(std::uint32_t address,
               std::uint32_t value);

private:
    struct Mapping
    {
        IBusDevice* device;
        std::uint32_t startAddress;
        std::uint32_t endAddress;
    };

    std::vector<Mapping> mappings;
};