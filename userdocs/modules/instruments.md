# Instruments

Modules that generate note sequences and handle MIDI input.

## Overview

Instrument modules create note messages (orange cables) that feed into synths or note effects. They're the starting point for most musical patches.

## Core Sequencers

### notesequencer
**Step sequencer for melodies**

The most commonly used sequencer in BespokeSynth. Create melodic patterns with adjustable pitch, velocity, and length.

**Key controls**:
- **grid**: Click to add/remove notes
- **length**: Number of steps in sequence
- **interval**: Note duration (1n, 2n, 4n, 8n, etc.)
- **octave**: Base octave for sequence
- **velocity**: Note velocity
- **notemode**: Chromatic, scale, pentatonic, or fifths

**Tips**:
- Shift+drag on grid to adjust velocity
- Use arrow keys to move notes
- Right-click for more options
- Accepts pulse input to trigger

### drumsequencer
**Step sequencer for drums**

Multi-row sequencer designed for drum programming. Each row outputs a different pitch.

**Key controls**:
- **grid**: Click to add/remove steps
- **measures**: Length in measures
- **step**: Step duration (4n, 8n, 16n, etc.)
- **rowpitch**: Output pitch for each row
- **offset**: Timing offset per row (humanization)
- **randomize**: Generate random patterns

**Tips**:
- Shift+drag to adjust step velocity
- Use offset to add groove
- Lock rows before randomizing to keep parts
- Connect grid controller for hands-on control

### notecanvas
**Piano roll editor**

Visual piano roll for drawing complex melodies and chords.

**Key controls**:
- **Shift+click**: Add note
- **Drag**: Move note
- **Drag edges**: Resize note
- **Alt+drag**: No grid snapping
- **Delete**: Remove note

**Tips**:
- Zoom in for precision editing
- Great for complex melodies
- Can draw chords
- Supports velocity editing

## MIDI Input

### midicontroller
**MIDI input device**

Receive MIDI from external controllers or DAWs.

**Key controls**:
- **controller**: Select MIDI device
- **channel**: MIDI channel (or "all")
- **bind**: Map MIDI CC to parameters
- **mappings**: View/edit CC mappings

**Tips**:
- Enable "bind (hold shift)" to map controls
- Hover over target, hold Shift, move MIDI control
- Supports MPE (MIDI Polyphonic Expression)
- Can output to multiple destinations

### keyboarddisplay
**Virtual keyboard**

On-screen keyboard for mouse/computer keyboard input.

**Key controls**:
- **octave**: Current octave
- **latch**: Hold notes
- **velocity**: Note velocity

**Tips**:
- Click keys to play
- Use computer keyboard (QWERTY as piano keys)
- Great for testing patches
- Can be used for performance

## Advanced Sequencers

### circlesequencer
**Polyrhythmic circular sequencer**

Displays sequences as concentric circles, each with independent length.

**Key controls**:
- **length**: Steps per ring
- **note**: Pitch for each ring
- **offset**: Timing offset per ring

**Tips**:
- Create polyrhythms easily
- Visual representation of rhythm
- Great for generative music

### euclideansequencer
**Euclidean rhythm generator**

Generates rhythms using Euclidean algorithm - evenly distributes hits across steps.

**Key controls**:
- **length**: Total steps
- **hits**: Number of hits to distribute
- **offset**: Rotate pattern

**Tips**:
- Creates musically interesting rhythms
- Common in electronic music
- Try 3 hits in 8 steps, 5 in 8, etc.

### polyrhythms
**Multiple independent rhythms**

Create complex polyrhythmic patterns with multiple voices.

**Key controls**:
- **rhythm**: Pattern for each voice
- **pitch**: Note for each voice
- **division**: Time division per voice

**Tips**:
- Layer different time signatures
- Great for experimental music
- Combine with euclidean patterns

## Note Generators

### randomnote
**Random note generator**

Generates random notes within specified range.

**Key controls**:
- **pitch**: Base pitch
- **range**: Pitch range
- **interval**: Note timing
- **probability**: Chance of note

**Tips**:
- Use with quantizer for musical randomness
- Great for generative patches
- Combine with notecounter for patterns

### notecreator
**Manual note trigger**

Trigger notes manually or via pulse input.

**Key controls**:
- **pitch**: Note pitch
- **velocity**: Note velocity
- **duration**: Note length
- **trigger**: Manual trigger button

**Tips**:
- Connect pulse to trigger input
- Use for testing synths
- Good for performance patches

### notecounter
**Stepped note generator**

