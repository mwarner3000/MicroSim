#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Board/BoardConfig.h"
#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Simulator/Simulator.h"

int main()
{
    // Basic pin selection.
    {
        GPIO gpio;

        gpio.write(0, 3);

        assert(gpio.read(0) == 3);
    }

    // Configure an output pin and drive it HIGH.
    {
        GPIO gpio;

        gpio.write(0, 5); // select pin 5
        gpio.write(1, 1); // output
        gpio.write(2, 1); // HIGH

        assert(
            gpio.getPin(5).getDirection() ==
            PinDirection::Output
        );

        assert(
            gpio.getPin(5)
				.getEffectiveVoltage(5.0)
				.value() == 5.0
        );

        assert(gpio.read(1) == 1);
        assert(gpio.read(2) == 1);
    }

    // Configure an output pin and drive it LOW.
    {
        GPIO gpio;

        gpio.write(0, 6);
        gpio.write(1, 1);
        gpio.write(2, 0);

        assert(
            gpio.getPin(6).getDirection() ==
            PinDirection::Output
        );

        assert(
				gpio.getPin(6)
					.getEffectiveVoltage(5.0)
					.value() == 0.0
        );

        assert(gpio.read(2) == 0);
    }

    // External voltage on an input pin should be readable.
    {
        GPIO gpio;

        gpio.getPin(3).setExternalVoltage(5.0);

        gpio.write(0, 3);

        assert(
            gpio.getPin(3).getDirection() ==
            PinDirection::Input
        );

        assert(gpio.read(3) == 1);
    }

    // Input below threshold should read LOW.
    {
        GPIO gpio;

        gpio.getPin(3).setExternalVoltage(2.0);

        gpio.write(0, 3);

        assert(gpio.read(3) == 0);
    }

    // Input above threshold should read HIGH.
    {
        GPIO gpio;

        gpio.getPin(3).setExternalVoltage(3.0);

        gpio.write(0, 3);

        assert(gpio.read(3) == 1);
    }

    // INPUT register is read-only.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.write(3, 1);
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Writing OUTPUT while pin is configured as input
	// should update the output latch without driving the pin.
	{
		GPIO gpio;

		gpio.write(0, 2); // select pin 2

		gpio.write(2, 1); // write HIGH to output latch

		assert(
			gpio.getPin(2).getDirection() ==
			PinDirection::Input
		);

		assert(
			gpio.getPin(2).getOutputLatch() == true
		);

		assert(
			gpio.read(2) == 1
		);
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

    // Invalid pin selection should fail.
    {
        GPIO gpio;

        bool exceptionThrown = false;

        try
        {
            gpio.write(0, 8);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid direct pin access should fail.
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

    // CPU side -> Bus -> GPIO -> Pin.
    {
        Bus bus;
        GPIO gpio;

        bus.attach(gpio, 0x1000, 0x1003);

        bus.write(0x1000, 7); // select pin 7
        bus.write(0x1001, 1); // output
        bus.write(0x1002, 1); // HIGH

        assert(
            gpio.getPin(7).getDirection() ==
            PinDirection::Output
        );

        assert(
            gpio.getPin(7)
				.getEffectiveVoltage(5.0)
				.value() == 5.0
        );
    }

    // World -> Pin -> GPIO -> Bus -> CPU side.
    {
        Bus bus;
        GPIO gpio;

        bus.attach(gpio, 0x1000, 0x1003);

        gpio.getPin(2).setExternalVoltage(5.0);

        bus.write(0x1000, 2); // select pin 2

        assert(
            bus.read(0x1003) == 1
        );
    }

    // GPIO pin count should be configurable beyond 32.
    {
        GPIO gpio(100);

        assert(gpio.getPinCount() == 100);

        gpio.write(0, 73);
        gpio.write(1, 1);
        gpio.write(2, 1);

        assert(
            gpio.getPin(73).getDirection() ==
            PinDirection::Output
        );

        assert(
            gpio.getPin(73)
				.getEffectiveVoltage(5.0)
				.value() == 5.0
        );
    }

    // Zero pins should be rejected.
    {
        bool exceptionThrown = false;

        try
        {
            GPIO gpio(0);
        }
        catch (const std::invalid_argument&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // BoardConfig should determine GPIO pin count.
    {
        BoardConfig config;
        config.gpioPins = 100;

        Simulator simulator(config);

        assert(
            simulator.getGPIO().getPinCount() == 100
        );
    }

    // Custom electrical configuration.
    {
        GPIO gpio(8, 3.3, 1.65);

        gpio.write(0, 0);
        gpio.write(1, 1);
        gpio.write(2, 1);

        assert(
            gpio.getPin(0)
				.getEffectiveVoltage(3.3)
				.value() == 3.3
        );

        gpio.getPin(1).setExternalVoltage(2.0);

        gpio.write(0, 1);

        assert(gpio.read(3) == 1);
    }
	
	// Output latch should be preserved while the pin is an input
	// and drive the pin when switched to output.
	{
		GPIO gpio;

		gpio.write(0, 2); // select pin 2

		// Pin starts as input.
		assert(
			gpio.getPin(2).getDirection() ==
			PinDirection::Input
		);

		// Preload the output latch HIGH while still an input.
		gpio.write(2, 1);

		assert(
			gpio.getPin(2).getOutputLatch() == true
		);

		// Switch the pin to output.
		gpio.write(1, 1);

		assert(
			gpio.getPin(2).getDirection() ==
			PinDirection::Output
		);

		// The previously stored HIGH should now drive the pin.
		auto voltage =
			gpio.getPin(2).getEffectiveVoltage(5.0);

		assert(voltage.has_value());
		assert(voltage.value() == 5.0);

		// Switch back to input.
		gpio.write(1, 0);

		// The latch should still remember HIGH.
		assert(
			gpio.getPin(2).getOutputLatch() == true
		);
	}
	
	// Floating digital input should read LOW.
	{
		GPIO gpio;

		gpio.write(0, 2); // select pin 2

		assert(
			gpio.getPin(2).getDirection() ==
			PinDirection::Input
		);

		assert(
			!gpio.getPin(2).hasExternalVoltage()
		);

		assert(gpio.read(3) == 0);
	}
	
	// INPUT register should reflect the level driven by an output pin.
	{
		GPIO gpio;

		gpio.write(0, 2); // select pin 2
		gpio.write(1, 1); // configure as output

		gpio.write(2, 1); // drive HIGH
		assert(gpio.read(3) == 1);

		gpio.write(2, 0); // drive LOW
		assert(gpio.read(3) == 0);
	}

    std::cout << "GPIO tests passed.\n";

    return 0;
}