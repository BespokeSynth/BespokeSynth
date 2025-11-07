# Module Reference

Complete reference for all BespokeSynth modules.

## Overview

BespokeSynth contains over 200 modules organized into categories. This reference provides detailed information about each module's purpose, controls, and usage.

## Module Categories

### [Instruments](instruments.md) 🎹
Generate note sequences and MIDI input.

**Common modules**:
- **notesequencer** - Step sequencer for melodies
- **drumsequencer** - Step sequencer for drums
- **midicontroller** - MIDI input device
- **keyboarddisplay** - Virtual keyboard
- **notecanvas** - Piano roll editor

**Use for**: Creating note patterns, MIDI input, sequencing

### [Note Effects](note-effects.md) 🎵
Process and transform note messages.

**Common modules**:
- **arpeggiator** - Arpeggiate chords
- **chorder** - Add harmony to notes
- **notedelayer** - Delay note messages
- **quantizer** - Quantize notes to scale
- **notegate** - Gate/filter notes

**Use for**: Note processing, harmony, timing effects

### [Synths](synths.md) 🔊
Generate audio from note messages.

**Common modules**:
- **karplusstrong** - Plucked string synthesis
- **fmsynth** - FM synthesis
- **sampler** - Sample playback
- **drumplayer** - Drum sample player
- **oscillator** - Basic waveform generator

**Use for**: Sound generation, synthesis

### [Audio Effects](audio-effects.md) 🎚️
Process audio signals.

**Common modules**:
- **effectchain** - Chain multiple effects
- **delay** - Echo/delay effect
- **freeverb** - Reverb effect
- **compressor** - Dynamic range compression
- **distortion** - Distortion/saturation

**Use for**: Audio processing, mixing, sound design

### [Modulators](modulators.md) 📈
Create control voltage (CV) signals.

**Common modules**:
- **lfo** - Low-frequency oscillator
- **envelope** - ADSR envelope
- **expression** - Manual control slider
- **curve** - Custom curve editor
- **ramp** - Ramp generator

**Use for**: Parameter modulation, automation

### [Pulse](pulse.md) ⚡
Timing, triggers, and gates.

**Common modules**:
- **pulser** - Generate regular pulses
- **pulsegate** - Gate pulse signals
- **pulsetrain** - Burst of pulses
- **audiotopulse** - Convert audio to pulses
- **notetopulse** - Convert notes to pulses

**Use for**: Triggering, timing, rhythm generation

### [Other](other.md) 🔧
Utilities and special functions.

**Common modules**:
- **transport** - Global tempo and timing
- **scale** - Global scale/key
- **script** - Python scripting
- **snapshots** - Save/recall presets
- **prefab** - Module groups
- **comment** - Documentation

**Use for**: Utilities, organization, scripting

## Signal Types

Understanding signal types is crucial for patching:

| Type | Color | Description | Modules |
|------|-------|-------------|---------|
| **Notes** | 🟠 Orange | MIDI-style note messages | Instruments, Note Effects, Synths |
| **Audio** | 🔵 Cyan | Audio waveforms | Synths, Audio Effects, Output |
| **Pulses** | 🟡 Yellow | Trigger/gate signals | Pulse modules, Instruments |
| **Modulators** | 🟢 Green | Control voltage (CV) | Modulators, any parameter |

## Common Signal Flows

### Basic Instrument
```
notesequencer → synth → output
```

### With Effects
```
notesequencer → synth → delay → reverb → output
```

### With Note Processing
```
notesequencer → arpeggiator → chorder → synth → output
```

### Pulse-Driven
```
pulser → notecounter → notesequencer → synth → output
```

### Modulated
```
lfo → synth (filter cutoff)
notesequencer → synth → output
```

## Module Naming Conventions

BespokeSynth modules follow naming patterns:

### By Function
- **sequencer** suffix: Creates sequences (notesequencer, drumsequencer)
- **player** suffix: Plays audio (drumplayer, sampleplayer)
- **to** infix: Converts signals (audiotopulse, notetopulse)
- **displayer** suffix: Visualizes data (notedisplayer, pulsedisplayer)

### By Signal Type
- **note** prefix: Works with notes (notedelayer, notegate)
- **pulse** prefix: Works with pulses (pulser, pulsegate)
- **audio** prefix: Works with audio (audiotopulse, audioleveltocv)

## Finding the Right Module

