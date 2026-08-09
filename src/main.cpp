#include <cstdint>
#include <iostream>

#include "Simulator/Simulator.h"

int main()
{
    Simulator simulator;

    Bus& bus = simulator.getBus();
    GPIO& gpio = simulator.getGPIO();

    bus.write(0x0000000A, 123);

    std::cout
        << "RAM: "
        << bus.read(0x0000000A)
        << std::endl;

    bus.write(0x00001000, 0xF0);
    bus.write(0x00001001, 0xA0);

    std::cout
        << "GPIO output: 0x"
        << std::hex
        << bus.read(0x00001001)
        << std::endl;

    gpio.getPin(0).setInput(true);

    std::cout
        << "GPIO input: 0x"
        << std::hex
        << bus.read(0x00001002)
        << std::endl;

    return 0;
}