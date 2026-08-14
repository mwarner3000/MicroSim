#pragma once

#include <cstdint>

namespace SimpleISA
{
    enum class Opcode : std::uint8_t
    {
        NOP         = 0x00,
        LegacyLOAD  = 0x01,
        LegacySTORE = 0x02,
        LegacyADD   = 0x03,

        MOVI  = 0x10,
        LOAD  = 0x11,
        STORE = 0x12,
        MOV   = 0x13,
        ADD   = 0x14,
        SUB   = 0x15,

        CMP  = 0x20,
        JMP  = 0x21,
        JZ   = 0x22,
        JNZ  = 0x23,
        CMPI = 0x24,

        HALT = 0xFF
    };

    constexpr std::uint32_t OpcodeMask  = 0xFF000000;
    constexpr std::uint32_t RdMask      = 0x00F00000;
    constexpr std::uint32_t RsMask      = 0x000F0000;
    constexpr std::uint32_t OperandMask = 0x0000FFFF;

    constexpr unsigned OpcodeShift = 24;
    constexpr unsigned RdShift = 20;
    constexpr unsigned RsShift = 16;

    struct Instruction
    {
        Opcode opcode;
        std::uint8_t rd;
        std::uint8_t rs;
        std::uint16_t operand;
    };

    constexpr Instruction decode(std::uint32_t word)
    {
        return
        {
            static_cast<Opcode>(
                (word & OpcodeMask) >> OpcodeShift
            ),

            static_cast<std::uint8_t>(
                (word & RdMask) >> RdShift
            ),

            static_cast<std::uint8_t>(
                (word & RsMask) >> RsShift
            ),

            static_cast<std::uint16_t>(
                word & OperandMask
            )
        };
    }

    constexpr std::uint32_t encode(
        Opcode opcode,
        std::uint8_t rd = 0,
        std::uint8_t rs = 0,
        std::uint16_t operand = 0)
    {
        return
            (static_cast<std::uint32_t>(opcode)
                << OpcodeShift)
            |
            (static_cast<std::uint32_t>(rd)
                << RdShift)
            |
            (static_cast<std::uint32_t>(rs)
                << RsShift)
            |
            operand;
    }
}