Outputs incrementing/decrementing notes.

**Key controls**:
- **start**: Starting pitch
- **end**: Ending pitch
- **step**: Increment amount
- **direction**: Up, down, or ping-pong
- **random**: Randomize order

**Tips**:
- Connect pulser to trigger
- Creates arpeggios and sequences
- Use with scale module for musical results

## Specialized Sequencers

### m185sequencer
**185-style sequencer**

Inspired by hardware sequencer, creates evolving patterns.

**Key controls**:
- **pattern**: Sequence pattern
- **transpose**: Transpose amount
- **length**: Pattern length

**Tips**:
- Great for generative music
- Creates evolving sequences
- Experiment with different patterns

### playsequencer
**Playback sequencer**

Plays back recorded note sequences.

**Key controls**:
- **record**: Record incoming notes
- **play**: Playback recorded notes
- **clear**: Clear recording

**Tips**:
- Record MIDI performances
- Loop recorded phrases
- Edit after recording

### slidersequencer
**Slider-based sequencer**

Sequence using sliders instead of grid.

**Key controls**:
- **sliders**: One per step
- **length**: Number of steps
- **interval**: Step timing

**Tips**:
- Good for smooth melodic lines
- Visual representation of melody
- Easy to see contour

## Note Loopers

### notelooper
**Loop and layer notes**

Records and loops note sequences with overdubbing.

**Key controls**:
- **record**: Start/stop recording
- **clear**: Clear loop
- **speed**: Playback speed
- **commit**: Finalize loop

**Tips**:
- Build up complex patterns
- Overdub multiple layers
- Use for live performance

### notechain
**Chain note patterns**

Chains multiple note patterns in sequence.

**Key controls**:
- **patterns**: List of patterns
- **current**: Current pattern
- **next**: Advance to next pattern

**Tips**:
- Create song structures
- Arrange patterns
- Good for composition

## Grid Controllers

### gridkeyboard
**Grid-based keyboard**

Use grid controller (Launchpad, etc.) as keyboard.

**Key controls**:
- **grid**: Connect grid controller
- **layout**: Key layout
- **octave**: Base octave

**Tips**:
- Requires grid controller
- Multiple layout options
- Great for performance

## Specialized Instruments

### fouronthefloor
**Four-on-the-floor kick pattern**

Generates classic 4/4 kick drum pattern.

**Key controls**:
- **enabled**: On/off
- **pitch**: Kick pitch

**Tips**:
- Instant dance beat
- Combine with other drums
- Simple but effective

### notesinger
**Vocal-style note input**

Experimental vocal input module.

**Key controls**:
- **pitch detection**: Detect sung pitch
- **threshold**: Detection sensitivity

**Tips**:
- Requires microphone
- Experimental feature
- Use with quantizer

## Common Workflows

### Basic Melody
```
notesequencer → synth → output
```

### Drums
```
drumsequencer → drumplayer → output
```

### MIDI Performance
```
midicontroller → synth → effects → output
```

### Generative
```
pulser → notecounter → quantizer → synth → output
```

### Polyrhythmic
```
circlesequencer → synth → output
euclideansequencer → synth → output
```

## Tips and Best Practices

### Sequencer Tips
1. **Start simple**: Use notesequencer for basic melodies
2. **Use scale mode**: Ensures notes are in key
3. **Adjust interval**: Match tempo and feel
4. **Layer sequences**: Multiple sequencers for complexity
5. **Save patterns**: Use snapshots for variations

### MIDI Tips
1. **Map controls**: Use MIDI CC for parameter control
2. **Check channel**: Ensure MIDI channel matches
3. **Test connection**: Use keyboarddisplay to verify
4. **Save mappings**: Document your MIDI setup
5. **Use MPE**: For expressive controllers

### Performance Tips
1. **Use snapshots**: Quick preset changes
2. **Map to controller**: Hands-on control
3. **Keep it simple**: Complex patches are hard to perform
4. **Practice**: Test before performing
5. **Have backup**: Save multiple versions

## Related Modules

**Note Effects**: Process instrument output
- arpeggiator, chorder, notedelayer

**Synths**: Generate sound from notes
- karplusstrong, fmsynth, sampler

**Pulse**: Trigger instruments
- pulser, pulsegate, pulsetrain

**Modulators**: Modulate instrument parameters
- lfo, envelope, expression

## Next Steps

- **[Note Effects](note-effects.md)** - Process note sequences
- **[Synths](synths.md)** - Generate sound from notes
- **[Pulse](pulse.md)** - Trigger and timing

