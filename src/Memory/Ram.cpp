#include "Memory/RAM.h"

#include <stdexcept>

RAM::RAM(std::uint32_t size)
    : memory(size, 0)
{
}

std::uint32_t RAM::read(std::uint32_t address)
{
    if (address >= memory.size())
    {
        throw std::out_of_range("RAM read out of bounds");
    }

    return memory[address];
}

void RAM::write(std::uint32_t address,
                std::uint32_t value)
{
    if (address >= memory.size())
    {
        throw std::out_of_range("RAM write out of bounds");
    }

    memory[address] = value;
}