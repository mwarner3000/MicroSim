#include "CPU/SimpleCPU.h"
#include "CPU/SimpleCPU/SimpleISA.h"

#include <stdexcept>

SimpleCPU::SimpleCPU(
    Bus& bus,
    InterruptController& interruptController
)
    : bus(bus),
      interruptController(interruptController),
      programCounter(0),
      registers{},
      halted(false),
      zeroFlag(false),
	  interruptReturnAddress(0),
	  servicingInterrupt(false)
{
}

void SimpleCPU::reset()
{
    programCounter = 0;
    registers.fill(0);
    halted = false;
    zeroFlag = false;
	interruptReturnAddress = 0;
	servicingInterrupt = false;
}

void SimpleCPU::tick(std::uint64_t /*cycle*/)
{
	if (!servicingInterrupt &&
		interruptController.hasPending())
	{
		const std::size_t interruptNumber =
			interruptController.getNextPending();

		interruptReturnAddress = programCounter;

		const std::uint32_t vectorAddress =
			InterruptVectorBase +
			static_cast<std::uint32_t>(
				interruptNumber
			);

		programCounter =
			bus.read(vectorAddress);

		interruptController.clear(
			interruptNumber
		);

		servicingInterrupt = true;
		halted = false;
	}
	
    if (halted)
    {
        return;
    }
	

    std::uint32_t rawInstruction =
        bus.read(programCounter);

    SimpleISA::Instruction instruction =
        SimpleISA::decode(rawInstruction);

    std::uint32_t legacyOperand =
        rawInstruction & 0x00FFFFFF;

    const std::uint8_t rd = instruction.rd;
    const std::uint8_t rs = instruction.rs;
    const std::uint16_t operand = instruction.operand;

    ++programCounter;

    switch (instruction.opcode)
	{
        case SimpleISA::Opcode::NOP:
			break;

		case SimpleISA::Opcode::LegacyLOAD:
			registers[0] = bus.read(legacyOperand);
			break;

		case SimpleISA::Opcode::LegacySTORE:
			bus.write(legacyOperand, registers[0]);
			break;

		case SimpleISA::Opcode::LegacyADD:
			registers[0] += legacyOperand;
			break;
			
		case SimpleISA::Opcode::MOVI:
			// MOV Rd, immediate

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid destination register"
				);
			}

			registers[rd] = operand;
			break;

		case SimpleISA::Opcode::LOAD:
			// LOAD Rd, [address]

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid destination register"
				);
			}

			registers[rd] = bus.read(operand);
			break;

		case SimpleISA::Opcode::STORE:
			// STORE Rd, [address]

			if (rd >= registers.size())
			{
				throw std::runtime_error(
					"Invalid source register"
				);
			}

			bus.write(operand, registers[rd]);
			break;

		case SimpleISA::Opcode::MOV:	
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

		case SimpleISA::Opcode::ADD:
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
			
		case SimpleISA::Opcode::SUB:
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
			
		case SimpleISA::Opcode::CMP:
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

		case SimpleISA::Opcode::JMP:
			// JMP address
			programCounter = operand;
			break;

		case SimpleISA::Opcode::JZ:
			// JZ address
			if (zeroFlag)
			{
				programCounter = operand;
			}
			break;

		case SimpleISA::Opcode::JNZ:
			// JNZ address
			if (!zeroFlag)
			{
				programCounter = operand;
			}
			break;
			
		case SimpleISA::Opcode::CMPI:
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
			
		case SimpleISA::Opcode::RETI:
			if (!servicingInterrupt)
			{
				throw std::runtime_error(
					"RETI executed outside interrupt handler"
				);
			}

			programCounter = interruptReturnAddress;
			servicingInterrupt = false;
			break;

        case SimpleISA::Opcode::HALT:
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