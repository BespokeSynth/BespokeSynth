# Python Scripting

Extend BespokeSynth with Python scripting for custom behavior and automation.

## Overview

BespokeSynth includes an embedded Python interpreter that allows you to write custom scripts to control modules, generate patterns, process data, and create entirely new behaviors.

## What Can You Do With Scripting?

- **Generative music**: Create algorithmic compositions
- **Custom sequencing**: Build unique sequencers
- **Parameter automation**: Automate any parameter
- **MIDI processing**: Custom MIDI effects
- **Data visualization**: Create custom displays
- **External control**: Interface with external systems
- **Game logic**: Build interactive music systems
- **Live coding**: Perform with code

## Getting Started

### Creating a Script Module

1. Right-click canvas
2. Select **script** from menu
3. Script module appears with code editor

### Your First Script

```python
# Simple script that prints when triggered
def on_pulse():
    print("Pulse received!")
```

**To trigger**:
1. Create a **pulser** module
2. Connect pulser to script module
3. Watch console for output

## Script Module Basics

### Code Editor

**Controls**:
- **run**: Execute script
- **stop**: Stop script
- **code editor**: Write Python code
- **resize**: Drag to resize editor

**Tips**:
- Code is saved with patch
- Syntax highlighting included
- Errors shown in red
- Use print() for debugging

### Callback Functions

Scripts respond to events via callback functions:

```python
def on_pulse():
    """Called when pulse received"""
    pass

def on_note(pitch, velocity):
    """Called when note received"""
    pass

def on_note_off(pitch):
    """Called when note released"""
    pass

def on_grid_button(col, row, velocity):
    """Called when grid button pressed"""
    pass

def on_slider(index, value):
    """Called when slider moved"""
    pass
```

## Core Concepts

### The `me` Object

Every script has access to `me` - the script module itself.

```python
# Get module name
name = me.get_name()

# Set parameter
me.set("slider1", 0.5)

# Get parameter
value = me.get("slider1")

# Schedule future event
me.schedule_call(my_function, 1000)  # Call after 1000ms
```

### The `bespoke` Object

Access to global BespokeSynth functions.

```python
# Get current time
time = bespoke.get_time()

# Get measure
measure = bespoke.get_measure()

# Get tempo
bpm = bespoke.get_tempo()

# Play note
bespoke.play_note(60, 100, 0.5)  # pitch, velocity, duration

# Set module parameter
bespoke.set("synth", "volume", 0.8)

# Get module parameter
vol = bespoke.get("synth", "volume")
```

### Timing

Scripts can schedule events and respond to timing.

```python
# Schedule function call
def my_function():
    print("Called!")

me.schedule_call(my_function, 1000)  # After 1000ms

# Get current time in measures
measure = bespoke.get_measure()

# Get time in seconds
time = bespoke.get_time()

# Get tempo
bpm = bespoke.get_tempo()
```

## Common Patterns

### Generative Sequencer

```python
import random

notes = [60, 62, 64, 65, 67, 69, 71, 72]  # C major scale

def on_pulse():
    # Play random note from scale
    pitch = random.choice(notes)
    velocity = random.randint(80, 120)
    bespoke.play_note(pitch, velocity, 0.25)
```

### Parameter Automation

```python
import math

def on_pulse():
    # Sine wave automation
    time = bespoke.get_time()
    value = (math.sin(time) + 1) / 2  # 0 to 1
    bespoke.set("synth", "filter_cutoff", value)
```

### Euclidean Rhythm

```python
def euclidean(steps, pulses):
    """Generate euclidean rhythm"""
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

step = 0
pattern = euclidean(16, 5)  # 5 hits in 16 steps

def on_pulse():
    global step
    if pattern[step]:
        bespoke.play_note(60, 100, 0.1)
    step = (step + 1) % len(pattern)
```

### MIDI Processing

```python
def on_note(pitch, velocity):
    # Transpose up an octave
    bespoke.play_note(pitch + 12, velocity, 0.5)
    
    # Also play original
    bespoke.play_note(pitch, velocity, 0.5)
```

