#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Bus/Bus.h"
#include "Devices/GPIO.h"

int main()
{
    // Basic register access.
    {
        GPIO gpio;

        gpio.write(0, 0xF0);
        gpio.write(1, 0xA0);

        assert(gpio.read(0) == 0xF0);
        assert(gpio.read(1) == 0xA0);
    }

    // Output register should update physical pin outputs.
    {
        GPIO gpio;

        gpio.write(1, 0xA0);

        assert(gpio.getPin(5).getOutput());
        assert(!gpio.getPin(6).getOutput());
        assert(gpio.getPin(7).getOutput());
    }

    // External pin input should appear in the input register.
    {
        GPIO gpio;

        gpio.getPin(0).setInput(true);
        gpio.getPin(3).setInput(true);

        std::uint32_t value = gpio.read(2);

        assert((value & 0x01) != 0);
        assert((value & 0x08) != 0);
        assert((value & 0x02) == 0);
    }

    // Input register should be read-only from the CPU side.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.write(2, 0xFF);
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid register reads should fail.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.read(4);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid register writes should fail.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.write(4, 0);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid pin indexes should fail.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.getPin(8);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Full path through the bus:
    // CPU side -> Bus -> GPIO -> Pin.
    {
        Bus bus;
        GPIO gpio;

        bus.attach(gpio, 0x1000, 0x1003);

        bus.write(0x1000, 0x80);
        bus.write(0x1001, 0x80);

        assert(bus.read(0x1000) == 0x80);
        assert(bus.read(0x1001) == 0x80);
        assert(gpio.getPin(7).getOutput());
    }

    // Full path in the opposite direction:
    // World -> Pin -> GPIO -> Bus -> CPU side.
    {
        Bus bus;
        GPIO gpio;

        bus.attach(gpio, 0x1000, 0x1003);

        gpio.getPin(2).setInput(true);

        std::uint32_t input = bus.read(0x1002);

        assert((input & 0x04) != 0);
    }

    std::cout << "GPIO tests passed.\n";

    return 0;
}