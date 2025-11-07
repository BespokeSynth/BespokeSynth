# Modulators

Modules that create control voltage (CV) signals for parameter modulation.

## Overview

Modulators generate control signals (purple cables) that can be connected to any slider to create movement and automation. They're essential for dynamic, evolving sounds.

## Core Modulators

### lfo
**Low-frequency oscillator**

Cyclic modulation at sub-audio rates.

**Key controls**:
- **rate**: Oscillation speed (Hz or musical)
- **type**: Sine, saw, square, triangle, random
- **min/max**: Modulation range

**Common uses**: Vibrato, tremolo, filter sweeps, rhythmic movement

**Tips**:
- Use musical rates for sync'd modulation
- Square wave for on/off effects
- Random for unpredictable movement
- Adjust min/max for modulation range

### envelope
**ADSR envelope generator**

Note-triggered envelope.

**Key controls**:
- **a**: Attack time
- **d**: Decay time
- **s**: Sustain level
- **r**: Release time

**Common uses**: Amplitude shaping, filter modulation, dynamic control

**Tips**:
- Connect to note input to trigger
- Fast attack for plucks
- Slow attack for pads
- Modulate filter cutoff for classic synth sound

### expression
**Manual control slider**

User-controlled modulation source.

**Key controls**:
- **value**: Current value
- **min/max**: Range

**Common uses**: Performance control, manual automation

**Tips**:
- Map to MIDI controller
- Use for live performance
- Great for filter sweeps
- Can be automated via script

## Advanced Modulators

### curve
**Custom curve editor**

Draw custom modulation curves.

**Key controls**:
- **curve editor**: Draw curve
- **length**: Curve duration
- **loop**: Loop curve

**Common uses**: Complex modulation shapes, custom envelopes

**Tips**:
- Draw any shape you want
- Great for unique modulation
- Can be note-triggered
- Save curves as presets

### ramp
**Linear ramp generator**

Generates linear ramps.

**Key controls**:
- **time**: Ramp duration
- **start**: Start value
- **end**: End value
- **trigger**: Trigger ramp

**Common uses**: Transitions, pitch slides, automation

### multislider
**Multiple sliders**

Multiple modulation outputs.

**Key controls**:
- **sliders**: Individual slider values
- **step**: Current step

**Common uses**: Step sequencing modulation, multiple targets

## Audio-to-CV Converters

### audioleveltocv
**Convert audio level to CV**

Tracks audio amplitude and outputs CV.

**Key controls**:
- **smooth**: Smoothing amount
- **min/max**: Output range

**Common uses**: Envelope following, dynamics-based modulation

**Tips**:
- Use for sidechain-style effects
- Great for reactive modulation
- Adjust smoothing for response

### pitchtocv
**Convert pitch to CV**

Tracks audio pitch and outputs CV.

**Key controls**:
- **smooth**: Smoothing amount

**Common uses**: Pitch tracking, pitch-based modulation

### audiotocv
**Convert audio to CV**

General audio-to-CV converter.

**Key controls**:
- **mode**: Conversion mode
- **smooth**: Smoothing

**Common uses**: Audio-reactive modulation

## Sequenced Modulation

### modulator
**Step sequencer for modulation**

Sequence modulation values.

**Key controls**:
- **steps**: Modulation values per step
- **length**: Number of steps
- **interval**: Step timing

**Common uses**: Rhythmic modulation, sequenced changes

**Tips**:
- Create rhythmic filter movement
- Sequence parameter changes
- Sync to tempo

### valuesequencer
**Value sequencer**

Sequences values over time.

**Key controls**:
- **values**: Sequence values
- **interval**: Step timing

**Common uses**: Parameter sequencing, automation

## Utility Modulators

### valueset
**Set of values**

Store and recall multiple values.

**Key controls**:
- **values**: Stored values
- **select**: Select value

**Common uses**: Preset values, switching between settings

### selector
**Value selector**

Select between multiple inputs.

**Key controls**:
- **inputs**: Input values
- **select**: Selected input

**Common uses**: Switching modulation sources, routing

### smoother
**Smooth value changes**

Smooths abrupt value changes.

**Key controls**:
- **smooth**: Smoothing amount

**Common uses**: Smoothing automation, preventing clicks

## Math & Logic

### add
**Add values**

Adds multiple CV signals.

**Common uses**: Combining modulation sources

### multiply
**Multiply values**

Multiplies CV signals.

**Common uses**: Scaling modulation, ring modulation

### scale
**Scale values**

Scales CV to different range.

**Key controls**:
- **in min/max**: Input range
- **out min/max**: Output range

**Common uses**: Range conversion, scaling modulation

### clamp
**Limit value range**

Clamps values to min/max.

**Key controls**:
- **min/max**: Clamp range

**Common uses**: Limiting modulation, safety

## Common Modulation Targets

### Filter Cutoff
```
lfo → synth (filter cutoff)
```
**Result**: Classic filter sweep

### Amplitude
```
lfo → synth (volume)
```
**Result**: Tremolo effect

### Pitch
```
lfo → synth (pitch)
```
**Result**: Vibrato

### Effect Parameters
```
lfo → delay (feedback)
```
**Result**: Evolving delay

### Multiple Targets
```
lfo → [filter cutoff, resonance, volume]
```
**Result**: Complex modulation

## Modulation Techniques

### Vibrato
- LFO → pitch
- Sine wave, 4-7 Hz
- Small amount (±5-10 cents)

### Tremolo
- LFO → volume
- Sine or triangle wave
- Musical rate (1/4, 1/8, etc.)

### Filter Sweep
- LFO → filter cutoff
- Saw or triangle wave
- Adjust min/max for range

### Envelope Filter
- Envelope → filter cutoff
- Fast attack, medium decay
- Classic synth sound

### Rhythmic Modulation
- Modulator (step sequencer) → parameter
- Sync to tempo
- Create rhythmic movement

## Tips & Best Practices

### Modulation Depth
1. **Start subtle**: Less is often more
2. **Adjust range**: Use min/max controls
3. **Layer modulation**: Multiple sources
4. **Sync to tempo**: Musical rates
5. **Experiment**: Try unexpected targets

### Performance
1. **Map to controller**: Use expression + MIDI
2. **Use snapshots**: Save modulation states
3. **Automate**: Script module for complex automation
4. **Keep it simple**: Too much modulation = chaos
5. **Test thoroughly**: Verify modulation ranges

### Creative Techniques
1. **Modulate modulators**: LFO → LFO rate
2. **Audio-rate modulation**: Fast LFO for FM
3. **Random modulation**: Random LFO for variation
4. **Envelope everything**: Not just amplitude
5. **Feedback modulation**: Modulate based on output

## Common Workflows

### Basic Vibrato
```
notesequencer → synth
lfo (sine, 5Hz) → synth (pitch)
```

### Filter Sweep
```
notesequencer → synth
lfo (saw, 1/4) → synth (filter cutoff)
```

### Dynamic Filter
```
notesequencer → synth
envelope → synth (filter cutoff)
```

### Rhythmic Effect
```
notesequencer → synth → delay
lfo (square, 1/8) → delay (feedback)
```

### Complex Modulation
```
notesequencer → synth
lfo1 → synth (filter cutoff)
lfo2 → synth (resonance)
envelope → synth (volume)
```

## Next Steps

- **[Synths](synths.md)** - Modulate synth parameters
- **[Audio Effects](audio-effects.md)** - Modulate effect parameters
- **[Scripting](../scripting/README.md)** - Advanced automation

