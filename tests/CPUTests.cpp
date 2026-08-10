#include <cassert>
#include <iostream>
#include <stdexcept>

#include "CPU/SimpleCPU.h"
#include "Bus/Bus.h"
#include "Memory/RAM.h"
#include "Devices/GPIO.h"
#include "Simulator/Simulator.h"

int main()
{
    // Reset state.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        SimpleCPU cpu(bus);

        cpu.reset();

        assert(cpu.getProgramCounter() == 0);
        assert(cpu.getRegister0() == 0);
        assert(!cpu.isHalted());
    }

    // NOP should advance the program counter.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        ram.write(0, 0x00000000);

        SimpleCPU cpu(bus);

        cpu.reset();
        cpu.tick(1);

        assert(cpu.getProgramCounter() == 1);
        assert(cpu.getRegister0() == 0);
        assert(!cpu.isHalted());
    }

    // ADD immediate.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        ram.write(0, 0x03000005);

        SimpleCPU cpu(bus);

        cpu.reset();
        cpu.tick(1);

        assert(cpu.getRegister0() == 5);
        assert(cpu.getProgramCounter() == 1);
    }

    // LOAD from memory.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        ram.write(0, 0x0100000A);
        ram.write(10, 123);

        SimpleCPU cpu(bus);

        cpu.reset();
        cpu.tick(1);

        assert(cpu.getRegister0() == 123);
    }

    // STORE to memory.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        // ADD R0, 42
        ram.write(0, 0x0300002A);

        // STORE R0, [10]
        ram.write(1, 0x0200000A);

        SimpleCPU cpu(bus);

        cpu.reset();

        cpu.tick(1);
        cpu.tick(2);

        assert(ram.read(10) == 42);
    }

    // HALT.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        ram.write(0, 0xFF000000);

        SimpleCPU cpu(bus);

        cpu.reset();
        cpu.tick(1);

        assert(cpu.isHalted());
        assert(cpu.getProgramCounter() == 1);

        // Additional ticks should do nothing.
        cpu.tick(2);

        assert(cpu.getProgramCounter() == 1);
    }

    // Invalid opcode should throw.
    {
        Bus bus;
        RAM ram(256);

        bus.attach(ram, 0x0000, 0x00FF);

        ram.write(0, 0xAA000000);

        SimpleCPU cpu(bus);

        cpu.reset();

        bool exceptionThrown = false;

        try
        {
            cpu.tick(1);
        }
        catch (const std::runtime_error&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Full program:
    //
    // LOAD R0, [10]
    // ADD  R0, 5
    // STORE R0, [11]
    // HALT
    {
        Simulator simulator;

        Bus& bus = simulator.getBus();
        SimpleCPU& cpu = simulator.getCPU();

        bus.write(0, 0x0100000A);
        bus.write(1, 0x03000005);
        bus.write(2, 0x0200000B);
        bus.write(3, 0xFF000000);

        bus.write(10, 20);
        bus.write(11, 0);

        cpu.reset();

        RunResult result = simulator.run(100);

        assert(result == RunResult::Halted);
        assert(cpu.getRegister0() == 25);
        assert(bus.read(11) == 25);
        assert(simulator.getClock().getCycle() == 4);
    }

    // Firmware-driven GPIO.
    {
        Simulator simulator;

        Bus& bus = simulator.getBus();
        SimpleCPU& cpu = simulator.getCPU();
        GPIO& gpio = simulator.getGPIO();

        // ADD R0, 0x80
        bus.write(0, 0x03000080);

        // STORE direction register.
        bus.write(1, 0x02001000);

        // STORE output register.
        bus.write(2, 0x02001001);

        // HALT
        bus.write(3, 0xFF000000);

        cpu.reset();

        RunResult result = simulator.run(100);

        assert(result == RunResult::Halted);

        assert(bus.read(0x1000) == 0x80);
        assert(bus.read(0x1001) == 0x80);
        assert(gpio.getPin(7).getOutput());
    }

    // Cycle limit should stop runaway firmware.
    {
        Simulator simulator;

        Bus& bus = simulator.getBus();
        SimpleCPU& cpu = simulator.getCPU();

        // RAM defaults to zero, so this is effectively
        // an endless stream of NOP instructions.
        cpu.reset();

        RunResult result = simulator.run(10);

        assert(result == RunResult::CycleLimitReached);
        assert(simulator.getClock().getCycle() == 10);
        assert(!cpu.isHalted());
    }

    std::cout << "CPU tests passed.\n";

    return 0;
}