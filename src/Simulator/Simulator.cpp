#include "Simulator/Simulator.h"

Simulator::Simulator()
    : ram(1024),
      gpio()
{
    bus.attach(
        ram,
        0x00000000,
        0x000003FF
    );

    bus.attach(
        gpio,
        0x00001000,
        0x00001003
    );
}

Bus& Simulator::getBus()
{
    return bus;
}

RAM& Simulator::getRAM()
{
    return ram;
}

GPIO& Simulator::getGPIO()
{
    return gpio;
}