### By Task

**I want to...**

**Create melodies**:
- notesequencer, notecanvas, midicontroller

**Create drum patterns**:
- drumsequencer, drumplayer

**Add harmony**:
- chorder, arpeggiator

**Generate sound**:
- karplusstrong, fmsynth, sampler

**Add effects**:
- effectchain, delay, reverb

**Modulate parameters**:
- lfo, envelope, expression

**Trigger events**:
- pulser, pulsegate

**Organize patch**:
- prefab, comment, snapshots

### By Category

Use the category links above to browse all modules in each category.

### By Search

In BespokeSynth:
1. Right-click canvas
2. Start typing module name
3. Matching modules appear

## Module Documentation Format

Each module page includes:

- **Description**: What the module does
- **Controls**: All parameters and their functions
- **Inputs**: What signals it accepts
- **Outputs**: What signals it produces
- **Common Uses**: Typical applications
- **Tips**: Usage suggestions
- **Related Modules**: Similar or complementary modules

## Special Modules

### Required Modules

These modules are always present and cannot be deleted:

**output**:
- Final destination for audio
- Volume control and limiter
- VU meter

**transport**:
- Global tempo (BPM)
- Time signature
- Swing amount

**scale**:
- Global key/scale
- Root note
- Scale type

### Utility Modules

**comment**:
- Add text notes to patch
- Document your work
- No signal processing

**label**:
- Text labels for organization
- Visual markers
- No signal processing

**prefab**:
- Save module groups
- Reusable templates
- Share with others

## Module Development

Want to create your own modules?

See [Module Development Guide](../development/module-development.md) for:
- Module architecture
- C++ module creation
- Python script modules
- Contributing modules

## Quick Reference Tables

### Instruments

| Module | Purpose | Complexity |
|--------|---------|------------|
| notesequencer | Step sequencer | ⭐⭐ |
| drumsequencer | Drum patterns | ⭐⭐ |
| midicontroller | MIDI input | ⭐ |
| notecanvas | Piano roll | ⭐⭐⭐ |
| randomnote | Random notes | ⭐ |

### Synths

| Module | Type | Complexity |
|--------|------|------------|
| karplusstrong | Physical modeling | ⭐⭐ |
| fmsynth | FM synthesis | ⭐⭐⭐ |
| sampler | Sample playback | ⭐⭐ |
| drumplayer | Drum samples | ⭐ |
| oscillator | Basic waveforms | ⭐ |

### Effects

| Module | Type | CPU Usage |
|--------|------|-----------|
| delay | Time-based | Low |
| freeverb | Reverb | Medium |
| compressor | Dynamics | Low |
| granulator | Granular | High |
| distortion | Saturation | Low |

### Modulators

| Module | Type | Use Case |
|--------|------|----------|
| lfo | Oscillator | Cyclic modulation |
| envelope | ADSR | Note-triggered |
| expression | Manual | Performance control |
| curve | Custom | Complex shapes |
| ramp | Linear | Transitions |

## Tips for Learning Modules

1. **Start simple**: Learn core modules first (notesequencer, karplusstrong, output)
2. **Experiment**: Try modules in isolation to understand them
3. **Read descriptions**: Each module has helpful documentation
4. **Watch tutorials**: Video tutorials demonstrate module usage
5. **Ask community**: Discord community is helpful
6. **Build patches**: Best way to learn is by doing

## Module Compatibility

### Version Compatibility

- Modules may be added in newer versions
- Older patches may not have newer modules
- Update BespokeSynth for latest modules

### Platform Compatibility

- Most modules work on all platforms (Windows, macOS, Linux)
- Some modules require specific features (e.g., VST plugins)
- Check module documentation for platform notes

## Next Steps

Explore module categories:

1. **[Instruments](instruments.md)** - Start here for note generation
2. **[Synths](synths.md)** - Learn sound synthesis
3. **[Audio Effects](audio-effects.md)** - Process your sounds
4. **[Modulators](modulators.md)** - Add movement
5. **[Note Effects](note-effects.md)** - Transform notes
6. **[Pulse](pulse.md)** - Master timing
7. **[Other](other.md)** - Utilities and special modules

Or jump to:
- **[User Guide](../user-guide/README.md)** - Learn the interface
- **[Scripting](../scripting/README.md)** - Python scripting
- **[Advanced Topics](../advanced/README.md)** - Deep dive

