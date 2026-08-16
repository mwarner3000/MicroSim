# MicroSim Architecture

## 1. Purpose

MicroSim is a modular embedded-controller simulation framework. Its primary responsibility is to simulate the internal behavior and external hardware interface of configurable microcontrollers.

A MicroSim controller should behave as though the environment connected to it is the real world. The controller itself should not require knowledge of the simulation environment that provides its inputs, consumes its outputs, or determines how quickly simulated world time runs.

This allows the same simulated controller and firmware to operate in different host environments.

## 2. Design Principles

### Hardware and software separation

The simulated CPU executes machine code. Assembly syntax, parsers, assemblers, compilers, and other toolchain components remain separate from simulated hardware.

### Environment independence

The controller contains no application-specific world state. It does not know whether a voltage represents vehicle speed, temperature, pressure, position, or another external quantity.

### Simulated time is not wall-clock time

MCU timing is determined by simulated hardware cycles and board clock frequency. Host execution speed must not alter simulated MCU behavior.

### Modular peripherals

Peripherals communicate through defined interfaces and should not depend directly on CPU implementation details.

### Bus-based CPU/device communication

The CPU communicates with RAM and memory-mapped peripherals through the system bus rather than directly accessing device implementations.

### Configurable hardware

Board configuration describes hardware characteristics such as memory capacity, clock frequency, pin count, electrical parameters, and memory-map locations.

### Deterministic execution

Given the same initial state and sequence of external inputs, simulation behavior should be repeatable regardless of how quickly the host computer performs the calculation.

### Testable subsystems

Major components should be independently testable before integration into a complete controller.

## 3. High-Level Architecture

```text
                    External Environment
                           │
                 voltages / simulated time
                           │
                           ▼
                    ┌──────────────┐
                    │     Pins     │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │ Peripherals  │
                    │ GPIO / Timer │
                    │    / etc.    │
                    └──────┬───────┘
                           │
                    ┌──────▼───────┐
                    │     Bus      │
                    └───┬──────┬───┘
                        │      │
                  ┌─────▼─┐  ┌─▼──────┐
                  │  CPU  │  │  RAM   │
                  └───────┘  └────────┘
```

`Simulator` currently assembles these components into a functioning controller.

## 4. Board Configuration

`BoardConfig` currently contains:

- `clockHz`
- `ramWords`
- `gpioPins`
- `timerCount`
- `logicVoltage`
- `digitalHighThreshold`
- `ramBase`
- `gpioBase`
- `timerBase`

Configuration describes hardware being constructed rather than requiring source changes for each board.

For example, two generic boards may differ in clock rate, RAM, GPIO count, voltage, and address layout without representing exact commercial MCU part numbers.

Not every configuration field is necessarily fully generalized throughout the simulator yet. In particular, `timerCount` does not currently construct an arbitrary number of timer instances.

## 5. CPU and Instruction Set

The CPU executes machine instructions and interacts with the system bus. Assembly-language text is not interpreted by the CPU.

```text
Assembler / Compiler
        │
        ▼
   Machine Code
        │
        ▼
      CPU
        │
        ▼
       Bus
```

The current development CPU uses SimpleISA. SimpleISA is a development ISA and does not define the only language or CPU architecture MicroSim may support.

A future assembler may use different syntax while producing compatible machine code. Future CPUs may use different instruction sets without requiring unrelated peripherals to be rewritten.

## 6. Bus and Memory Map

The system bus routes memory-mapped reads and writes to attached devices implementing the common bus-device interface.

```text
CPU
 │
 ▼
Bus
 ├── RAM
 ├── GPIO
 ├── Timer
 └── future peripherals
```

`BoardConfig` currently supplies base addresses for RAM, GPIO, and Timer. The simulator uses these configured locations when attaching the devices.

Devices should not require direct references to the CPU.

## 7. Memory

RAM is a bus device. Its size is determined by board configuration when the simulator is constructed, and the CPU accesses it through the system bus.

The current RAM model stores 32-bit values indexed by bus address.

## 8. GPIO and Pins

### Physical pin model