### Conditional Logic

```python
count = 0

def on_pulse():
    global count
    count += 1
    
    # Play every 4th pulse
    if count % 4 == 0:
        bespoke.play_note(60, 100, 0.25)
```

## Advanced Features

### Sliders

Add custom sliders to script module:

```python
# Define sliders
me.add_slider("speed", 0.5, 0, 1)
me.add_slider("pitch", 60, 0, 127)

# Use slider values
def on_pulse():
    speed = me.get("speed")
    pitch = int(me.get("pitch"))
    bespoke.play_note(pitch, 100, speed)
```

### Grid Control

Control grid controllers (Launchpad, etc.):

```python
def on_grid_button(col, row, velocity):
    if velocity > 0:  # Button pressed
        # Light up button
        me.set_grid_led(col, row, 127)
        
        # Play note
        pitch = 60 + col + (row * 12)
        bespoke.play_note(pitch, velocity, 0.5)
    else:  # Button released
        # Turn off LED
        me.set_grid_led(col, row, 0)
```

### External Libraries

Import Python libraries:

```python
import random
import math
import time

# Can also import installed packages
# import numpy as np
# import mido
```

### State Management

Maintain state between calls:

```python
# Global variables persist
notes_played = []
current_scale = [60, 62, 64, 65, 67, 69, 71, 72]

def on_pulse():
    global notes_played
    
    # Choose note
    pitch = random.choice(current_scale)
    notes_played.append(pitch)
    
    # Keep last 10 notes
    if len(notes_played) > 10:
        notes_played.pop(0)
    
    bespoke.play_note(pitch, 100, 0.25)
```

## API Reference

See [API Reference](api-reference.md) for complete documentation of:
- `me` object methods
- `bespoke` object methods
- Module-specific APIs
- Callback functions

## Examples

See [Examples](examples.md) for:
- Generative music examples
- MIDI processing examples
- Automation examples
- Grid controller examples
- Advanced techniques

## Tips & Best Practices

### Performance
1. **Avoid heavy computation**: Scripts run in audio thread
2. **Use global variables**: For state between calls
3. **Schedule expensive operations**: Don't block audio
4. **Test thoroughly**: Errors can crash BespokeSynth
5. **Profile code**: Identify bottlenecks

### Debugging
1. **Use print()**: Output to console
2. **Check console**: Errors appear in red
3. **Test incrementally**: Build up complexity
4. **Save often**: Before testing risky code
5. **Use try/except**: Catch errors gracefully

### Code Organization
1. **Comment your code**: Explain logic
2. **Use functions**: Organize code
3. **Name clearly**: Descriptive variable names
4. **Keep it simple**: Start simple, add complexity
5. **Save as prefab**: Reuse scripts

## Common Issues

### Script Won't Run

**Check**:
- Syntax errors (shown in red)
- Indentation (Python is whitespace-sensitive)
- Click "run" button
- Check console for errors

### No Output

**Check**:
- Callback functions defined correctly
- Input connected (pulse, notes, etc.)
- print() statements for debugging
- Module enabled

### Performance Issues

**Solutions**:
- Reduce computation in callbacks
- Use simpler algorithms
- Schedule heavy operations
- Profile code

## Security

**Warning**: Scripts can execute arbitrary Python code.

**Best practices**:
- Only run scripts you trust
- Review code before running
- Be careful with file operations
- Don't run untrusted scripts

## Next Steps

- **[Getting Started](getting-started.md)** - Detailed tutorial
- **[API Reference](api-reference.md)** - Complete API docs
- **[Examples](examples.md)** - Example scripts
- **[Module APIs](module-apis.md)** - Module-specific scripting

## Resources

- [Python Documentation](https://docs.python.org/3/)
- [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ) - Ask questions
- [GitHub Examples](https://github.com/BespokeSynth/BespokeSynth/tree/main/scripts) - Community scripts

