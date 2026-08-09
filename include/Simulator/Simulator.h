#pragma once

#include <cstdint>
#include <memory>

#include "Bus/Bus.h"
#include "Devices/GPIO.h"
#include "Memory/RAM.h"

class Simulator
{
public:
    Simulator();

    Bus& getBus();
    RAM& getRAM();
    GPIO& getGPIO();

private:
    Bus bus;
    RAM ram;
    GPIO gpio;
};