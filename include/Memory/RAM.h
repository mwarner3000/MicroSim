#pragma once

#include <cstdint>
#include <vector>

#include "Bus/IBusDevice.h"

class RAM : public IBusDevice
{
public:
    explicit RAM(std::uint32_t size);

    std::uint32_t read(std::uint32_t address) override;

    void write(std::uint32_t address,
               std::uint32_t value) override;

private:
    std::vector<std::uint32_t> memory;
};