`GPIO` owns a runtime-sized `std::vector<Pin>`. `BoardConfig::gpioPins` determines the number of pins created.

The physical/logical pin count is therefore not limited by the width of a CPU register. A generic GPIO device may contain 8, 100, or more pins without requiring the user to divide those pins into artificial 32-pin banks.

Each `Pin` currently stores simulated voltage and runtime direction. Pins default to input.

### Generic CPU-facing GPIO interface

The current generic GPIO peripheral uses four memory-mapped offsets:

| Offset | Register | Description |
| --- | --- | --- |
| 0 | `PIN_SELECT` | Select the pin used by subsequent GPIO operations |
| 1 | `DIRECTION` | Read/set selected pin direction (`0` input, `1` output) |
| 2 | `OUTPUT` | Read/drive selected output pin (`0` LOW, `1` HIGH) |
| 3 | `INPUT` | Read selected pin (`0` LOW, `1` HIGH); CPU read-only |

For example, to drive pin 73 HIGH, firmware conceptually performs:

```text
PIN_SELECT = 73
DIRECTION  = 1
OUTPUT     = 1
```

Internally this operates on `pins[73]`. There is no requirement for firmware to address a synthetic 32-pin bank merely because the CPU uses 32-bit values.

This pin-selected interface is the current generic MicroSim GPIO design. A future CPU or board model may expose a different GPIO register layout when that is useful for approximating another hardware architecture, while retaining the same underlying pin concept.

### Input behavior

The external environment may provide a voltage to an input pin. GPIO compares the selected pin voltage against `digitalHighThreshold` when producing a digital input value.

```text
External environment
        │ voltage
        ▼
       Pin
        │
        ▼
      GPIO
        │ threshold
        ▼
       0/1
        │
        ▼
       CPU
```

### Output behavior

For a configured output pin, the current generic GPIO maps:

```text
Digital LOW  -> 0 V
Digital HIGH -> configured logic voltage
```

The external environment may observe that voltage and determine what it means in the simulated world.

### Electrical scope

Pin voltage is modeled relative to the simulated board reference. MicroSim does not determine whether a voltage represents an open circuit, sensor value, relay command, motor command, or other world behavior. Detailed circuit simulation is outside the current core scope.

## 9. Timer

The timer is memory mapped and clock driven. It maintains internal state including a counter, period, enabled state, and expiration state.

The timer continues to participate in MCU clock advancement independently of whether the CPU is executing useful instructions. Additional timer instances and more advanced timer behavior may be added later.

## 10. Clock and Simulated Time

### Fundamental cycle definition

One MicroSim cycle represents one hardware clock cycle of the configured MCU.

`BoardConfig::clockHz` determines the relationship between simulated elapsed time and hardware cycles:

```text
cycles = simulated seconds × clockHz
```

For example:

```text
16 MHz controller + 1 ms = 16,000 cycles
32 MHz controller + 1 ms = 32,000 cycles
```

### Advancing cycles

`Simulator::advanceCycles(N)` executes `N` simulated hardware cycles. It is not a time skip. Clock-driven hardware receives those cycles and may change state during them.

### Advancing time

`Simulator::advanceTime(std::chrono::nanoseconds)` converts requested simulated elapsed time into hardware cycles using the configured clock frequency and then executes those cycles.

Fractional-cycle timing is accumulated between calls so repeated small time advances do not continually discard sub-cycle time.

### Wall-clock independence

MicroSim hardware does not decide whether the host simulation runs in real time, slow motion, accelerated time, or as fast as computationally possible.

For example, ten years of simulated world time may be calculated in far less than ten years of real wall-clock time if the host has sufficient computational performance. The simulated MCU still experiences its configured number of cycles per simulated second.

The simulation-speed multiplier therefore belongs to the host environment, not the MCU.

### Halted CPU versus MCU clock

A halted CPU does not imply that the MCU clock ceases to exist. Clock-driven peripherals may continue advancing while the CPU performs no further instruction fetches. Future sleep or power-management features may introduce explicit clock-gating behavior separately.

