# MicroSim

MicroSim is a modular microcontroller simulation framework written in C++.

The goal of MicroSim is to provide configurable simulated microcontrollers that behave as self-contained embedded controllers. A simulated controller can execute firmware, interact with peripherals, receive external input signals, and produce output signals without needing to know anything about the environment in which it is being used.

MicroSim is intended for experimentation, prototyping, robotics simulation, embedded-system development, and testing generic microcontroller configurations.

## Project Goals

MicroSim is designed around several core goals:

- Simulate configurable microcontroller hardware.
- Keep simulated hardware separate from assembly syntax, parsers, compilers, and other software toolchains.
- Keep the microcontroller independent from the external simulated world.
- Allow external simulators to provide inputs, consume outputs, and advance simulated time.
- Keep simulated MCU time independent from host wall-clock speed.
- Support multiple independent simulated controllers in the same system.
- Allow future communication between controllers through interfaces such as CAN.
- Allow users to approximate different classes of microcontrollers by changing characteristics such as clock speed, RAM size, GPIO count, electrical parameters, and memory-map locations.
- Keep peripherals modular so additional devices can be added without redesigning the core simulator.

MicroSim is not intended to be an exact transistor-level, circuit-level, or brand-specific reproduction of a commercial microcontroller. Instead, it aims to model embedded-controller behavior and resource constraints at a useful level of abstraction.

## Current Status

MicroSim is under active development. Implemented components currently include:

- System bus
- Configurable RAM size and base address
- GPIO with runtime-configurable pin count
- Voltage-based `Pin` model
- Pin-selected GPIO register interface
- Configurable GPIO and timer base addresses
- Timer peripheral
- Simulation clock
- Clock-frequency-based simulated time advancement
- Simple CPU
- Simple instruction set
- Board configuration
- Multi-node simulation infrastructure
- Automated tests for major subsystems

The project should currently be considered experimental and its APIs may change during development.

## Hardware / Software Separation

MicroSim separates simulated hardware from the software used to program it.

The CPU executes machine instructions defined by its instruction set. Assembly-language syntax is not part of the simulated hardware. This allows future assemblers, compilers, parsers, or other development tools to target a MicroSim CPU without requiring changes to the CPU, peripherals, or other simulated hardware.

The current SimpleISA instruction set exists to develop and test the simulator and does not prevent other instruction sets or software toolchains from being added in the future.

## Simulated Time

One MicroSim clock cycle represents one hardware clock cycle of the configured simulated controller.

`BoardConfig::clockHz` determines how many MCU cycles occur during a requested amount of simulated time. For example, advancing a 16 MHz controller by 1 ms executes 16,000 hardware cycles, while a 32 MHz controller executes 32,000 cycles during the same simulated interval.

Advancing cycles does not skip hardware activity. The CPU and other clock-driven devices are advanced during those cycles.

MicroSim does not use host wall-clock time to determine MCU behavior. An external simulator may run slower than real time, in real time, or thousands of times faster than real time without changing what a given amount of simulated MCU time means.

The host controls how quickly simulated world time progresses. The MCU does not know or control the simulation speed multiplier.

## External Simulation Boundary

A MicroSim controller does not model the physical world around it.

For example, MicroSim does not need to know whether an input voltage represents wheel speed, temperature, pressure, a switch, a sensor, or some other world quantity. The external environment supplies input values to the simulated controller, firmware determines how those values are used, and the external environment decides what controller outputs affect.

The host advances MicroSim by simulated time rather than exposing a world clock directly to firmware. Firmware observes time through simulated MCU hardware such as timers and counters.

This boundary is intended to allow the same MicroSim controller and firmware to be integrated into unrelated simulation environments without rewriting world-specific values into the controller.

## GPIO and Pins

GPIO uses a runtime-sized `std::vector<Pin>`. The number of pins is therefore a board configuration property rather than being limited by a 32-bit register width.

The generic GPIO peripheral currently uses a pin-selected register interface:

| Offset | Register | Behavior |
| --- | --- | --- |
| 0 | `PIN_SELECT` | Selects the pin addressed by subsequent GPIO operations |
| 1 | `DIRECTION` | Reads or sets the selected pin direction |
| 2 | `OUTPUT` | Reads or drives the selected output pin LOW/HIGH |
| 3 | `INPUT` | Reads the selected pin as LOW/HIGH and is CPU read-only |

This allows a generic board to expose 8, 100, or more pins without dividing the user-visible GPIO model into artificial 32-pin banks. CPU word width remains an implementation property and does not define the total number of simulated pins.

Pins carry simulated voltage values. Digital HIGH/LOW interpretation uses the board's configured logic voltage and digital HIGH threshold. Detailed circuit behavior and the physical meaning of those voltages remain outside MicroSim's core scope.

## Configurable Boards

`BoardConfig` currently provides configuration for:

- CPU clock frequency
- RAM size
- GPIO pin count
- timer count
- logic voltage
- digital HIGH threshold
- RAM base address
- GPIO base address
- timer base address

The long-term goal is to let users create generic board configurations approximating the resources and performance characteristics needed for a real embedded project without requiring MicroSim to reproduce a particular commercial MCU exactly.

## Building

MicroSim uses CMake and requires a C++20-compatible compiler.

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

On systems where a specific CMake generator is required, select the appropriate generator when configuring the build.

## Running Tests

After building:

```sh
ctest --test-dir build --output-on-failure
```

The current test suite covers RAM, Bus, GPIO, Timer, CPU, and Simulation behavior.

## Project Structure

```text
MicroSim/
├── docs/       Project design and architecture documentation
├── include/    Public headers
├── src/        Implementation source
├── tests/      Automated tests
├── CMakeLists.txt
└── README.md
```

## Future Direction

Likely future development includes:

- Formal external-simulator integration APIs
- More configurable peripheral construction
- Additional timers and peripherals
- Analog input / ADC support
- PWM and other pin functions
- Interrupt handling
- CAN or similar controller-to-controller communication
- Additional CPU architectures or instruction sets
- External assembler/compiler tooling
- More flexible board profiles and peripheral layouts

These items describe project direction and should not be assumed to be implemented unless documented otherwise.

## Scope

MicroSim focuses on the simulated embedded controller.

Detailed physical-world simulation, vehicle dynamics, robotics physics, circuit simulation, sensor physics, and similar environment-specific behavior are outside the core scope of the project. Those systems can instead interact with MicroSim through its external interfaces.