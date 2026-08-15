#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Bus/IBusDevice.h"
#include "Devices/Pin.h"

class GPIO : public IBusDevice
{
public:
    GPIO(
        double logicVoltage = 5.0,
        double digitalHighThreshold = 2.5
    );

    std::uint32_t read(std::uint32_t address) override;

    void write(
        std::uint32_t address,
        std::uint32_t value
    ) override;

    Pin& getPin(std::size_t index);

private:
    std::uint8_t directionRegister;
    std::uint8_t outputRegister;

    double logicVoltage;
    double digitalHighThreshold;

    std::array<Pin, 8> pins;
};