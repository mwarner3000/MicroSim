#pragma once

#include <cstdint>

#include "CPU/ICPU.h"
#include "Bus/Bus.h"

class SimpleCPU : public ICPU
{
public:
    explicit SimpleCPU(Bus& bus);

    void reset() override;

    void tick(std::uint64_t cycle) override;

    std::uint32_t getProgramCounter() const override;

private:
    Bus& bus;

    std::uint32_t programCounter;
};