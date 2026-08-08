#include <iostream>

#include "Memory/RAM.h"

int main()
{
    RAM ram(1024);

    ram.write(10, 123);

    std::uint32_t value = ram.read(10);

    std::cout << "RAM test value: " << value << std::endl;

    return 0;
}