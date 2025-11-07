# Synths

Modules that generate audio from note messages.

## Overview

Synth modules convert note messages (orange cables) into audio signals (cyan cables). They are the sound generators of BespokeSynth.

## Physical Modeling

### karplusstrong
**Plucked string synthesis**

Physical modeling of plucked strings. Warm, organic sound.

**Key controls**:
- **excitation freq**: Pluck brightness
- **feedback**: String sustain
- **damping**: High-frequency damping
- **stretch**: Inharmonicity
- **inv decay**: Decay time

**Sound character**: Plucked strings, guitar, harp, bass
**CPU usage**: Low
**Polyphony**: Yes

**Tips**:
- Great for bass and melodic lines
- Adjust excitation for brightness
- Use envelope for dynamics
- Very CPU-efficient

### karplus
**Simple Karplus-Strong**

Simplified version of karplusstrong.

**Key controls**:
- **feedback**: String sustain
- **filter**: Tone control

**Sound character**: Simple plucked sounds
**CPU usage**: Very low

## FM Synthesis

### fmsynth
**FM synthesis**

Classic FM synthesis with multiple operators.

**Key controls**:
- **harmonicity**: Operator frequency ratios
- **mod idx**: Modulation index (brightness)
- **a/d/s/r**: Envelope controls
- **vol**: Volume

**Sound character**: Bells, brass, electric piano, bass
**CPU usage**: Low-Medium
**Polyphony**: Yes

**Tips**:
- Adjust mod idx for brightness
- Higher harmonicity = more metallic
- Great for bells and brass
- Experiment with ratios

## Sample Playback

### sampler
**Sample player**

Plays audio samples triggered by notes.

**Key controls**:
- **edit**: Sample editor
- **root**: Root pitch of sample
- **a/d/s/r**: Envelope
- **speed**: Playback speed
- **shuffle**: Random sample selection

**Sound character**: Any (depends on samples)
**CPU usage**: Low-Medium
**Polyphony**: Yes

**Tips**:
- Drag audio files onto module
- Set root pitch to match sample
- Use envelope to shape sound
- Great for realistic instruments

### drumplayer
**Drum sample player**

Specialized sample player for drums.

**Key controls**:
- **vol**: Volume per pad
- **speed**: Playback speed per pad
- **pan**: Pan per pad
- **individual outs**: Separate outputs

**Sound character**: Drums, percussion
**CPU usage**: Low
**Polyphony**: Yes

**Tips**:
- Load drum samples via right-click
- Each pitch triggers different sample
- Use with drumsequencer
- Individual outputs for mixing

### sampleplayer
**Advanced sample player**

More advanced sample playback with looping.

**Key controls**:
- **sample**: Sample selection
- **loop**: Loop mode
- **start/end**: Loop points
- **speed**: Playback speed

**Sound character**: Any
**CPU usage**: Low-Medium

## Subtractive Synthesis

### oscillator
**Basic waveform oscillator**

Simple oscillator with basic waveforms.

**Key controls**:
- **type**: Sine, saw, square, triangle
- **mult**: Frequency multiplier
- **detune**: Detuning
- **pw**: Pulse width (square wave)
- **sync**: Hard sync

**Sound character**: Basic waveforms
**CPU usage**: Very low
**Polyphony**: Yes

**Tips**:
- Building block for synthesis
- Combine multiple oscillators
- Use with filters
- Very CPU-efficient

### signalgenerator
**Signal generator**

Generates test signals and waveforms.

**Key controls**:
- **type**: Waveform type
- **freq**: Frequency
- **volume**: Output level

**Sound character**: Test tones
**CPU usage**: Very low

**Common uses**: Testing, drones, LFO audio rate

## Specialized Synths

### beats
**Drum synthesizer**

Synthesizes drum sounds.

**Key controls**:
- **type**: Kick, snare, hat, etc.
- **tone**: Tone control
- **decay**: Decay time
- **noise**: Noise amount

**Sound character**: Synthesized drums
**CPU usage**: Low

**Tips**:
- No samples needed
- Adjustable drum sounds
- Great for electronic drums
- CPU-efficient

### vocoder
**Vocoder effect**

Classic vocoder effect.

**Key controls**:
- **carrier**: Carrier input
- **modulator**: Modulator input (usually voice)
- **bands**: Number of bands

**Sound character**: Robot voice, talking synths
**CPU usage**: Medium-High

**Common uses**: Vocal effects, talking instruments

## Polyphony

Most synths support polyphony (multiple simultaneous notes).

**Polyphony controls**:
- **voices**: Number of simultaneous voices
- **voicelimit**: Maximum voices

**Tips**:
- More voices = more CPU usage
- Reduce voices if CPU is high
- Some synths are monophonic by default

## Common Workflows

### Basic Synth
```
notesequencer → karplusstrong → output
```

### Layered Synths
```
notesequencer → [karplusstrong + fmsynth] → output
```

### Drums
```
drumsequencer → drumplayer → output
```

### Textural
```
notesequencer → sampler → granulator (effect) → reverb → output
```

### FM Bass
```
notesequencer → fmsynth → compressor → output
```

## Sound Design Tips

### Plucked Sounds
- Use karplusstrong
- Short decay for plucks
- Longer decay for sustain
- Adjust excitation for brightness

### Pads
- Use fmsynth or sampler
- Long attack and release
- Add reverb
- Layer multiple synths
- Use granulator effect for texture

### Bass
- Use karplusstrong or fmsynth
- Low harmonicity for FM
- High feedback for Karplus
- Add subtle distortion

### Bells
- Use fmsynth
- High harmonicity
- Short decay
- Experiment with ratios

### Drums
- Use beats or drumplayer
- beats for synthesized
- drumplayer for samples
- Layer for thickness

## CPU Optimization

**Low CPU synths**:
- karplusstrong
- oscillator
- beats

**Medium CPU synths**:
- fmsynth
- sampler

**High CPU synths**:
- vocoder

**Tips**:
- Reduce polyphony
- Use simpler synths
- Freeze/bounce to audio
- Delete unused voices

## Next Steps

- **[Audio Effects](audio-effects.md)** - Process synth output
- **[Modulators](modulators.md)** - Modulate synth parameters
- **[Instruments](instruments.md)** - Generate notes for synths

