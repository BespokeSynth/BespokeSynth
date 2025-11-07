# Audio Effects

Modules that process audio signals.

## Overview

Audio effects process audio signals (cyan cables) to shape, enhance, or transform sound. They sit between synths and the output module.

## Effect Chain

### effectchain
**Chain multiple effects**

Container for chaining multiple effects together.

**Key controls**:
- **add effect**: Add effect to chain
- **effects list**: Reorder, enable/disable effects
- **dry/wet**: Mix control

**Common uses**: Building effect chains, organizing effects

**Tips**:
- Drag effects to reorder
- Disable effects to bypass
- Save as prefab for reuse
- Most versatile effect module

## Time-Based Effects

### delay
**Echo/delay effect**

Classic delay/echo effect.

**Key controls**:
- **delay**: Delay time (ms or musical)
- **feedback**: Number of repeats
- **dry/wet**: Mix control

**CPU usage**: Low
**Common uses**: Echo, doubling, rhythmic delays

**Tips**:
- Use musical timing (1/4, 1/8, etc.)
- Low feedback for slapback
- High feedback for infinite delays
- Combine with reverb

### freeverb
**Reverb effect**

Spacious reverb effect based on Freeverb algorithm.

**Key controls**:
- **roomsize**: Size of space
- **damping**: High-frequency absorption
- **dry/wet**: Mix control
- **width**: Stereo width

**CPU usage**: Medium
**Common uses**: Ambience, space, depth

**Tips**:
- Start with small room size
- Increase damping for natural sound
- Use subtly for realism
- Great on pads and leads

## Dynamics

### compressor
**Dynamic range compressor**

Reduces dynamic range, evening out levels.

**Key controls**:
- **threshold**: Level where compression starts
- **ratio**: Amount of compression
- **attack**: How fast compression engages
- **release**: How fast compression releases
- **makeup**: Output gain

**CPU usage**: Low
**Common uses**: Leveling, punch, glue

**Tips**:
- Use on drums for punch
- Gentle compression for glue
- Fast attack for transients
- Slow attack for sustain

### gate
**Noise gate**

Silences audio below threshold.

**Key controls**:
- **threshold**: Gate threshold
- **attack/release**: Gate timing

**CPU usage**: Very low
**Common uses**: Noise reduction, rhythmic gating

### pumper
**Sidechain-style pumping**

Creates pumping/ducking effect.

**Key controls**:
- **speed**: Pump speed
- **amount**: Pump depth

**CPU usage**: Low
**Common uses**: EDM pumping, rhythmic movement

## Distortion & Saturation

### distortion
**Distortion/overdrive**

Adds harmonic distortion and saturation.

**Key controls**:
- **amount**: Distortion amount
- **type**: Distortion type
- **dry/wet**: Mix control

**CPU usage**: Low
**Common uses**: Warmth, aggression, character

**Tips**:
- Subtle for warmth
- Heavy for aggression
- Use after compression
- Great on bass and leads

### bitcrush
**Bit reduction and sample rate reduction**

Lo-fi digital degradation.

**Key controls**:
- **samplerate**: Sample rate reduction
- **bits**: Bit depth reduction

**CPU usage**: Very low
**Common uses**: Lo-fi effects, digital degradation

**Tips**:
- Subtle for character
- Extreme for glitchy sounds
- Combine with distortion
- Great for drums

## Filtering

### biquad
**Biquad filter**

Versatile digital filter.

**Key controls**:
- **type**: Lowpass, highpass, bandpass, notch
- **freq**: Cutoff frequency
- **q**: Resonance

**CPU usage**: Very low
**Common uses**: Tone shaping, subtractive synthesis

### butterworth
**Butterworth filter**

Smooth filter with flat passband.

**Key controls**:
- **freq**: Cutoff frequency
- **order**: Filter steepness

**CPU usage**: Low
**Common uses**: Clean filtering, tone shaping

### dcremover
**DC offset removal**

Removes DC offset from audio.

**CPU usage**: Very low
**Common uses**: Cleaning up audio, preventing clicks

## EQ

### basiceq
**Basic equalizer**

Simple 3-band EQ.

**Key controls**:
- **low**: Low frequency gain
- **mid**: Mid frequency gain
- **high**: High frequency gain

**CPU usage**: Low
**Common uses**: Tone shaping, mixing

