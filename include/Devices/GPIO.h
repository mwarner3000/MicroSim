#pragma once

#include <array>
#include <cstdint>

#include "Bus/IBusDevice.h"
#include "Devices/Pin.h"

class GPIO : public IBusDevice
{
public:
    GPIO();

    std::uint32_t read(std::uint32_t address) override;

    void write(std::uint32_t address,
               std::uint32_t value) override;

    Pin& getPin(std::size_t index);

private:
    std::uint8_t directionRegister;
    std::uint8_t outputRegister;

    std::array<Pin, 8> pins;
};