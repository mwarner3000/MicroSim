#pragma once

enum class PinDirection
{
    Input,
    Output
};

class Pin
{
public:
    Pin();

    double getVoltage() const;
    void setVoltage(double volts);

    PinDirection getDirection() const;
    void setDirection(PinDirection direction);

private:
    double voltage;
    PinDirection direction;
};