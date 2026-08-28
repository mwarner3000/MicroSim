#include <iostream>

#include "Simulator/Simulator.h"

int main()
{
    Simulator simulator;

    Bus& bus = simulator.getBus();
    SimpleCPU& cpu = simulator.getCPU();
    GPIO& gpio = simulator.getGPIO();

    // Program:
    //
    // 0: ADD   R0, 0x80
    // 1: STORE R0, [0x1000]   ; direction register
    // 2: STORE R0, [0x1001]   ; output register
    // 3: HALT
    //
    // This configures GPIO pin 7 as an output
    // and then drives it HIGH.

    bus.write(0, 0x03000080);
    bus.write(1, 0x02001000);
    bus.write(2, 0x02001001);
    bus.write(3, 0xFF000000);

    cpu.reset();

	simulator.run(1000);

    std::cout
        << "R0: 0x"
        << std::hex
        << std::endl;

    std::cout
        << "GPIO direction: 0x"
        << bus.read(0x1000)
        << std::endl;

    std::cout
        << "GPIO output: 0x"
        << bus.read(0x1001)
        << std::endl;

	std::cout
		<< "Pin 7 voltage: "
		<< " V"
		<< std::endl;

    std::cout
        << "Cycles: "
        << std::dec
        << simulator.getClock().getCycle()
        << std::endl;

    return 0;
}