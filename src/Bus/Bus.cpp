#include "Bus/Bus.h"

#include <stdexcept>

void Bus::attach(IBusDevice& device,
                 std::uint32_t startAddress,
                 std::uint32_t endAddress)
{
    if (startAddress > endAddress)
    {
        throw std::invalid_argument(
            "Bus mapping has an invalid address range"
        );
    }

    mappings.push_back(
        {
            &device,
            startAddress,
            endAddress
        }
    );
}

std::uint32_t Bus::read(std::uint32_t address)
{
    for (const Mapping& mapping : mappings)
    {
        if (address >= mapping.startAddress &&
            address <= mapping.endAddress)
        {
            std::uint32_t deviceAddress =
                address - mapping.startAddress;

            return mapping.device->read(deviceAddress);
        }
    }

    throw std::out_of_range(
        "Bus read from unmapped address"
    );
}

void Bus::write(std::uint32_t address,
                std::uint32_t value)
{
    for (const Mapping& mapping : mappings)
    {
        if (address >= mapping.startAddress &&
            address <= mapping.endAddress)
        {
            std::uint32_t deviceAddress =
                address - mapping.startAddress;

            mapping.device->write(deviceAddress, value);
            return;
        }
    }

    throw std::out_of_range(
        "Bus write to unmapped address"
    );
}