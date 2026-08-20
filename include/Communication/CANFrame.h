#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

struct CANFrame
{
    std::uint32_t id = 0;
    std::array<std::uint8_t, 8> data{};
    std::size_t length = 0;

    void validate() const
    {
        if (length > data.size())
        {
            throw std::out_of_range(
                "CAN frame payload exceeds 8 bytes"
            );
        }
    }
};