**Tips**:
- Cut before boost
- Subtle adjustments
- Use on individual tracks
- Great for mixing

## Modulation Effects

### tremolo
**Amplitude modulation**

Rhythmic volume modulation.

**Key controls**:
- **rate**: Modulation speed
- **amount**: Modulation depth

**CPU usage**: Very low
**Common uses**: Rhythmic movement, vintage effects

### chorus
**Chorus effect**

Thickens sound with pitch-modulated delays.

**Key controls**:
- **rate**: LFO speed
- **depth**: Modulation depth
- **feedback**: Feedback amount

**CPU usage**: Low
**Common uses**: Thickness, width, movement

### phaser
**Phaser effect**

Sweeping notch filter effect.

**Key controls**:
- **rate**: LFO speed
- **depth**: Sweep depth
- **feedback**: Resonance

**CPU usage**: Low
**Common uses**: Sweeping sounds, movement

## Granular

### granulator
**Granular processor**

Granular audio processing.

**Key controls**:
- **position**: Playback position
- **grain size**: Grain length
- **spacing**: Grain spacing
- **speed**: Playback speed

**CPU usage**: High
**Common uses**: Textures, time-stretching, effects

**Tips**:
- CPU-intensive
- Great for pads
- Experiment with grain size
- Can freeze audio

### loopergranulator
**Looping granulator**

Granular processor with looping.

**Key controls**:
- **record**: Record loop
- **position**: Playback position
- **grain size**: Grain length

**CPU usage**: High
**Common uses**: Live looping, textures

## Pitch Effects

### pitchshift
**Pitch shifter**

Shifts pitch without changing speed.

**Key controls**:
- **shift**: Pitch shift amount (semitones)
- **ratio**: Pitch ratio

**CPU usage**: Medium
**Common uses**: Harmonization, pitch correction, effects

**Tips**:
- Use for harmonies
- Subtle shifts for thickness
- Extreme shifts for effects
- Can introduce artifacts

## Utility

### gainstage
**Gain/volume control**

Simple volume control.

**Key controls**:
- **gain**: Volume level

**CPU usage**: Very low
**Common uses**: Level adjustment, mixing

### muter
**Mute switch**

Mutes audio signal.

**Key controls**:
- **mute**: On/off

**CPU usage**: Very low
**Common uses**: Muting tracks, performance

### noisify
**Add noise**

Adds noise to signal.

**Key controls**:
- **amount**: Noise level
- **type**: Noise type

**CPU usage**: Very low
**Common uses**: Texture, lo-fi effects

## Effect Chains

### Typical Effect Order

**General guideline**:
```
Source → EQ → Compression → Distortion → Modulation → Delay → Reverb → Output
```

**Why this order?**:
1. **EQ first**: Shape tone before processing
2. **Compression**: Even out dynamics
3. **Distortion**: Add harmonics
4. **Modulation**: Add movement
5. **Delay**: Rhythmic repeats
6. **Reverb last**: Space and ambience

### Common Effect Chains

**Vocal chain**:
```
synth → compressor → eq → delay → reverb → output
```

**Guitar chain**:
```
synth → distortion → delay → reverb → output
```

**Drum bus**:
```
drums → compressor → eq → output
```

**Pad chain**:
```
synth → chorus → delay → reverb → output
```

**Bass chain**:
```
synth → compressor → distortion → eq → output
```

## Tips & Best Practices

### Mixing Tips
1. **Less is more**: Don't over-process
2. **Use subtly**: Extreme settings rarely sound good
3. **EQ before compression**: Shape tone first
4. **Reverb last**: Add space at the end
5. **Save CPU**: Use effects efficiently

### Creative Tips
1. **Experiment**: Try unusual effect orders
2. **Automate**: Modulate effect parameters
3. **Parallel processing**: Blend wet/dry
4. **Layer effects**: Multiple subtle effects
5. **Feedback loops**: Use carefully for textures

### CPU Optimization
1. **Use effectchain**: More efficient than separate modules
2. **Disable unused effects**: Save CPU
3. **Avoid granulator**: Unless needed (CPU-heavy)
4. **Freeze tracks**: Render to audio
5. **Simplify chains**: Remove unnecessary effects

## Next Steps

- **[Modulators](modulators.md)** - Modulate effect parameters
- **[Synths](synths.md)** - Generate audio to process
- **[Advanced Topics](../advanced/README.md)** - Advanced techniques

