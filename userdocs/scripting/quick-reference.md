# Python Scripting Quick Reference

Quick reference card for BespokeSynth Python scripting.

## Callback Functions

```python
def on_pulse():
    """Called when pulse received"""
    pass

def on_note(pitch, velocity):
    """Called when note received (0-127)"""
    pass

def on_grid_button(col, row, velocity):
    """Called when grid button pressed"""
    pass

def on_osc(message):
    """Called when OSC message received"""
    pass

def on_midi(messageType, control, value, channel):
    """Called when MIDI received"""
    pass

def on_sysex(data):
    """Called when SysEx received"""
    pass
```

## The `me` Object

### Note Output

```python
# Play note with auto note-off
me.play_note(pitch, velocity, length=1.0/16, pan=0, output_index=0)

# Send note message (no auto note-off)
me.note_msg(pitch, velocity, pan=0, output_index=0)

# Schedule note
me.schedule_note(delay, pitch, velocity, length=1.0/16, pan=0, output_index=0)

# Schedule note message
me.schedule_note_msg(delay, pitch, velocity, pan=0, output_index=0)
```

### Scheduling

```python
# Schedule function call
me.schedule_call(delay, "function_name()")

# Example
def my_func():
    print("Called!")
me.schedule_call(1.0/4, "my_func()")
```

### Parameters

```python
# Set parameter
me.set(path, value)

# Get parameter
value = me.get(path)

# Set text
me.set_text(path, text)

# Get text
text = me.get_text(path)

# Schedule parameter change
me.schedule_set(delay, path, value)

# Adjust parameter
me.adjust(path, amount)
```

### MIDI

```python
# Send CC
me.send_cc(control, value, output_index=0)
```

### Utility

```python
# Print to console
me.output(obj)

# Stop script
me.stop()

# Get self reference
module = me.me()

# Get caller
caller = me.get_caller()

# Set output count
me.set_num_note_outputs(num)

# Connect OSC
me.connect_osc_input(port)
```

## The `bespoke` Module

### Timing

```python
# Get measure time (float)
time = bespoke.get_measure_time()

# Get measure (int)
measure = bespoke.get_measure()

# Get current step
step = bespoke.get_step(subdivision)

# Count per measure
count = bespoke.count_per_measure(subdivision)

# Time until subdivision
delay = bespoke.time_until_subdivision(subdivision)

# Get time signature ratio
ratio = bespoke.get_time_sig_ratio()

# Get tempo
bpm = bespoke.get_tempo()

# Reset transport
bespoke.reset_transport()
```

### Scale & Pitch

```python
# Get root note
root = bespoke.get_root()

# Get scale intervals
scale = bespoke.get_scale()

# Get scale range
notes = bespoke.get_scale_range(octave, count)

# Tone to pitch
pitch = bespoke.tone_to_pitch(index)

# Name to pitch
pitch = bespoke.name_to_pitch("C4")

# Pitch to name
name = bespoke.pitch_to_name(60)

# Pitch to frequency
freq = bespoke.pitch_to_freq(60)
```

### Utility

```python
# Deterministic random
value = bespoke.random(seed, index)

# Background text
bespoke.set_background_text(str, size=50, xPos=150, yPos=250, 
                            red=1, green=1, blue=1)

# Get all modules
modules = bespoke.get_modules()

# Get module controls
controls = bespoke.get_controls(path)

# Location management
bespoke.location_recall(location)
bespoke.location_store(location)
```

## Module APIs

### Generic Module

```python
# Get module
mod = module.get(path)

# Create module
mod = module.create(moduleType, x, y)

# Module methods
mod.set_position(x, y)
x = mod.get_position_x()
y = mod.get_position_y()
w = mod.get_width()
h = mod.get_height()
mod.set_target(target)
target = mod.get_target()
targets = mod.get_targets()
mod.set_name(name)
mod.delete()
mod.set(path, value)
value = mod.get(path)
mod.adjust(path, amount)
mod.set_focus(zoom)
```

### Note Sequencer

```python
seq = notesequencer.get(path)
seq.set_step(step, row, velocity=127, length=1.0)
seq.set_pitch(step, pitch, velocity=127, length=1.0)
```

### Drum Sequencer

```python
drums = drumsequencer.get(path)
drums.set(step, pitch, velocity)
vel = drums.get(step, pitch)
```

### Bassline Sequencer

