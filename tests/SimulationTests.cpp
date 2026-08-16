#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Simulation/Simulation.h"
#include "CPU/SimpleCPU/SimpleISA.h"

int main()
{
    // Node creation.
    {
        Simulation simulation;

        assert(simulation.getNodeCount() == 0);

        simulation.createNode();
        simulation.createNode();

        assert(simulation.getNodeCount() == 2);
    }

    // Nodes should have independent memory.
    {
        Simulation simulation;

        Simulator& nodeA = simulation.createNode();
        Simulator& nodeB = simulation.createNode();

        nodeA.getBus().write(100, 111);
        nodeB.getBus().write(100, 222);

        assert(nodeA.getBus().read(100) == 111);
        assert(nodeB.getBus().read(100) == 222);
    }

    // Nodes should have independent CPUs.
    {
        Simulation simulation;

        Simulator& nodeA = simulation.createNode();
        Simulator& nodeB = simulation.createNode();

        nodeA.getBus().write(
            0,
            SimpleISA::encode(
                SimpleISA::Opcode::MOVI,
                1,
                0,
                10
            )
        );

        nodeB.getBus().write(
            0,
            SimpleISA::encode(
                SimpleISA::Opcode::MOVI,
                1,
                0,
                20
            )
        );

        simulation.tick();

        assert(nodeA.getCPU().getRegister(1) == 10);
        assert(nodeB.getCPU().getRegister(1) == 20);
    }

    // One global simulation tick should advance every node once.
    {
        Simulation simulation;

        Simulator& nodeA = simulation.createNode();
        Simulator& nodeB = simulation.createNode();

        simulation.tick();
        simulation.tick();
        simulation.tick();

        assert(nodeA.getClock().getCycle() == 3);
        assert(nodeB.getClock().getCycle() == 3);
    }

    // Invalid node access.
    {
        Simulation simulation;

        bool exceptionThrown = false;

        try
        {
            simulation.getNode(0);
        }
        catch (const std::out_of_range&)
        {
            exceptionThrown = true;
        }

        assert(exceptionThrown);
    }
	
	//boardconfig test
	{
		BoardConfig config;

		config.clockHz = 32'000'000;
		config.ramWords = 256;

		Simulator simulator(config);

		assert(
			simulator.getConfig().clockHz ==
			32'000'000
		);

		assert(
			simulator.getConfig().ramWords ==
			256
		);
	}
	
	//verify RAM changes
	{
		BoardConfig smallConfig;
		smallConfig.ramWords = 256;

		BoardConfig largeConfig;
		largeConfig.ramWords = 2048;

		Simulator smallBoard(smallConfig);
		Simulator largeBoard(largeConfig);

		// Last valid address on small board.
		smallBoard.getBus().write(255, 123);

		assert(
			smallBoard.getBus().read(255) == 123
		);

		// Address 256 should not exist on the small board.
		bool smallBoardRejected = false;

		try
		{
			smallBoard.getBus().write(256, 123);
		}
		catch (const std::out_of_range&)
		{
			smallBoardRejected = true;
		}

		assert(smallBoardRejected);

		// But address 256 is valid on the larger board.
		largeBoard.getBus().write(256, 456);

		assert(
			largeBoard.getBus().read(256) == 456
		);
	}
	
	// Board should support a custom memory map.
	{
		BoardConfig config;

		config.ramBase   = 0x00000000;
		config.gpioBase  = 0x00010000;
		config.timerBase = 0x00020000;

		Simulator simulator(config);

		Bus& bus = simulator.getBus();

		// RAM
		bus.write(config.ramBase, 123);
		assert(bus.read(config.ramBase) == 123);

		// GPIO direction register
		bus.write(config.gpioBase, 0x01);
		assert(bus.read(config.gpioBase) == 0x01);

		// Timer period register
		bus.write(config.timerBase + 1, 100);
		assert(bus.read(config.timerBase + 1) == 100);
	}
	
	// Overlapping memory regions should be rejected.
	{
		BoardConfig config;

		config.ramWords = 1024;

		// Deliberately put GPIO inside RAM.
		config.gpioBase = 0x00000100;

		bool exceptionThrown = false;

		try
		{
			Simulator simulator(config);
		}
		catch (const std::invalid_argument&)
		{
			exceptionThrown = true;
		}

		assert(exceptionThrown);
	}
	
    std::cout << "Simulation tests passed.\n";

    return 0;
}

