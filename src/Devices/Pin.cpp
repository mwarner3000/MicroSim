#include "Devices/Pin.h"

Pin::Pin()
    : inputState(false),
      outputState(false)
{
}

void Pin::setInput(bool state)
{
    inputState = state;
}

bool Pin::getInput() const
{
    return inputState;
}

void Pin::setOutput(bool state)
{
    outputState = state;
}

bool Pin::getOutput() const
{
    return outputState;
}