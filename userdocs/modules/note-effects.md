# Note Effects

Modules that process and transform note messages.

## Overview

Note effects sit between instruments and synths, transforming note messages (orange cables) in creative ways. They can add harmony, change timing, filter notes, and more.

## Harmony & Chords

### arpeggiator
**Arpeggiate chords into sequences**

Breaks chords into individual notes played in sequence.

**Key controls**:
- **interval**: Speed of arpeggio (4n, 8n, 16n, etc.)
- **arpmode**: Up, down, up/down, random, etc.
- **octaves**: Number of octaves to span
- **hold**: Sustain notes

**Common uses**: Synth arpeggios, rhythmic patterns, chord expansion

### chorder
**Add harmony to single notes**

Automatically adds chord notes to incoming notes.

**Key controls**:
- **chord**: Chord type (major, minor, 7th, etc.)
- **inversion**: Chord inversion
- **voicing**: Spread of notes

**Common uses**: Instant harmonies, chord pads, layered sounds

### chordholder
**Hold and sustain chords**

Holds chord until new chord received.

**Key controls**:
- **hold**: Enable/disable hold
- **clear**: Clear held chord

**Common uses**: Pad sounds, sustained chords, performance

## Timing & Rhythm

### notedelayer
**Delay note messages**

Delays notes by specified time.

**Key controls**:
- **delay**: Delay time
- **feedback**: Number of repeats
- **velocity decay**: Velocity reduction per repeat

**Common uses**: Echo effects, rhythmic delays, doubling

### noteecho
**Multi-tap note delay**

Multiple delayed copies of notes.

**Key controls**:
- **interval**: Time between echoes
- **echoes**: Number of repeats
- **velocity**: Velocity scaling

**Common uses**: Rhythmic patterns, fills, complexity

### noteratchet
**Repeat notes rapidly**

Repeats each note multiple times.

**Key controls**:
- **subdivision**: Repeat speed
- **repeats**: Number of repeats
- **velocity**: Velocity curve

**Common uses**: Trap-style hi-hats, fills, energy

### notehocket
**Distribute notes between outputs**

Alternates notes between multiple outputs.

**Key controls**:
- **outputs**: Number of outputs
- **pattern**: Distribution pattern

**Common uses**: Stereo effects, multiple synths, call-and-response

## Pitch Manipulation

### pitchbender
**Bend pitch of notes**

Adds pitch bend to notes.

**Key controls**:
- **bend**: Bend amount
- **curve**: Bend curve shape

**Common uses**: Expressive pitch, slides, vibrato

### pitchdive
**Pitch dive effect**

Notes dive down in pitch.

**Key controls**:
- **amount**: Dive depth
- **time**: Dive duration

**Common uses**: Synth drops, transitions, effects

### capo
**Transpose notes**

Shifts all notes up or down by semitones.

**Key controls**:
- **pitch**: Transpose amount

**Common uses**: Key changes, octave shifts, transposition

### noteoctaver
**Add octave doubling**

Adds notes one or more octaves up/down.

**Key controls**:
- **octave**: Octave offset
- **mix**: Blend with original

**Common uses**: Bass octaves, layered sounds, thickness

### pitchremap
**Remap pitches**

Maps input pitches to different output pitches.

**Key controls**:
- **mapping**: Pitch mapping table

**Common uses**: Custom scales, pitch correction, creative effects

## Filtering & Gating

### notegate
**Gate notes based on conditions**

Allows or blocks notes based on criteria.

**Key controls**:
- **gate**: Open/closed
- **probability**: Chance of passing

**Common uses**: Muting, probability, conditional routing

### notefilter
**Filter notes by pitch/velocity**

Filters notes based on pitch or velocity range.

**Key controls**:
- **min pitch**: Minimum pitch
- **max pitch**: Maximum pitch
- **min velocity**: Minimum velocity
- **max velocity**: Maximum velocity

**Common uses**: Range limiting, velocity filtering, splits

### noterangefilter
**Filter by pitch range**

Only passes notes within specified range.

**Key controls**:
- **min**: Minimum pitch
- **max**: Maximum pitch

**Common uses**: Keyboard splits, range limiting

### notechance
**Probabilistic note gate**

Randomly passes or blocks notes.

**Key controls**:
- **probability**: Chance of passing (0-100%)

**Common uses**: Generative music, variation, randomness

## Routing & Distribution

### noterouter
**Route notes to multiple outputs**

Sends notes to different outputs based on criteria.

**Key controls**:
- **outputs**: Number of outputs
- **routing**: Routing method (round-robin, pitch, velocity, etc.)

**Common uses**: Multi-timbral setups, layering, splits

### notepanner
**Pan notes in stereo**

Assigns pan position to notes.

**Key controls**:
- **pan**: Pan position
- **spread**: Stereo spread

