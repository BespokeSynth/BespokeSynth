# Getting Started with Python Scripting

A step-by-step guide to writing your first BespokeSynth scripts.

## Prerequisites

- BespokeSynth installed and running
- Basic Python knowledge (variables, functions, loops)
- Understanding of MIDI concepts (pitch, velocity, notes)

## Your First Script

### Step 1: Create a Script Module

1. Right-click on the BespokeSynth canvas
2. Navigate to the module menu
3. Select **script**
4. A script module appears with a code editor

### Step 2: Write Hello World

In the code editor, type:

```python
def on_pulse():
    print("Hello from BespokeSynth!")
```

### Step 3: Add a Pulser

1. Right-click canvas again
2. Select **pulser** module
3. Click and drag from pulser's output to script module's input
4. The pulser will now trigger your script

### Step 4: Run the Script

1. Click the **run** button on the script module
2. Watch the console output (bottom of screen)
3. You should see "Hello from BespokeSynth!" appearing on each pulse

**Congratulations!** You've written your first BespokeSynth script.

---

## Playing Your First Note

Let's make the script play a note instead of just printing.

### Update Your Script

```python
def on_pulse():
    me.play_note(60, 100, 1.0/8)
```

**What this does:**
- `me.play_note()` - Plays a note from the script module
- `60` - MIDI pitch (middle C)
- `100` - Velocity (how loud, 0-127)
- `1.0/8` - Duration (1/8 of a measure)

### Connect Audio Output

1. Create a **synth** module (or any instrument)
2. Connect script module's output to the synth
3. Connect synth to **output** module
4. Click **run** on the script

You should now hear a note playing on each pulse!

---

## Understanding Callbacks

Scripts respond to events through **callback functions**. These are special function names that BespokeSynth calls automatically.

### Common Callbacks

#### `on_pulse()`

Called when a pulse is received.

```python
def on_pulse():
    print("Pulse!")
```

#### `on_note(pitch, velocity)`

Called when a MIDI note is received.

```python
def on_note(pitch, velocity):
    print(f"Note: {pitch}, Velocity: {velocity}")
    me.play_note(pitch, velocity, 1.0/4)
```

**Try it:**
1. Create a **notesequencer** or **keyboard** module
2. Connect it to your script module
3. Play notes and watch them trigger your script

#### `on_grid_button(col, row, velocity)`

Called when a grid controller button is pressed.

```python
def on_grid_button(col, row, velocity):
    if velocity > 0:  # Button pressed
        print(f"Button {col},{row} pressed")
```

---

## Working with Variables

Use global variables to maintain state between callbacks.

### Example: Counter

```python
count = 0

def on_pulse():
    global count
    count += 1
    print(f"Pulse #{count}")
```

**Important:** Use `global` keyword to modify variables defined outside functions.

### Example: Toggle

```python
is_playing = False

def on_pulse():
    global is_playing
    
    if is_playing:
        me.play_note(60, 100, 1.0/8)
    
    is_playing = not is_playing  # Toggle
```

---

## Using the `me` Object

The `me` object represents your script module and provides methods to interact with BespokeSynth.

### Playing Notes

```python
# Play a note with automatic note-off
me.play_note(pitch, velocity, length)

# Examples:
me.play_note(60, 100, 1.0/4)     # Quarter note
me.play_note(64, 80, 1.0/8)      # Eighth note
me.play_note(67, 120, 1.0/16)    # Sixteenth note
```

### Scheduling Events

```python
# Schedule a note to play later
me.schedule_note(delay, pitch, velocity, length)

# Example: Play note after 1/4 measure
me.schedule_note(1.0/4, 60, 100, 1.0/8)
```

### Controlling Parameters

```python
# Set a parameter value
me.set("slider1", 0.5)

# Get a parameter value
value = me.get("slider1")

# Adjust a parameter
me.adjust("slider1", 0.1)  # Increase by 0.1
```

---

## Using the `bespoke` Module

The `bespoke` module provides global functions for timing, scales, and more.

### Timing

```python
# Get current measure (with decimal)
time = bespoke.get_measure_time()  # e.g., 4.75

# Get current measure (integer)
measure = bespoke.get_measure()  # e.g., 4

# Get current tempo
bpm = bespoke.get_tempo()  # e.g., 120.0
```

### Scales

```python
# Get current scale notes
scale = bespoke.get_scale_range(4, 8)  # Octave 4, 8 notes
# Returns: [60, 62, 64, 65, 67, 69, 71, 72] for C major

# Convert note name to pitch
pitch = bespoke.name_to_pitch("C4")  # Returns 60

# Convert pitch to frequency
freq = bespoke.pitch_to_freq(60)  # Returns 261.63 Hz
```

---

## Common Patterns

### Random Notes from Scale

