# MicroSim

MicroSim is a modular microcontroller simulation framework written in C++.

The goal of MicroSim is to provide configurable simulated microcontrollers that
behave as self-contained embedded controllers. A simulated controller can execute
firmware, interact with peripherals, receive external input signals, and produce
output signals without needing to know anything about the environment in which it
is being used.

MicroSim is intended for experimentation, education, prototyping, robotics
simulation, and embedded-system development.

## Project Goals

MicroSim is designed around several core goals:

- Simulate configurable microcontroller hardware.
- Keep simulated hardware separate from user software and toolchains.
- Keep the microcontroller independent from the external simulated world.
- Allow external simulators to provide inputs and consume outputs.
- Support multiple simulated controllers in the same system.
- Allow future communication between controllers through interfaces such as CAN.
- Allow users to approximate different classes of microcontrollers by changing
  characteristics such as clock speed, RAM size, GPIO count, and electrical
  parameters.
- Keep peripherals modular so additional devices can be added without redesigning
  the core simulator.

MicroSim is not intended to be an exact transistor-level or electrical simulation
of a specific commercial microcontroller.

Instead, it aims to model the behavior and resource constraints of a configurable
embedded controller at a useful level of abstraction.

## Current Status

MicroSim is currently under active development.

Implemented components currently include:

- System bus
- RAM
- GPIO
- Voltage-based GPIO pin model
- Timer peripheral
- Simulation clock
- Simple CPU
- Simple instruction set
- Board configuration
- Multi-node simulation infrastructure
- Automated tests for major subsystems

Some configuration options already exist in `BoardConfig` but are not yet fully
implemented throughout the simulator.

The project should currently be considered experimental and its APIs may change
during development.

## Hardware / Software Separation

MicroSim separates the simulated hardware from the software used to program it.

The CPU executes machine instructions defined by its instruction set. Assembly
syntax is not intended to be part of the simulated hardware.

This allows future assemblers, compilers, parsers, or other development tools to
target a MicroSim CPU without requiring changes to the CPU, peripherals, or other
simulated hardware.

The current SimpleISA instruction set exists to develop and test the simulator and
does not prevent other instruction sets or software toolchains from being added in
the future.

## External Simulation Boundary

A MicroSim controller does not model the physical world around it.

For example, MicroSim does not need to know that an input voltage represents:

- wheel speed
- temperature
- pressure
- a switch
- a sensor
- a joystick
- a motor
- vehicle state

The external environment supplies electrical/input values to the simulated
controller. Firmware running on the controller determines how those values are
used.

Likewise, MicroSim exposes controller outputs and the external environment decides
what those outputs affect.

This is intended to allow a MicroSim controller to be integrated into different
simulation environments without rewriting the controller or its firmware for each
environment.

## Configurable Boards

`BoardConfig` is intended to describe the resources and characteristics of a
simulated controller.

Current or planned configurable properties include:

- CPU clock frequency
- RAM size
- GPIO pin count
- timer count
- logic voltage
- digital input threshold

The long-term goal is to allow users to create different generic board
configurations in much the same way that real embedded projects may choose between
microcontrollers with different resources and performance characteristics.

MicroSim is not currently intended to reproduce a particular commercial MCU
exactly.

## Building

MicroSim uses CMake and requires a C++20-compatible compiler.

From the repository root:

```sh
cmake -S . -B build
cmake --build build```

On systems where a specific CMake generator is required, select the appropriate
generator when configuring the build.

##Running Tests

After building:

```sh
ctest --test-dir build --output-on-failure```

The current test suite covers major components including:

RAM
Bus
GPIO
Timer
CPU
Simulation
Project Structure
MicroSim/
├── docs/       Project design and architecture documentation
├── include/    Public headers
├── src/        Implementation source
├── tests/      Automated tests
├── CMakeLists.txt
└── README.md

##Future Direction

Possible future development includes:

Fully configurable GPIO counts
Configurable memory maps
Additional timers and peripherals
Analog input / ADC support
PWM and other pin functions
Interrupt handling
Additional CPU architectures or instruction sets
External assembler/compiler tooling
Multiple-controller systems
CAN or similar controller-to-controller communication
APIs for integration with third-party simulation environments

These items describe project direction and should not be assumed to be implemented
unless documented otherwise.

##Scope

MicroSim focuses on the simulated embedded controller.

Detailed physical-world simulation, vehicle dynamics, robotics physics, circuit
simulation, sensor physics, and similar environment-specific behavior are outside
the core scope of the project.

Those systems can instead interact with MicroSim through its external interfaces.