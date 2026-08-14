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

    std::cout << "Simulation tests passed.\n";

    return 0;
}