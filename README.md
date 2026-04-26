# Modular Wavetable Synth (Standalone)

A C++17 standalone synth with a GTK UI and modular DSP blocks (oscillator, filter, envelope, wavetable manager).

## What It Does

- Runs as a desktop app (`modular-synth`) with real-time audio output (PortAudio).
- Uses a modular signal path implemented in `AudioPipeline`.
- Includes a step sequencer tab (first page) with:
  - BPM control
  - start/stop
  - editable step grid
  - configurable column count
  - configurable time signature (`numerator/denominator`)
  - visual column segmentation based on `timeSignatureDenominator`
- Includes a core test binary (`synth-tests`) with live test status output.

## Build And Run

This project uses a `Makefile` (not CMake).

- Build app: `make`
- Run app: `make run`
- Clean: `make clean`
- Rebuild: `make rebuild`
- Debug build: `make debug`
- Build and run tests: `make test`

## Dependencies

- `g++` with C++17 support
- GTK 3 development packages (via `pkg-config gtk+-3.0`)
- PortAudio (`-lportaudio`)
- POSIX threads (`-pthread`)

## Project Layout

```
ModularSynth/
├── src/
│   ├── core/
│   │   ├── audio_output.h/.cpp
│   │   ├── audio_pipeline.h/.cpp
│   │   ├── wavetable.h/.cpp
│   │   └── wavetable_manager.h/.cpp
│   ├── modules/
│   │   ├── oscillator.h/.cpp
│   │   ├── filter.h/.cpp
│   │   └── envelope.h/.cpp
│   ├── app/
│   │   ├── synth_component.h/.cpp
│   │   ├── midi_handler.h/.cpp
│   │   ├── gui.h/.cpp
│   │   ├── plugin_interface.h/.cpp    # legacy/reference
│   │   └── vst_plugin.h/.cpp          # legacy/reference
│   └── main.cpp
├── tests/
│   └── test_synth_core.cpp
├── Makefile
└── README.md
```

## Sequencer Notes

- Sequencer is the first notebook tab.
- Grid defaults to 12 rows x 16 columns.
- Column count can be changed at runtime; existing step values are preserved where possible.
- Time signature affects step timing interval and segmentation.
- Segment boundaries are currently visual spacing groups computed from:
  - `segmentSize = max(1, sequencerColumns / timeSignatureDenominator)`

## Current Signal Flow

Pipeline processing is implemented as:

1. Oscillator render to per-oscillator buffer
2. Optional filter stage
3. Optional envelope gain stage
4. Mixdown to output channel

The app currently uses a monophonic note trigger helper (`noteOn`/`noteOff` with note matching).

## Tests

`make test` builds and runs `synth-tests`.

Current test coverage includes:

- envelope sustain/release behavior
- envelope parameter clamping
- oscillator sample-rate scaling behavior
- wavetable interpolation and wrapping
- wavetable manager add/remove and generation
- pipeline note-off note matching
- pipeline large-buffer processing smoke test

Test runner prints status as it executes:

- `[ RUN ]`
- `[ PASS ]` / `[ FAIL ]`
- final summary with pass/fail totals

## Notes On Legacy Plugin Code

`plugin_interface` and `vst_plugin` sources are kept in `src/app/` for reference and future work, but the standalone build target is the primary supported runtime path.