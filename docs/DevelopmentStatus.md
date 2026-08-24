# MicroSim Development Status

MicroSim is under active development. The current development priority is to complete and stabilize a **single simulated MCU** before expanding the project to coordinated multi-controller simulation.

## Current Development Milestone: Single MCU

The active milestone is a self-contained configurable simulated microcontroller that can execute firmware and interact with its own memory, peripherals, pins, timers, and interrupt system.

Current single-MCU work includes the system bus, configurable RAM, GPIO and voltage-based pins, Timer, InterruptController, SimpleCPU/SimpleISA, board configuration, explicit cycle/time advancement, cooperative real-time synchronization, external pin access, and subsystem/integration testing.

Development should prioritize correctness and completeness of this single-controller model before adding features whose primary purpose is communication or synchronization between multiple controllers.

## Work in Progress / Deferred

The following areas have preliminary code or design work but are **not considered complete current capabilities**:

### CAN

CANFrame, CANBus, and CANController groundwork exists, but CAN development is paused until the single-MCU milestone is complete. Existing isolated CAN tests may exercise implemented pieces, but CAN should not yet be treated as a finished end-to-end controller communication feature.

A multi-node CAN firmware/interrupt integration test is currently disabled because correct behavior depends on coordinated event-driven scheduling between independent MCU clocks.

### Multiple Controllers

The `Simulation` layer contains early multi-node infrastructure. This is groundwork for future systems containing multiple independent MicroSim controllers, not a completed multi-controller simulation model.

Each future controller is intended to retain its own CPU, memory, peripherals, firmware, configuration, clock state, and interrupt state while sharing a common simulated-world timeline.

### Event-Driven Scheduler

Initial scheduler groundwork exists to order simulated events by timestamp. The intended future architecture uses a shared simulation timeline so controllers with different clock frequencies can execute in correct chronological order rather than one controller being advanced for an entire time slice before another.

This scheduler is currently experimental and is not yet the execution mechanism for the completed single-MCU path.

Fractional clock periods are being handled separately from the multi-node scheduler so common frequencies such as 16 MHz do not accumulate timing drift merely because one cycle is 62.5 ns rather than a whole number of nanoseconds.

## Future Multi-Controller Direction

After the single-MCU model is complete and stable, multi-controller work can resume around a shared event-driven timeline. The intended model is approximately:

```text
                 Shared simulation time
                         |
                    Event scheduler
                         |
          +--------------+--------------+
          |              |              |
        MCU A          MCU B           CAN
     next event      next event      next event
```

The scheduler should process due events chronologically. This avoids making simulated behavior depend on the order in which controllers happen to be stored or iterated by the host program.

CAN can then be integrated on top of that timing model rather than defining the timing architecture itself.

## Scope Boundary

MicroSim remains focused on the embedded controller. Physical-world simulation, robotics physics, vehicle dynamics, detailed circuit simulation, and application-specific sensor models remain outside the core project.

An external environment may provide inputs and observe outputs while the simulated MCU treats those signals as its physical world.

## Status Terminology

Documentation should use these terms consistently:

- **Implemented** — part of the active supported simulation path and covered by relevant tests.
- **Experimental / WIP** — code or design groundwork exists, but behavior or API is not yet considered complete.
- **Future** — planned direction that should not be assumed to exist yet.

CAN, coordinated multi-controller execution, and the event-driven scheduler are currently **experimental/WIP** and should not be presented as completed MicroSim capabilities.
