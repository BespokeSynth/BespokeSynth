# Pulse Modules

Modules for timing, triggers, and gate signals.

## Overview

Pulse modules work with trigger/gate signals (yellow cables). They're used for timing, triggering events, and creating rhythmic patterns.

## Pulse Generators

### pulser
**Regular pulse generator**

Generates pulses at regular intervals.

**Key controls**:
- **interval**: Time between pulses (musical or ms)
- **offset**: Phase offset
- **enabled**: On/off

**Common uses**: Triggering sequencers, clock source, rhythmic triggers

**Tips**:
- Use musical intervals (1/4, 1/8, 1/16)
- Offset for polyrhythms
- Connect to sequencer triggers
- Foundation of generative patches

### pulsetrain
**Burst of pulses**

Generates a burst of rapid pulses.

**Key controls**:
- **count**: Number of pulses in burst
- **interval**: Time between pulses
- **trigger**: Trigger burst

**Common uses**: Fills, rolls, rapid triggers

**Tips**:
- Great for drum fills
- Use for ratcheting effects
- Trigger with another pulse
- Adjust interval for speed

### pulsesequence
**Sequenced pulses**

Sequence of pulses with individual timing.

**Key controls**:
- **steps**: Pulse pattern
- **interval**: Base timing

**Common uses**: Complex rhythms, euclidean patterns

## Pulse Processing

### pulsegate
**Gate pulse signals**

Allows or blocks pulses.

**Key controls**:
- **gate**: Open/closed
- **probability**: Chance of passing

**Common uses**: Conditional triggering, probability, muting

**Tips**:
- Use probability for variation
- Gate for muting sections
- Combine with pulser for patterns
- Great for generative music

### pulsedelayer
**Delay pulses**

Delays pulse signals.

**Key controls**:
- **delay**: Delay time
- **feedback**: Number of repeats

**Common uses**: Rhythmic delays, echoes, polyrhythms

**Tips**:
- Create rhythmic complexity
- Use feedback for repeats
- Musical timing for sync
- Layer with original

### pulsehocket
**Distribute pulses**

Alternates pulses between outputs.

**Key controls**:
- **outputs**: Number of outputs
- **pattern**: Distribution pattern

**Common uses**: Alternating triggers, polyrhythms

### pulserouter
**Route pulses**

Routes pulses to different outputs.

**Key controls**:
- **outputs**: Number of outputs
- **routing**: Routing method

**Common uses**: Conditional routing, multiple destinations

## Pulse Utilities

### pulsedisplayer
**Display pulses**

Visualizes pulse signals.

**Common uses**: Debugging, monitoring, learning

### pulsebutton
**Manual pulse trigger**

Manually trigger pulses.

**Key controls**:
- **trigger**: Trigger button

**Common uses**: Manual triggering, testing, performance

**Tips**:
- Click to trigger
- Map to MIDI controller
- Use for testing patches
- Performance tool

### pulseflag
**Pulse flag/latch**

Latches on pulse, resets on another.

**Key controls**:
- **set**: Set flag
- **reset**: Reset flag

**Common uses**: State control, logic, gating

### pulselimit
**Limit pulse rate**

Limits how fast pulses can pass.

**Key controls**:
- **min interval**: Minimum time between pulses

**Common uses**: Rate limiting, preventing overload

## Pulse Converters

### notetopulse
**Convert notes to pulses**

Converts note messages to pulse triggers.

**Key controls**:
- **mode**: Note on, note off, or both

**Common uses**: Triggering from notes, note-driven pulses

**Tips**:
- Trigger sequencers from notes
- Use note velocity for pulse strength
- Great for note-driven generative patches

### audiotopulse
**Convert audio to pulses**

Converts audio transients to pulses.

**Key controls**:
- **threshold**: Detection threshold
- **sensitivity**: Detection sensitivity

**Common uses**: Beat detection, audio-reactive triggers

**Tips**:
- Detect drum hits
- Audio-reactive visuals
- Trigger from audio
- Adjust threshold carefully

### boundstopulse
**Convert bounds to pulses**

Triggers pulse when value crosses boundary.

**Key controls**:
- **threshold**: Boundary value
- **mode**: Rising, falling, or both

**Common uses**: Threshold triggering, logic

## Pulse Logic

### pulsechance
**Probabilistic pulse gate**

Randomly passes or blocks pulses.

**Key controls**:
- **probability**: Chance of passing (0-100%)

**Common uses**: Variation, generative music, randomness

**Tips**:
- Add variation to patterns
- 50% for half-time feel
- Low values for sparse patterns
- High values for busy patterns

### pulsecounter
**Count pulses**

Counts incoming pulses.

**Key controls**:
- **count**: Current count
- **reset**: Reset counter

**Common uses**: Sequencing, logic, patterns

## Common Workflows

### Basic Clock
```
pulser (1/16) → notesequencer → synth → output
```

### Polyrhythm
```
pulser (1/4) → sequencer1 → synth1
pulser (1/6) → sequencer2 → synth2
```

### Probability Pattern
```
pulser → pulsechance (70%) → notesequencer → synth
```

### Audio-Reactive
```
drums → audiotopulse → notesequencer → synth
```

### Delayed Triggers
```
pulser → pulsedelayer → notesequencer → synth
```

## Pulse Patterns

### Euclidean Rhythms
Use euclideansequencer for mathematically distributed pulses:
- 3 hits in 8 steps = [X..X..X.]
- 5 hits in 8 steps = [X.X.X.X.X]
- 7 hits in 16 steps = [X.X.X.X.X.X.X...]

### Polyrhythms
Layer pulsers with different intervals:
- Pulser 1: 1/4 (quarter notes)
- Pulser 2: 1/6 (triplets)
- Pulser 3: 1/5 (quintuplets)

### Probability Patterns
Use pulsechance for variation:
- 100%: Every pulse passes
- 75%: Most pulses pass
- 50%: Half pulses pass
- 25%: Sparse pattern

## Tips & Best Practices

### Timing
1. **Use musical intervals**: Sync to tempo
2. **Offset for groove**: Add swing/shuffle
3. **Layer rhythms**: Polyrhythms for complexity
4. **Test timing**: Listen carefully
5. **Keep it simple**: Start with basic patterns

### Generative Music
1. **Use probability**: Add variation
2. **Layer pulsers**: Multiple clock sources
3. **Delay pulses**: Create echoes
4. **Random elements**: Pulsechance for unpredictability
5. **Feedback loops**: Pulses triggering pulses

### Performance
1. **Manual triggers**: Pulsebutton for control
2. **Gate sections**: Pulsegate for muting
3. **Map to controller**: MIDI control of gates
4. **Use snapshots**: Save pulse configurations
5. **Keep visual**: Pulsedisplayer for monitoring

## Creative Techniques

### Generative Triggers
```
pulser → pulsechance → notecounter → notesequencer → synth
```

### Rhythmic Complexity
```
pulser → [pulsedelayer1, pulsedelayer2, pulsedelayer3] → sequencer
```

### Audio-Reactive Sequencing
```
drums → audiotopulse → randomnote → synth
```

### Polyrhythmic Layers
```
pulser (1/4) → sequencer1 → synth1
pulser (1/6) → sequencer2 → synth2
pulser (1/5) → sequencer3 → synth3
```

## Next Steps

- **[Instruments](instruments.md)** - Trigger instruments with pulses
- **[Modulators](modulators.md)** - Pulse-triggered modulation
- **[Scripting](../scripting/README.md)** - Advanced pulse logic

