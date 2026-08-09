#pragma once

#include <cstdint>

class Clock
{
public:
    Clock();

    void tick();

    std::uint64_t getCycle() const;

private:
    std::uint64_t cycle;
};