## 11. Simulator

`Simulator` represents one complete simulated controller instance. It owns and connects the board's current major components:

- Bus
- RAM
- GPIO
- Timer
- SimpleCPU
- Clock

It constructs these components according to `BoardConfig`, coordinates clock-driven execution, and exposes cycle/time advancement APIs.

`Simulator` should not contain environment-specific application logic.

## 12. External Environment Boundary

MicroSim stops at the controller boundary.

Consider a simulated vehicle. A vehicle/sensor model outside MicroSim might determine:

```text
Wheel speed -> sensor model -> 2.5 V
```

MicroSim receives only the electrical input:

```text
Input pin = 2.5 V
```

Firmware interprets that signal according to the program running on the controller.

The reverse applies to outputs. If MicroSim produces an output pin voltage, the external environment decides whether it activates a relay, changes motor behavior, illuminates a lamp, or causes another world effect.

The external environment also controls simulated world time. It tells a MicroSim instance how much simulated time to advance; the MCU does not directly query a world clock.

A host integration may conceptually operate as:

```text
set controller inputs
        │
        ▼
advance MicroSim by simulated dt
        │
        ▼
observe controller outputs
        │
        ▼
advance external world model
```

The exact external integration API is not yet finalized.

## 13. Multiple Controllers

A future system may contain multiple independent MicroSim instances:

```text
        ┌───────────┐
        │   Bot A   │
        │ MicroSim  │
        └─────┬─────┘
              │
              │ shared CAN
              │
        ┌─────┴───────────┐
        │                 │
 ┌──────▼─────┐    ┌──────▼─────┐
 │   Bot B    │    │   Bot C    │
 │ MicroSim   │    │ MicroSim   │
 └────────────┘    └────────────┘
```

Each controller remains an independent embedded system with its own CPU, memory, peripherals, firmware, configuration, and clock state.

CAN or similar controller-to-controller communication belongs to the simulated embedded system and is distinct from the external world/environment interface.

The existing multi-node `Simulation` infrastructure is an early step toward this capability; its final role and API are not yet settled.

## 14. Testing

Current test targets include:

- `RAMTests`
- `BusTests`
- `GPIOTests`
- `TimerTests`
- `CPUTests`
- `SimulationTests`

Tests should be added whenever new hardware behavior is introduced or a bug is discovered.

Important integration paths include:

```text
Firmware -> CPU -> Bus -> GPIO -> Pin
```

and:

```text
External Input -> Pin -> GPIO -> Bus -> CPU
```

Timing tests should also verify that configured clock rates produce the expected number of hardware cycles for the same simulated elapsed time.

## 15. Current Limitations

MicroSim is early in development. Current limitations include:

- Only one Timer object is currently constructed even though `timerCount` exists in configuration.
- The GPIO electrical model is intentionally simplified.
- The generic GPIO interface does not yet model alternate functions, analog conversion, pull-ups, output-driver characteristics, or pin multiplexing.
- External simulator integration APIs are not yet finalized.
- CAN and other multi-controller communication are not yet implemented.
- Interrupt handling is not yet implemented.
- DMA and multiple bus masters are not yet implemented.
- Large single-call time conversions are not yet designed as a fast-forward mechanism; advancing time currently represents actual hardware execution.

These limitations should be addressed incrementally rather than by introducing assumptions that unnecessarily constrain future board models.

## 16. Out of Scope

The core MicroSim project is not intended to become:

- a vehicle physics simulator
- a robotics physics engine
- a circuit/SPICE simulator
- a mechanical simulator
- a sensor-physics simulator
- an exact emulator of a specific commercial MCU

External systems may implement those capabilities and communicate with MicroSim through defined interfaces.

## 17. Future Architecture

Likely future areas include:

- formal external integration APIs
- CAN communication
- interrupts
- ADC and analog-capable peripherals
- PWM
- additional timer instances
- more configurable peripheral layouts
- additional CPUs and instruction sets
- assembler/compiler tooling
- configurable board profiles
- richer pin capabilities where required

These are architectural directions rather than promises of currently implemented functionality.