```python
bass = basslinesequencer.get(path)
bass.set_step(step, tone, velocity, tie)

# Velocity types
basslinesequencer.StepVelocityType.Off
basslinesequencer.StepVelocityType.Ghost
basslinesequencer.StepVelocityType.Normal
basslinesequencer.StepVelocityType.Accent
```

### Note Canvas

```python
canvas = notecanvas.get(path)
canvas.add_note(measurePos, pitch, velocity, length)
canvas.clear()
canvas.fit()
```

### Sample Player

```python
sampler = sampleplayer.get(path)
sampler.set_cue_point(pitch, startSeconds, lengthSeconds, speed)
sampler.fill(data)
sampler.play_cue(cue, speedMult=1, startOffsetSeconds=0)
length = sampler.get_length_seconds()
```

### Grid

```python
g = grid.get(path)
g.set(col, row, value)
value = g.get(col, row)
g.set_grid(cols, rows)
g.set_label(row, label)
g.set_color(colorIndex, r, g, b)
g.highlight_cell(col, row, delay, duration, colorIndex=1)
g.set_division(division)
g.set_momentary(momentary)
g.set_cell_color(col, row, colorIndex)
colorIndex = g.get_cell_color(col, row)
g.add_listener(script)
g.clear()
```

### MIDI Controller

```python
controller = midicontroller.get(path)
controller.set_connection(messageType, control, controlPath, ...)
controller.send_note(pitch, velocity, forceNoteOn=False, channel=-1, page=0)
controller.send_cc(ctl, value, channel=-1, page=0)
controller.send_program_change(program, channel=-1, page=0)
controller.send_pitchbend(bend, channel=-1, page=0)
controller.send_data(a, b, c, page=0)
controller.send_sysex(data, page=0)
controller.add_script_listener(script)
controller.resync_controller_state()

# Message types
midicontroller.Note
midicontroller.Control
midicontroller.Program
midicontroller.PitchBend
```

### OSC

```python
# OSC Controller
osc = osccontroller.get(path)
osc.add_control(address, isFloat)

# OSC Output
out = oscoutput.get(path)
out.send_float(address, val)
out.send_int(address, val)
out.send_string(address, val)
```

### Envelope

```python
env = envelope.get(path)
env.start([[time, value], ...])
env.schedule(delay, [[time, value], ...])
```

### Snapshots

```python
snaps = snapshots.get(path)
size = snaps.get_size()
current = snaps.get_current_snapshot()
exists = snaps.has_snapshot(index)
snaps.set_snapshot(index)
snaps.store_snapshot(index, label="")
snaps.delete_snapshot(index)
```

## Common Patterns

### Random Scale Notes

```python
import random

def on_pulse():
    scale = bespoke.get_scale_range(4, 8)
    pitch = random.choice(scale)
    me.play_note(pitch, 100, 1.0/8)
```

### Simple Sequencer

```python
notes = [60, 62, 64, 65]
step = 0

def on_pulse():
    global step
    me.play_note(notes[step], 100, 1.0/8)
    step = (step + 1) % len(notes)
```

### Note Echo

```python
def on_note(pitch, velocity):
    me.play_note(pitch, velocity, 1.0/4)
    me.schedule_note(1.0/4, pitch, velocity * 0.7, 1.0/4)
```

### LFO Automation

```python
import math

def on_pulse():
    time = bespoke.get_measure_time()
    lfo = (math.sin(time * math.pi * 2) + 1) / 2
    me.set("filter~cutoff", lfo)
```

### Euclidean Rhythm

```python
def euclidean(steps, pulses):
    pattern = []
    bucket = 0
    for i in range(steps):
        bucket += pulses
        if bucket >= steps:
            bucket -= steps
            pattern.append(1)
        else:
            pattern.append(0)
    return pattern

pattern = euclidean(16, 5)
step = 0

def on_pulse():
    global step
    if pattern[step]:
        me.play_note(60, 100, 1.0/16)
    step = (step + 1) % 16
```

## Tips

- Use `global` keyword to modify variables outside functions
- Note durations are in measures (1.0 = 1 measure, 1.0/4 = quarter note)
- MIDI pitch 60 = middle C
- Velocity range: 0-127
- Pan range: -1 (left) to 1 (right)
- Use `print()` or `me.output()` for debugging
- Click **run** button after editing script
- Save your patch frequently

## See Also

- [Getting Started](getting-started.md) - Step-by-step tutorial
- [API Reference](api-reference.md) - Complete documentation
- [Examples](examples.md) - More examples
