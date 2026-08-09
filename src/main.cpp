#include <cstdint>
#include <iostream>

#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Memory/RAM.h"

int main()
{
    RAM ram(1024);
    GPIO gpio;

    Bus bus;

    bus.attach(
        ram,
        0x00000000,
        0x000003FF
    );

    bus.attach(
        gpio,
        0x00001000,
        0x00001003
    );

    // CPU configures GPIO pins 4-7 as outputs.
    bus.write(0x00001000, 0xF0);

    // CPU drives pins 5 and 7 HIGH.
    bus.write(0x00001001, 0xA0);

    std::cout
        << "GPIO output register: 0x"
        << std::hex
        << bus.read(0x00001001)
        << std::endl;

    std::cout
        << "Pin 5 output: "
        << gpio.getPin(5).getOutput()
        << std::endl;

    // Simulated external device drives pin 0 HIGH.
    gpio.getPin(0).setInput(true);

    std::cout
        << "GPIO input register: 0x"
        << std::hex
        << bus.read(0x00001002)
        << std::endl;

    return 0;
}