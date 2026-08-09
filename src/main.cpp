#include <iostream>

#include "Bus/Bus.h"
#include "Memory/RAM.h"

int main()
{
    RAM ram1(1024);
    RAM ram2(256);

    Bus bus;

    bus.attach(
        ram1,
        0x00000000,
        0x000003FF
    );

    bus.attach(
        ram2,
        0x00000400,
        0x000004FF
    );
	
	bus.write(0x0400, 456);

std::cout << bus.read(0x0400) << std::endl;

    return 0;
}