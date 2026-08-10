#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Bus/Bus.h"
#include "Devices/Timer.h"
#include "Simulator/Simulator.h"

int main()
{
    // Initial state.
    {
        Timer timer;

        assert(timer.read(0) == 0);
        assert(timer.read(1) == 0);
        assert(timer.read(2) == 0);
        assert(timer.read(3) == 0);
    }

    // Register reads and writes.
    {
        Timer timer;

        timer.write(0, 3);
        timer.write(1, 10);
        timer.write(2, 1);

        assert(timer.read(0) == 3);
        assert(timer.read(1) == 10);
        assert(timer.read(2) == 1);
    }

    // Disabled timer should not advance.
    {
        Timer timer;

        timer.write(1, 5);

        timer.tick(1);
        timer.tick(2);
        timer.tick(3);

        assert(timer.read(0) == 0);
        assert(timer.read(3) == 0);
    }

    // Enabled timer should advance.
    {
        Timer timer;

        timer.write(1, 5);
        timer.write(2, 1);

        timer.tick(1);
        timer.tick(2);
        timer.tick(3);

        assert(timer.read(0) == 3);
    }

    // Timer should expire and reset.
    {
        Timer timer;

        timer.write(1, 3);
        timer.write(2, 1);

        timer.tick(1);
        timer.tick(2);
        timer.tick(3);
        timer.tick(4);

        assert(timer.read(0) == 0);
        assert(timer.read(3) == 1);
    }

    // Writing 1 to status should clear expiration.
    {
        Timer timer;

        timer.write(1, 1);
        timer.write(2, 1);

        timer.tick(1);
        timer.tick(2);

        assert(timer.read(3) == 1);

        timer.write(3, 1);

        assert(timer.read(3) == 0);
    }

    // Disabling the timer should clear expiration.
    {
        Timer timer;

        timer.write(1, 1);
        timer.write(2, 1);

        timer.tick(1);
        timer.tick(2);

        assert(timer.read(3) == 1);

        timer.write(2, 0);

        assert(timer.read(2) == 0);
        assert(timer.read(3) == 0);
    }

    // Invalid register reads should fail.
    {
        Timer timer;

        bool exceptionThrown = false;

        try
        {
            timer.read(4);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Invalid register writes should fail.
    {
        Timer timer;

        bool exceptionThrown = false;

        try
        {
            timer.write(4, 0);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }

    // Bus access should reach the timer correctly.
    {
        Bus bus;
        Timer timer;

        bus.attach(timer, 0x2000, 0x2003);

        bus.write(0x2001, 5);
        bus.write(0x2002, 1);

        assert(bus.read(0x2001) == 5);
        assert(bus.read(0x2002) == 1);
    }

    // Simulator ticks should advance the timer exactly once per cycle.
    {
        Simulator simulator;

        Bus& bus = simulator.getBus();

        bus.write(0x2001, 10);
        bus.write(0x2002, 1);

        simulator.tick();
        simulator.tick();
        simulator.tick();

        assert(simulator.getClock().getCycle() == 3);
        assert(bus.read(0x2000) == 3);
    }

    std::cout << "Timer tests passed.\n";

    return 0;
}