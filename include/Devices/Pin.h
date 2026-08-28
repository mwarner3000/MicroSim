#pragma once

#include <optional>

enum class PinDirection
{
    Input,
    Output
};

class Pin
{
public:
    Pin();

    PinDirection getDirection() const;
    void setDirection(PinDirection direction);
	
	bool getOutputLatch() const;
	void setOutputLatch(bool value);

	bool hasExternalVoltage() const;
	double getExternalVoltage() const;
	void setExternalVoltage(double voltage);
	void clearExternalVoltage();
	std::optional<double> getEffectiveVoltage(
		double logicVoltage
	) const;

private:
    PinDirection direction;
	bool outputLatch;
	std::optional<double> externalVoltage;
};