**Common uses**: Stereo width, spatial effects

### notepanalternator
**Alternate notes left/right**

Alternates notes between left and right.

**Common uses**: Stereo ping-pong, width, movement

### notepanrandom
**Random pan per note**

Randomly pans each note.

**Key controls**:
- **amount**: Randomization amount

**Common uses**: Stereo width, natural feel, space

## Velocity & Expression

### velocityscaler
**Scale note velocity**

Multiplies or adjusts velocity.

**Key controls**:
- **scale**: Velocity multiplier
- **offset**: Velocity offset

**Common uses**: Dynamics control, velocity curves

### noteexpression
**Add expression to notes**

Adds MPE expression data.

**Key controls**:
- **pressure**: Pressure amount
- **timbre**: Timbre amount

**Common uses**: MPE synthesis, expression, dynamics

### notehumanizer
**Humanize timing and velocity**

Adds subtle randomization for human feel.

**Key controls**:
- **timing**: Timing randomization
- **velocity**: Velocity randomization

**Common uses**: Natural feel, groove, realism

## Quantization & Scales

### quantizer
**Quantize notes to scale**

Forces notes to nearest scale degree.

**Key controls**:
- **scale**: Uses global scale module

**Common uses**: Ensuring notes are in key, generative music

### scaledegree
**Convert scale degrees to pitches**

Interprets input as scale degrees.

**Key controls**:
- **degree**: Scale degree offset

**Common uses**: Modal playing, scale-based sequencing

### scaledetect
**Detect scale from notes**

Analyzes incoming notes to detect scale.

**Common uses**: Auto-scale detection, analysis

## MIDI & MPE

### midicc
**Send MIDI CC messages**

Generates MIDI CC from notes.

**Key controls**:
- **cc**: CC number
- **value**: CC value

**Common uses**: MIDI control, automation

### midioutput
**Send notes to external MIDI**

Outputs notes to external MIDI device.

**Key controls**:
- **device**: MIDI output device
- **channel**: MIDI channel

**Common uses**: Controlling external synths, DAW integration

### mpesmoother
**Smooth MPE data**

Smooths MPE expression data.

**Key controls**:
- **amount**: Smoothing amount

**Common uses**: MPE synthesis, expression control

### mpetweaker
**Adjust MPE parameters**

Modifies MPE expression values.

**Key controls**:
- **pressure**: Pressure adjustment
- **timbre**: Timbre adjustment

**Common uses**: MPE customization, expression shaping

## Utility

### notedisplayer
**Display incoming notes**

Visualizes note messages.

**Common uses**: Debugging, monitoring, learning

### notelatch
**Latch notes on/off**

Holds notes until same note received again.

**Key controls**:
- **latch**: Enable/disable

**Common uses**: Sustain, drone notes, performance

### sustainpedal
**MIDI sustain pedal**

Implements sustain pedal behavior.

**Key controls**:
- **sustain**: Pedal state

**Common uses**: Piano-style sustain, performance

### noteflusher
**Clear stuck notes**

Sends note-offs for all notes.

**Common uses**: Fixing stuck notes, reset

## Advanced

### notestepper
**Step through note sequence**

Advances through notes one at a time.

**Key controls**:
- **step**: Manual step button
- **reset**: Reset to beginning

**Common uses**: Manual sequencing, step-by-step control

### notetable
**Note lookup table**

Maps input notes to output notes via table.

**Key controls**:
- **table**: Note mapping

**Common uses**: Custom mappings, creative routing

### notestream
**Stream notes over time**

Spreads chord notes over time.

**Key controls**:
- **interval**: Time between notes
- **order**: Note order

**Common uses**: Strumming, arpeggios, texture

### notestrummer
**Strum chords**

Strums chord notes like a guitar.

**Key controls**:
- **strum time**: Duration of strum
- **direction**: Up or down

**Common uses**: Guitar-like strumming, natural chords

## Common Workflows

### Harmony
```
notesequencer → chorder → synth → output
```

### Rhythmic Delay
```
notesequencer → notedelayer → synth → output
```

### Arpeggio
```
midicontroller → arpeggiator → synth → output
```

### Generative
```
randomnote → quantizer → notedelayer → synth → output
```

### Multi-timbral
```
notesequencer → noterouter → [synth1, synth2, synth3] → output
```

## Tips

1. **Chain effects**: Combine multiple note effects
2. **Use quantizer**: Keep random notes musical
3. **Experiment with timing**: Delays and echoes create rhythm
4. **Layer harmony**: Chorder + octaver for thick sounds
5. **Add humanization**: Makes sequences feel natural

## Next Steps

- **[Instruments](instruments.md)** - Generate notes
- **[Synths](synths.md)** - Turn notes into sound
- **[Audio Effects](audio-effects.md)** - Process audio

