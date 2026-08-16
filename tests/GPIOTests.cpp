#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Board/BoardConfig.h"
#include "Simulator/Simulator.h"

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

		// Pins 5, 6, and 7 are outputs.
		gpio.write(0, 0xE0);

		// Pins 5 and 7 HIGH, pin 6 LOW.
		gpio.write(1, 0xA0);

		assert(
			gpio.getPin(5).getDirection() ==
			PinDirection::Output
		);

		assert(
			gpio.getPin(6).getDirection() ==
			PinDirection::Output
		);

		assert(
			gpio.getPin(7).getDirection() ==
			PinDirection::Output
		);

		assert(gpio.getPin(5).getVoltage() == 5.0);
		assert(gpio.getPin(6).getVoltage() == 0.0);
		assert(gpio.getPin(7).getVoltage() == 5.0);
	}

    // External pin input should appear in the input register.
    {
        GPIO gpio;

        gpio.getPin(0).setVoltage(5.0);
        gpio.getPin(3).setVoltage(5.0);

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
        assert(
			gpio.getPin(7).getDirection() ==
			PinDirection::Output
		);

		assert(
			gpio.getPin(7).getVoltage() == 5.0
		);
    }

    // Full path in the opposite direction:
    // World -> Pin -> GPIO -> Bus -> CPU side.
    {
        Bus bus;
        GPIO gpio;

        bus.attach(gpio, 0x1000, 0x1003);

        gpio.getPin(2).setVoltage(5.0);

        std::uint32_t input = bus.read(0x1002);

        assert((input & 0x04) != 0);
    }
	
	// Output latch should take effect when a pin becomes an output.
	{
		GPIO gpio;

		// Store HIGH in pin 7's output latch while it is still an input.
		gpio.write(1, 0x80);

		assert(
			gpio.getPin(7).getDirection() ==
			PinDirection::Input
		);

		// Now configure pin 7 as an output.
		gpio.write(0, 0x80);

		assert(
			gpio.getPin(7).getDirection() ==
			PinDirection::Output
		);

		assert(
			gpio.getPin(7).getVoltage() == 5.0
		);
	}
	
	// GPIO pin count should be configurable.
	{
		GPIO gpio(16);

		assert(gpio.getPinCount() == 16);

		gpio.write(0, 0x8000);
		gpio.write(1, 0x8000);

		assert(
			gpio.getPin(15).getDirection() ==
			PinDirection::Output
		);

		assert(
			gpio.getPin(15).getVoltage() == 5.0
		);
	}

	// Invalid GPIO pin counts should fail.
	{
		bool zeroFailed = false;
		bool tooManyFailed = false;

		try
		{
			GPIO gpio(0);
		}
		catch (const std::invalid_argument&)
		{
			zeroFailed = true;
		}

		try
		{
			GPIO gpio(33);
		}
		catch (const std::invalid_argument&)
		{
			tooManyFailed = true;
		}

		assert(zeroFailed);
		assert(tooManyFailed);
	}

	//test user created board
	{
		BoardConfig config;
		config.gpioPins = 16;

		Simulator simulator(config);

		assert(
			simulator.getGPIO().getPinCount() == 16
		);
	}

    std::cout << "GPIO tests passed.\n";

    return 0;
}

