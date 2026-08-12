#include "CPU/SimpleCPU.h"

#include <stdexcept>

SimpleCPU::SimpleCPU(Bus& bus)
    : bus(bus),
      programCounter(0),
      registers{},
      halted(false),
      zeroFlag(false)
{
}

void SimpleCPU::reset()
{
    programCounter = 0;
    registers.fill(0);
    halted = false;
    zeroFlag = false;
}

void SimpleCPU::tick(std::uint64_t /*cycle*/)
{
    if (halted)
    {
        return;
    }

    std::uint32_t instruction = bus.read(programCounter);
	std::uint32_t legacyOperand = instruction & 0x00FFFFFF;

    std::uint8_t opcode =
		static_cast<std::uint8_t>((instruction >> 24) & 0xFF);

	std::uint8_t rd =
		static_cast<std::uint8_t>((instruction >> 20) & 0x0F);

	std::uint8_t rs =
		static_cast<std::uint8_t>((instruction >> 16) & 0x0F);

	std::uint16_t operand =
		static_cast<std::uint16_t>(instruction & 0xFFFF);

    ++programCounter;

    switch (opcode)
    {
        case 0x00:
			// NOP
			break;

		case 0x01:
			// Legacy LOAD R0, [address]
			registers[0] = bus.read(legacyOperand);
			break;

		case 0x02:
			// Legacy STORE R0, [address]
			bus.write(legacyOperand, registers[0]);
			break;

		case 0x03:
			// Legacy ADD R0, immediate
			registers[0] += legacyOperand;
			break;
			
		case 0x10:
			// MOV Rd, immediate

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid destination register"
				);
			}

			registers[rd] = operand;
			break;

		case 0x11:
			// LOAD Rd, [address]

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid destination register"
				);
			}

			registers[rd] = bus.read(operand);
			break;

		case 0x12:
			// STORE Rd, [address]

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid source register"
				);
			}

			bus.write(operand, registers[rd]);
			break;

		case 0x13:
			// MOV Rd, Rs

			if (rd >= registers.size() ||
				rs >= registers.size())
			{
				throw std::runtime_error(
					"Invalid register"
				);
			}

			registers[rd] = registers[rs];
			break;

		case 0x14:
			// ADD Rd, Rs

			if (rd >= registers.size() ||
				rs >= registers.size())
			{
				throw std::runtime_error(
					"Invalid register"
				);
			}

			registers[rd] += registers[rs];
			break;
			
		case 0x15:
			// SUB Rd, Rs

			if (rd >= registers.size() ||
				rs >= registers.size())
			{
				throw std::runtime_error(
					"Invalid register"
				);
			}

			registers[rd] -= registers[rs];
			break;
			
		case 0x20:
			// CMP Rd, Rs

			if (rd >= registers.size() ||
				rs >= registers.size())
			{
				throw std::runtime_error(
					"Invalid register"
				);
			}

			zeroFlag =
				registers[rd] == registers[rs];

			break;

		case 0x21:
			// JMP address
			programCounter = operand;
			break;

		case 0x22:
			// JZ address
			if (zeroFlag)
			{
				programCounter = operand;
			}
			break;

		case 0x23:
			// JNZ address
			if (!zeroFlag)
			{
				programCounter = operand;
			}
			break;
			
		case 0x24:
			// CMPI Rd, immediate

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid register"
				);
			}

			zeroFlag =
				registers[rd] == operand;

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
    return registers[0];
}

bool SimpleCPU::isHalted() const
{
    return halted;
}

std::uint32_t SimpleCPU::getRegister(std::size_t index) const
{
    if (index >= registers.size())
    {
        throw std::out_of_range(
            "Invalid CPU register index"
        );
    }

    return registers[index];
}

bool SimpleCPU::getZeroFlag() const
{
    return zeroFlag;
}