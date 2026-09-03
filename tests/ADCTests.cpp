#include <cassert>
#include <cstdint>

#include "Devices/ADC.h"
#include "Devices/ADCConfig.h"
#include "Devices/GPIO.h"
#include "Interrupts/InterruptController.h"

int main()
{
    GPIO gpio(8, 5.0, 2.5);
    InterruptController interruptController;

    ADCConfig config;
	
	//ADC 1
    ADC adc(
        config,
        gpio,
        interruptController
    );
	
	gpio.getPin(0).setExternalVoltage(2.5);
	
	adc.write(0, 0);
	
	adc.write(1, 1);
	
	assert((adc.read(2) & (1u << 0)) != 0);
	
	for (std::uint32_t i = 0;
     i < config.conversionCycles - 1;
     ++i)
	{
		adc.tick(i);
	}
	assert((adc.read(2) & (1u << 0)) != 0); // BUSY
	assert((adc.read(2) & (1u << 1)) == 0); // not COMPLETE
	
	adc.tick(config.conversionCycles - 1);
	
	assert((adc.read(2) & (1u << 0)) == 0);
	
	assert(adc.read(3) == 512);
	
	assert(adc.read(4) == 0);
	
	//ADC 2
	ADC adc2(
		config,
		gpio,
		interruptController
	);
	
	gpio.getPin(0).setExternalVoltage(1.0);

	adc2.write(0, 0);
	adc2.write(1, 1);
	
	gpio.getPin(0).setExternalVoltage(4.0);
	
	for (std::uint32_t i = 0;
		 i < config.conversionCycles;
		 ++i)
	{
		adc2.tick(i);
	}
	
	assert(adc2.read(3) == 205);
	
	//ADC 3
	ADC adc3(
		config,
		gpio,
		interruptController
	);
	
	gpio.getPin(0).setExternalVoltage(1.0);
	gpio.getPin(1).setExternalVoltage(4.0);
	
	adc3.write(0, 0);
	adc3.write(1, 1);
	
	adc3.write(0, 1);
	
	for (std::uint32_t i = 0;
		 i < config.conversionCycles;
		 ++i)
	{
		adc3.tick(i);
	}
	
	assert(adc3.read(3) == 205);
	assert(adc3.read(4) == 0);
	
	adc3.write(1, 1);
	
	for (std::uint32_t i = 0;
		 i < config.conversionCycles;
		 ++i)
	{
		adc3.tick(i);
	}
	
	assert(adc3.read(3) == 818);
	assert(adc3.read(4) == 1);
	
	//ADC 4
	ADC adc4(
		config,
		gpio,
		interruptController
	);

	gpio.getPin(0).setExternalVoltage(1.0);

	adc4.write(0, 0);
	adc4.write(1, 1);
	
	for (std::uint32_t i = 0;
		 i < config.conversionCycles / 2;
		 ++i)
	{
		adc4.tick(i);
	}
	
	adc4.write(1, 1);
	
	for (std::uint32_t i = config.conversionCycles / 2;
		 i < config.conversionCycles;
		 ++i)
	{
		adc4.tick(i);
	}
	
	assert((adc4.read(2) & (1u << 0)) == 0); // not BUSY
	assert((adc4.read(2) & (1u << 1)) != 0); // COMPLETE
	assert(adc4.read(3) == 205);
	
	//ADC 5
	ADC adc5(
		config,
		gpio,
		interruptController
	);
	
	adc5.write(0, 12);
	adc5.write(1, 1);
	
	assert((adc5.read(2) & (1u << 0)) == 0); // not BUSY
	assert((adc5.read(2) & (1u << 3)) != 0); // INVALID_CHANNEL
	
	assert(adc5.read(3) == 0);
	assert(adc5.read(4) == 0);
	
	adc5.write(2, (1u << 3));
	
	assert((adc5.read(2) & (1u << 3)) == 0);
	
    return 0;
}