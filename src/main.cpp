#include <cstdint>
#include <iostream>

#include "Bus/Bus.h"
#include "Memory/RAM.h"

int main()
{
    RAM ram(1024);

    Bus bus;

    bus.attach(
        ram,
        0x00000000,
        0x000003FF
    );

    bus.write(10, 123);

    std::uint32_t value = bus.read(10);

    std::cout << "Bus test value: "
              << value
              << std::endl;

    return 0;
}