#include "CPU/SimpleCPU.h"

#include <stdexcept>

SimpleCPU::SimpleCPU(Bus& bus)
    : bus(bus),
      programCounter(0),
      register0(0),
      halted(false)
{
}

void SimpleCPU::reset()
{
    programCounter = 0;
    register0 = 0;
    halted = false;
}

void SimpleCPU::tick(std::uint64_t /*cycle*/)
{
    if (halted)
    {
        return;
    }

    std::uint32_t instruction = bus.read(programCounter);

    std::uint8_t opcode =
        static_cast<std::uint8_t>((instruction >> 24) & 0xFF);

    std::uint32_t operand =
        instruction & 0x00FFFFFF;

    ++programCounter;

    switch (opcode)
    {
        case 0x00:
            // NOP
            break;

        case 0x01:
            // LOAD R0, [address]
            register0 = bus.read(operand);
            break;

        case 0x02:
            // STORE R0, [address]
            bus.write(operand, register0);
            break;

        case 0x03:
            // ADD R0, immediate
            register0 += operand;
            break;

        case 0xFF:
            // HALT
            halted = true;
            break;

        default:
            throw std::runtime_error(
                "SimpleCPU encountered invalid opcode"
            );
    }
}

std::uint32_t SimpleCPU::getProgramCounter() const
{
    return programCounter;
}

std::uint32_t SimpleCPU::getRegister0() const
{
    return register0;
}

bool SimpleCPU::isHalted() const
{
    return halted;
}