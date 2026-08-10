#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Memory/RAM.h"

int main()
{
    RAM ram(1024);

    // Newly created RAM should contain zero.
    assert(ram.read(0) == 0);
    assert(ram.read(1023) == 0);

    // Basic read/write.
    ram.write(10, 123);
    assert(ram.read(10) == 123);

    // Different addresses should remain independent.
    ram.write(20, 456);
    assert(ram.read(10) == 123);
    assert(ram.read(20) == 456);

    // Out-of-range read should throw.
    bool readException = false;

    try
    {
        ram.read(1024);
    }
    catch (const std::out_of_range&)
    {
        readException = true;
    }

    assert(readException);

    // Out-of-range write should throw.
    bool writeException = false;

    try
    {
        ram.write(1024, 1);
    }
    catch (const std::out_of_range&)
    {
        writeException = true;
    }

    assert(writeException);

    std::cout << "RAM tests passed.\n";

    return 0;
}
