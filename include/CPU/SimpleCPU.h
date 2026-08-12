#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "CPU/ICPU.h"
#include "Bus/Bus.h"

class SimpleCPU : public ICPU
{
public:
    static constexpr std::size_t RegisterCount = 8;

    explicit SimpleCPU(Bus& bus);

    void reset() override;
    void tick(std::uint64_t cycle) override;

    std::uint32_t getProgramCounter() const override;

    std::uint32_t getRegister(std::size_t index) const;

    // Keep this temporarily so our existing tests still work.
    std::uint32_t getRegister0() const;

    bool isHalted() const;
	bool getZeroFlag() const;

private:
    Bus& bus;

    std::uint32_t programCounter;

    std::array<std::uint32_t, RegisterCount> registers;

    bool halted;
	bool zeroFlag;
};