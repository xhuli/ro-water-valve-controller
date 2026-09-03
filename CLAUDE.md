# RO Water Valve Controller — Project Instructions

Repository: `ro-water-valve-controller-master`

Fast orientation for Claude in this project. **`README.md` is the source of truth for hardware/behavior facts** (pin map, control behavior, build/target details, safety rationale) — this file doesn't restate them, to avoid the two drifting apart (which already happened once: this file said "8 MHz" while the board's real PlatformIO definition was 16.5 MHz, caught and fixed 2026-09-02/03). Read `README.md` first for what the device does and how it's built; this file is about how to work in the repo.

---

## 1. What this project is

RO Water Valve Controller is an ATtiny85-based automatic fill controller for an RO/DI water reservoir — normal + independent high-level sensor, electric fill valve, LED/buzzer alarm. See `README.md` for the full behavior description, pin map, and safety rationale.

Firmware is C++/Arduino for ATtiny85. The project is intentionally small: no FSM, RTOS, EEPROM configuration, serial console, or component registry. Single-maintainer project (xhuli / Gorjan Djundev). Favors simplicity/explicitness over abstraction — mirrors the sibling `AquariumAto` project's stated preference, but the two are **not** a shared codebase (different MCU, different constraint budget — see §6).

---

## 2. Repository layout

```text
platformio.ini          # authoritative build/upload config
CMakeLists.txt          # CLion/CMake build — secondary but functional (see README's Build section)
README.md               # facts: overview, pin map, control behavior, build, testing, safety
BACKLOG.md              # live tracked work items and their status — check here before assuming something is still undone
src/main/main.cpp       # composition root: pin map, wiring, setup()/loop()
include/                # one header per class: Buzzer.h, Switchable.h, SwitchableWithDelay.h, WaterLevelSensor.h
test/                   # host-side regression tests (controllerTest.cpp) + standalone CMake project
doc/images/              # schematic, PCB, hardware photos
```

For file-by-file behavior, read the code — it's short. For what each of the four `include/` classes does, see the doc comment at the top of its own header.

---

## 3. Build and target

PlatformIO is authoritative. `CMakeLists.txt` is secondary but has been fixed to produce a real, working build (not just IDE navigation) — see `README.md`'s Build section for exact commands and for the current clock/target facts, which live there now, not here.

Do not silently change MCU, clock, fuses, board definition, or upload protocol — update `README.md` alongside any such change so the two stay in sync.

---

## 4. Existing helper classes

`Buzzer`, `Switchable`, `SwitchableWithDelay`, `WaterLevelSensor` live one-per-file under `include/`, each depending only on free Arduino functions (`pinMode`/`digitalWrite`/`digitalRead`/`millis`/`delay`) rather than `<Arduino.h>` directly, so they compile against both the real core (on-device) and a host test double (`test/ArduinoTestDouble.h`). Each timing method exposes a `uint32_t nowMs` overload (pure, deterministic, used by tests) alongside a `millis()`-based zero-arg one (used by production code).

The immediate valve functions (`switchOnNow`/`switchOffNow`) have safety significance — see README's Safety section. Keep the code simple and explicit; avoid adding abstraction without a clear benefit.

`Buzzer<N>` requires even `N` (`static_assert` in `include/Buzzer.h`) — see the class comment there for why (odd `N` makes the pin-toggle-at-every-boundary mechanism flip each slot's ON/OFF meaning every other repeat).

---

## 5. Testing

`test/controllerTest.cpp` drives the real production classes directly (not reimplementations) via `test/ArduinoTestDouble.h`, a minimal host-side stand-in for the Arduino functions those classes need, with a fabricated clock. Separate, standalone CMake project from the root build — see README's Testing section for commands and coverage.

When changing safety-related logic (the high-sensor override, the anti-chatter delay, immediate-shutoff bypass), add or update tests in `controllerTest.cpp` alongside the change. Prefer testing production code over writing a parallel copy — that was tried once (the old `buzzerTest.cpp`) and provided no real regression protection.

---

## 6. Relationship to `AquariumAto`

`AquariumAto` (checked out as a sibling folder, `AquariumAto/`, itself a separate git repo) targets a different MCU (ATmega328 vs this project's ATtiny85) with a much larger constraint budget (2KB vs 512B SRAM) and uses `virtual`/vtable-based `Runnable`/`AbstractSwitchable` patterns this project deliberately avoids. **Do not** introduce a shared library between the two projects or assume AquariumATO features exist here. Borrowing a *pattern* (e.g. the `process(nowMs)` testability split, used by both projects' timing classes) on a case-by-case basis is fine; sharing code is not — see `BACKLOG.md`'s "Explicitly out of scope" section for the fuller reasoning.

---

## 7. Hardware and electrical safety

The project can control low-voltage and potentially **mains-voltage** valve circuitry. Treat the high-voltage PCB area as hazardous. When modifying firmware or docs:

- distinguish low-voltage logic from mains switching;
- do not recommend live probing of energized mains circuitry;
- verify replacement valve voltage/current requirements;
- preserve PCB isolation, clearances, protection, and grounding assumptions.

Firmware must not be treated as the only flood or electrical safety layer — see README's Safety section for the independent-sensor/watchdog/no-timeout rationale.

---

## 8. Development conventions

1. High-level detection must always force valve OFF, immediately (no delay).
2. Keep `loop()` non-blocking — it has no blocking calls today and the watchdog assumes that stays true.
3. Preserve current sensor polarity unless hardware verification supports a change.
4. Do not silently change the anti-chatter valve delay or the buzzer's even-length requirement.
5. Keep firmware pin definitions (README's pin map, `main.cpp`'s `Pins` namespace) synchronized with hardware.
6. PlatformIO is authoritative; CMake is secondary but must stay a working build, not just navigation.
7. Avoid unnecessary dynamic allocation; ATtiny85 resources are limited (512B SRAM, 6012B flash for this board).
8. Consider flash/SRAM impact of every feature — check `pio run -e usbasp`'s size output before and after.
9. Preserve immediate high-level shutdown.
10. Add regression tests with behavior changes (see §5).
11. Surface unrelated behavioral drift instead of silently fixing it — same spirit as this file/README's own recent 8MHz-vs-16.5MHz correction.
12. Keep `README.md` aligned with actual firmware behavior; keep this file aligned with `README.md` (point at it, don't duplicate).
13. Track ongoing work in `BACKLOG.md`, not in this file.

---

## 9. Source-of-truth order

For implementation questions, inspect:

1. `src/main/main.cpp` and `include/*.h`
2. `README.md`
3. `platformio.ini`
4. `doc/images/electric.png` and `doc/images/pcb.png`
5. `BACKLOG.md` (current tracked work/status)
6. `test/`

If documentation and implementation disagree, surface the discrepancy rather than silently choosing one.
