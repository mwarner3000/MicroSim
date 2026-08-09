#include <iostream>

#include "Simulator/Simulator.h"

int main()
{
    Simulator simulator;

    std::cout
        << "Program counter: "
        << simulator.getCPU().getProgramCounter()
        << std::endl;

    simulator.getCPU().reset();

    std::cout
        << "After reset: "
        << simulator.getCPU().getProgramCounter()
        << std::endl;

    simulator.tick();

    std::cout
        << "After one tick: "
        << simulator.getCPU().getProgramCounter()
        << std::endl;

    return 0;
}