```python
import random

def on_pulse():
    # Get scale notes
    scale = bespoke.get_scale_range(4, 8)
    
    # Pick random note
    pitch = random.choice(scale)
    
    me.play_note(pitch, 100, 1.0/8)
```

### Simple Sequencer

```python
notes = [60, 62, 64, 65, 67, 69, 71, 72]
step = 0

def on_pulse():
    global step
    
    # Play current note
    me.play_note(notes[step], 100, 1.0/8)
    
    # Move to next step
    step = (step + 1) % len(notes)
```

### Note Echo

```python
def on_note(pitch, velocity):
    # Play original
    me.play_note(pitch, velocity, 1.0/4)
    
    # Play echo
    me.schedule_note(1.0/4, pitch, velocity * 0.7, 1.0/4)
```

### Parameter Automation

```python
import math

def on_pulse():
    # Get current time
    time = bespoke.get_measure_time()
    
    # Create sine wave (0 to 1)
    value = (math.sin(time * math.pi * 2) + 1) / 2
    
    # Apply to parameter
    me.set("filter~cutoff", value)
```

---

## Debugging Tips

### Use print() Statements

```python
def on_pulse():
    print("Debug: pulse received")
    pitch = 60
    print(f"Playing pitch: {pitch}")
    me.play_note(pitch, 100, 1.0/8)
```

### Use me.output()

```python
def on_note(pitch, velocity):
    me.output(f"Note: {pitch}, Vel: {velocity}")
```

### Check for Errors

- Errors appear in red in the console
- Check indentation (Python is whitespace-sensitive)
- Make sure to click **run** after editing

### Test Incrementally

1. Start with simple code
2. Test it works
3. Add one feature at a time
4. Test after each addition

---

## Best Practices

### 1. Comment Your Code

```python
# This is a simple sequencer
notes = [60, 62, 64, 65]  # C major scale fragment
step = 0  # Current step in sequence

def on_pulse():
    global step
    # Play current note
    me.play_note(notes[step], 100, 1.0/8)
    # Advance to next step
    step = (step + 1) % len(notes)
```

### 2. Use Descriptive Names

```python
# Good
current_pitch = 60
note_velocity = 100

# Bad
x = 60
v = 100
```

### 3. Keep Functions Small

```python
# Break complex logic into functions
def get_random_scale_note():
    scale = bespoke.get_scale_range(4, 8)
    return random.choice(scale)

def on_pulse():
    pitch = get_random_scale_note()
    me.play_note(pitch, 100, 1.0/8)
```

### 4. Save Often

- Scripts are saved with your patch
- Click **save** in BespokeSynth regularly
- Test risky code in a separate patch first

---

## Next Steps

### Learn More

- **[API Reference](api-reference.md)** - Complete documentation of all functions
- **[Examples](examples.md)** - More complex script examples
- **[Python Scripting Overview](README.md)** - Full feature overview

### Try These Challenges

1. **Euclidean Rhythm**: Implement a Euclidean rhythm generator
2. **Chord Player**: Make a script that plays chords when triggered
3. **Arpeggiator**: Create an arpeggiator that responds to incoming notes
4. **Generative Melody**: Build a system that generates melodies algorithmically
5. **Grid Sequencer**: Use a grid controller to create a step sequencer

### Join the Community

- [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ) - Ask questions and share scripts
- [GitHub Discussions](https://github.com/BespokeSynth/BespokeSynth/discussions) - Community forum
- [Example Scripts](https://github.com/BespokeSynth/BespokeSynth/tree/main/scripts) - Community contributions

---

## Troubleshooting

### Script Won't Run

**Check:**
- Syntax errors (shown in red)
- Indentation is correct
- Clicked the **run** button
- Python is installed correctly

### No Sound

**Check:**
- Script module connected to an instrument
- Instrument connected to output
- Volume levels are up
- Audio device is working

### Notes Not Triggering

**Check:**
- Callback function name is correct (`on_pulse`, `on_note`, etc.)
- Input module is connected
- Input module is enabled and running

### Performance Issues

**Solutions:**
- Reduce computation in callbacks
- Avoid heavy loops
- Use simpler algorithms
- Profile your code to find bottlenecks

---

## Quick Reference

### Essential Functions

```python
# Play notes
me.play_note(pitch, velocity, length)
me.schedule_note(delay, pitch, velocity, length)

# Parameters
me.set(path, value)
value = me.get(path)

# Timing
time = bespoke.get_measure_time()
measure = bespoke.get_measure()
bpm = bespoke.get_tempo()

# Scales
scale = bespoke.get_scale_range(octave, count)
pitch = bespoke.name_to_pitch("C4")

# Output
print("message")
me.output("message")
```

### Essential Callbacks

```python
def on_pulse():
    pass

def on_note(pitch, velocity):
    pass

def on_grid_button(col, row, velocity):
    pass
```

---

Happy scripting! 🎵
