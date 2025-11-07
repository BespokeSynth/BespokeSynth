# Python Scripting Examples

Practical examples for BespokeSynth Python scripting.

## Table of Contents

- [Basic Examples](#basic-examples)
- [Generative Music](#generative-music)
- [MIDI Processing](#midi-processing)
- [Sequencing](#sequencing)
- [Parameter Automation](#parameter-automation)
- [Grid Controllers](#grid-controllers)
- [Advanced Techniques](#advanced-techniques)

---

## Basic Examples

### Hello World

```python
def on_pulse():
    print("Hello from BespokeSynth!")
```

### Play a Note on Pulse

```python
def on_pulse():
    me.play_note(60, 100, 1.0/8)  # Middle C, velocity 100, 1/8 measure
```

### Note Echo

```python
def on_note(pitch, velocity):
    # Play original note
    me.play_note(pitch, velocity, 1.0/4)
    
    # Play echo after 1/4 measure
    me.schedule_note(1.0/4, pitch, velocity * 0.7, 1.0/4)
    
    # Play second echo
    me.schedule_note(2.0/4, pitch, velocity * 0.5, 1.0/4)
```

### Simple Counter

```python
count = 0

def on_pulse():
    global count
    count += 1
    print(f"Pulse count: {count}")
```

---

## Generative Music

### Random Scale Notes

```python
import random

def on_pulse():
    # Get current scale
    scale = bespoke.get_scale_range(4, 8)  # Octave 4, 8 notes
    
    # Pick random note
    pitch = random.choice(scale)
    velocity = random.randint(80, 120)
    
    me.play_note(pitch, velocity, 1.0/8)
```

### Euclidean Rhythm Generator

```python
def euclidean(steps, pulses):
    """Generate Euclidean rhythm pattern"""
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

# Create patterns
kick_pattern = euclidean(16, 4)   # 4 hits in 16 steps
snare_pattern = euclidean(16, 3)  # 3 hits in 16 steps
hihat_pattern = euclidean(16, 7)  # 7 hits in 16 steps

step = 0

def on_pulse():
    global step
    
    # Play drums based on patterns
    if kick_pattern[step]:
        me.play_note(36, 127, 1.0/16)  # Kick
    
    if snare_pattern[step]:
        me.play_note(38, 100, 1.0/16)  # Snare
    
    if hihat_pattern[step]:
        me.play_note(42, 80, 1.0/16)   # Hi-hat
    
    step = (step + 1) % 16
```

### Markov Chain Melody

```python
import random

# Transition probabilities (pitch -> [possible next pitches])
transitions = {
    60: [60, 62, 64],  # C -> C, D, E
    62: [60, 64, 65],  # D -> C, E, F
    64: [62, 65, 67],  # E -> D, F, G
    65: [64, 67, 69],  # F -> E, G, A
    67: [65, 69, 71],  # G -> F, A, B
    69: [67, 71, 72],  # A -> G, B, C
    71: [69, 72, 60],  # B -> A, C, C
    72: [71, 60, 62],  # C -> B, C, D
}

current_pitch = 60

def on_pulse():
    global current_pitch
    
    # Play current note
    me.play_note(current_pitch, 100, 1.0/8)
    
    # Choose next note based on transitions
    if current_pitch in transitions:
        current_pitch = random.choice(transitions[current_pitch])
```

### Probabilistic Rhythm

```python
import random

def on_pulse():
    # 75% chance to play
    if random.random() < 0.75:
        # Random velocity
        velocity = random.randint(60, 120)
        
        # Random pitch from pentatonic scale
        pitches = [60, 62, 64, 67, 69]  # C pentatonic
        pitch = random.choice(pitches)
        
        me.play_note(pitch, velocity, 1.0/16)
```

### Generative Arpeggio

```python
import random

# Arpeggio patterns
patterns = [
    [0, 2, 4, 7],      # Major 7th
    [0, 3, 7, 10],     # Minor 7th
    [0, 4, 7, 11],     # Major 7th
    [0, 3, 6, 10],     # Diminished 7th
]

current_pattern = patterns[0]
step = 0
root = 60

def on_pulse():
    global step, current_pattern, root
    
    # Play current note in pattern
    pitch = root + current_pattern[step]
    me.play_note(pitch, 100, 1.0/16)
    
    step += 1
    
    # Change pattern every 16 steps
    if step >= 16:
        step = 0
        current_pattern = random.choice(patterns)
        root = random.choice([48, 53, 55, 60])  # Random root
```

---

## MIDI Processing

### Octave Doubler

```python
def on_note(pitch, velocity):
    # Play original
    me.play_note(pitch, velocity, 1.0/4)
    
    # Play octave up
    me.play_note(pitch + 12, velocity * 0.8, 1.0/4)
    
    # Play octave down
    me.play_note(pitch - 12, velocity * 0.8, 1.0/4)
```

### Chord Generator

```python
def on_note(pitch, velocity):
    # Major chord
    me.play_note(pitch, velocity, 1.0/2)      # Root
    me.play_note(pitch + 4, velocity, 1.0/2)  # Major 3rd
    me.play_note(pitch + 7, velocity, 1.0/2)  # Perfect 5th
```

### Note Quantizer

```python
def on_note(pitch, velocity):
    # Quantize to C major scale
    scale = [0, 2, 4, 5, 7, 9, 11]  # Major scale intervals
    
    # Get pitch class (0-11)
    pitch_class = pitch % 12
    
    # Find nearest scale note
    nearest = min(scale, key=lambda x: abs(x - pitch_class))
    
    # Calculate quantized pitch
    octave = pitch // 12
    quantized = octave * 12 + nearest
    
    me.play_note(quantized, velocity, 1.0/4)
```

### Velocity Curve

```python
def on_note(pitch, velocity):
    # Apply exponential velocity curve
    curved_velocity = (velocity / 127.0) ** 2 * 127
    me.play_note(pitch, curved_velocity, 1.0/4)
```

### Note Delay

```python
delays = []

def on_note(pitch, velocity):
    # Play original
    me.play_note(pitch, velocity, 1.0/4)
    
    # Schedule delayed repeats
    for i in range(1, 4):
        delay = i * 1.0/8
        vel = velocity * (0.7 ** i)  # Decay velocity
        me.schedule_note(delay, pitch, vel, 1.0/8)
```

---

## Sequencing

### Step Sequencer

```python
# Define sequence
sequence = [
    (60, 100),  # C
    (62, 80),   # D
    (64, 90),   # E
    (65, 100),  # F
    (67, 110),  # G
    (64, 85),   # E
    (62, 75),   # D
    (60, 100),  # C
]

step = 0

def on_pulse():
    global step
    
    pitch, velocity = sequence[step]
    me.play_note(pitch, velocity, 1.0/8)
    
    step = (step + 1) % len(sequence)
```

### Polyrhythmic Sequencer

```python
# Two sequences with different lengths
seq1 = [60, 64, 67, 72]  # 4 steps
seq2 = [48, 55, 52]      # 3 steps

step1 = 0
step2 = 0

def on_pulse():
    global step1, step2
    
    # Play both sequences
    me.play_note(seq1[step1], 100, 1.0/16, -0.5, 0)  # Pan left
    me.play_note(seq2[step2], 80, 1.0/16, 0.5, 0)    # Pan right
    
    step1 = (step1 + 1) % len(seq1)
    step2 = (step2 + 1) % len(seq2)
```

### Conditional Sequencer

```python
sequence = [60, 62, 64, 65, 67, 69, 71, 72]
step = 0

def on_pulse():
    global step
    
    measure = bespoke.get_measure()
    
    # Play every step in measures 0-3
    if measure < 4:
        me.play_note(sequence[step], 100, 1.0/8)
    
    # Play every other step in measures 4-7
    elif measure < 8:
        if step % 2 == 0:
            me.play_note(sequence[step], 100, 1.0/8)
    
    # Play every fourth step in measures 8+
    else:
        if step % 4 == 0:
            me.play_note(sequence[step], 100, 1.0/8)
    
    step = (step + 1) % len(sequence)
```

### Dynamic Note Canvas

```python
import random

canvas = None

def on_pulse():
    global canvas
    
    # Get canvas reference
    if canvas is None:
        canvas = notecanvas.get("notecanvas")
    
    # Clear every 4 measures
    measure = bespoke.get_measure()
    if measure % 4 == 0 and bespoke.get_measure_time() % 1 < 0.01:
        canvas.clear()
        
        # Generate new pattern
        for i in range(16):
            pos = i / 4.0  # Position in measures
            pitch = random.choice([60, 62, 64, 67, 69])
            velocity = random.randint(80, 120)
            length = random.choice([0.25, 0.5, 1.0])
            
            canvas.add_note(pos, pitch, velocity, length)
```

---

## Parameter Automation

### LFO Automation

```python
import math

def on_pulse():
    # Get current time
    time = bespoke.get_measure_time()
    
    # Sine wave LFO (0 to 1)
    lfo = (math.sin(time * math.pi * 2) + 1) / 2
    
    # Modulate filter cutoff
    me.set("filter~cutoff", lfo)
```

### Envelope Follower

```python
import math

envelope_value = 0

def on_note(pitch, velocity):
    global envelope_value
    
    # Set envelope based on velocity
    envelope_value = velocity / 127.0
    
    # Play note
    me.play_note(pitch, velocity, 1.0/4)

def on_pulse():
    global envelope_value
    
    # Decay envelope
    envelope_value *= 0.95
    
    # Apply to parameter
    me.set("volume", envelope_value)
```

### Step Automation

```python
# Automation sequence
automation = [0.1, 0.3, 0.5, 0.7, 0.9, 0.7, 0.5, 0.3]
step = 0

def on_pulse():
    global step

    # Set parameter
    me.set("filter~cutoff", automation[step])

    step = (step + 1) % len(automation)
```

### Random Walk Automation

```python
import random

current_value = 0.5

def on_pulse():
    global current_value

    # Random walk
    change = random.uniform(-0.05, 0.05)
    current_value = max(0, min(1, current_value + change))

    # Apply to parameter
    me.set("oscillator~pw", current_value)
```

---

## Grid Controllers

### Simple Grid Sequencer

```python
# Initialize grid
g = grid.get("grid")
g.set_grid(8, 4)

# Sequence data
sequence = [[0] * 8 for _ in range(4)]

step = 0

def on_grid_button(col, row, velocity):
    global sequence

    if velocity > 0:  # Button pressed
        # Toggle step
        sequence[row][col] = 1 - sequence[row][col]

        # Update LED
        g.set(col, row, sequence[row][col])

def on_pulse():
    global step

    # Highlight current step
    for row in range(4):
        g.highlight_cell(step, row, 0, 0.1, 2)

        # Play if step is active
        if sequence[row][step]:
            pitch = 60 + row * 3
            me.play_note(pitch, 100, 1.0/16)

    step = (step + 1) % 8
```

### Grid Drum Machine

```python
g = grid.get("grid")
g.set_grid(16, 4)

# Set row labels
g.set_label(0, "Kick")
g.set_label(1, "Snare")
g.set_label(2, "HiHat")
g.set_label(3, "Clap")

# Drum pitches
drums = [36, 38, 42, 39]

# Pattern storage
pattern = [[0] * 16 for _ in range(4)]

step = 0

def on_grid_button(col, row, velocity):
    if velocity > 0:
        pattern[row][col] = 1 - pattern[row][col]
        g.set(col, row, pattern[row][col])

def on_pulse():
    global step

    # Play drums
    for row in range(4):
        if pattern[row][step]:
            me.play_note(drums[row], 127, 1.0/16)

        # Highlight current step
        g.highlight_cell(step, row, 0, 0.05, 1)

    step = (step + 1) % 16
```

### Grid Chord Player

```python
g = grid.get("grid")
g.set_grid(8, 8)
g.set_momentary(True)

# Chord definitions (relative to root)
chords = {
    0: [0, 4, 7],      # Major
    1: [0, 3, 7],      # Minor
    2: [0, 4, 7, 11],  # Maj7
    3: [0, 3, 7, 10],  # Min7
}

def on_grid_button(col, row, velocity):
    if velocity > 0:
        # Calculate root pitch
        root = 48 + col * 2 + row * 12

        # Get chord type
        chord_type = row % 4
        intervals = chords[chord_type]

        # Play chord
        for interval in intervals:
            me.play_note(root + interval, 100, 1.0/2)

        # Light up button
        g.set(col, row, 1.0)
    else:
        # Turn off light
        g.set(col, row, 0.0)
```

### Grid XY Controller

```python
g = grid.get("grid")
g.set_grid(8, 8)

last_x = 0
last_y = 0

def on_grid_button(col, row, velocity):
    global last_x, last_y

    if velocity > 0:
        # Clear previous position
        g.set(last_x, last_y, 0)

        # Set new position
        g.set(col, row, 1.0)

        # Map to parameters
        x_value = col / 7.0  # 0 to 1
        y_value = row / 7.0  # 0 to 1

        me.set("filter~cutoff", x_value)
        me.set("filter~q", y_value)

        last_x = col
        last_y = row
```

---

## Advanced Techniques

### Multi-Output Routing

```python
# Set up 4 outputs
me.set_num_note_outputs(4)

def on_note(pitch, velocity):
    # Route to different outputs based on pitch
    if pitch < 48:
        output = 0  # Bass
    elif pitch < 60:
        output = 1  # Mid-low
    elif pitch < 72:
        output = 2  # Mid-high
    else:
        output = 3  # High

    me.play_note(pitch, velocity, 1.0/4, 0, output)
```

### Probability-Based Generative System

```python
import random

# Musical rules
rules = {
    'note_probability': 0.7,
    'rest_probability': 0.3,
    'repeat_probability': 0.4,
    'jump_probability': 0.3,
    'step_probability': 0.3,
}

current_pitch = 60
scale = [60, 62, 64, 65, 67, 69, 71, 72]

def on_pulse():
    global current_pitch

    # Decide whether to play
    if random.random() < rules['note_probability']:
        velocity = random.randint(70, 110)
        me.play_note(current_pitch, velocity, 1.0/8)

    # Decide next pitch
    rand = random.random()

    if rand < rules['repeat_probability']:
        # Repeat current pitch
        pass
    elif rand < rules['repeat_probability'] + rules['jump_probability']:
        # Jump to random scale note
        current_pitch = random.choice(scale)
    else:
        # Step to adjacent scale note
        current_index = scale.index(current_pitch) if current_pitch in scale else 0
        direction = random.choice([-1, 1])
        new_index = (current_index + direction) % len(scale)
        current_pitch = scale[new_index]
```

### State Machine Sequencer

```python
import random

# Define states
states = {
    'intro': {
        'pattern': [60, 60, 62, 64],
        'next_states': ['verse', 'intro'],
        'probabilities': [0.3, 0.7],
    },
    'verse': {
        'pattern': [60, 64, 67, 64, 62, 60, 62, 64],
        'next_states': ['verse', 'chorus'],
        'probabilities': [0.7, 0.3],
    },
    'chorus': {
        'pattern': [67, 69, 71, 72, 71, 69, 67, 65],
        'next_states': ['verse', 'chorus', 'outro'],
        'probabilities': [0.5, 0.3, 0.2],
    },
    'outro': {
        'pattern': [72, 71, 69, 67, 65, 64, 62, 60],
        'next_states': ['outro'],
        'probabilities': [1.0],
    },
}

current_state = 'intro'
step = 0

def on_pulse():
    global current_state, step

    # Get current pattern
    pattern = states[current_state]['pattern']

    # Play current note
    me.play_note(pattern[step], 100, 1.0/8)

    step += 1

    # Check if pattern is complete
    if step >= len(pattern):
        step = 0

        # Transition to next state
        next_states = states[current_state]['next_states']
        probabilities = states[current_state]['probabilities']
        current_state = random.choices(next_states, probabilities)[0]

        print(f"Transitioning to: {current_state}")
```

### OSC Integration

```python
# Connect to OSC input
me.connect_osc_input(5000)

# Store OSC values
osc_values = {}

def on_osc(message):
    # Parse OSC message
    parts = message.split()
    address = parts[0]

    if len(parts) > 1:
        try:
            value = float(parts[1])
            osc_values[address] = value
            print(f"OSC: {address} = {value}")
        except:
            pass

def on_pulse():
    # Use OSC values to control parameters
    if '/filter/cutoff' in osc_values:
        me.set("filter~cutoff", osc_values['/filter/cutoff'])

    if '/note/pitch' in osc_values:
        pitch = int(osc_values['/note/pitch'])
        me.play_note(pitch, 100, 1.0/8)
```

### MIDI Controller Integration

```python
# Get MIDI controller reference
controller = midicontroller.get("midicontroller")
controller.add_script_listener(me.me())

# Store CC values
cc_values = {}

def on_midi(messageType, control, value, channel):
    if messageType == 1:  # Control Change
        cc_values[control] = value
        print(f"CC{control} = {value}")

def on_pulse():
    # Use CC values for generative parameters
    if 1 in cc_values:  # Mod wheel
        # Control note density
        density = cc_values[1] / 127.0
        if random.random() < density:
            scale = bespoke.get_scale_range(4, 8)
            pitch = random.choice(scale)
            me.play_note(pitch, 100, 1.0/16)
```

### Snapshot Automation

```python
snaps = snapshots.get("snapshots")

def on_pulse():
    measure = bespoke.get_measure()

    # Change snapshot every 4 measures
    if measure % 4 == 0 and bespoke.get_measure_time() % 1 < 0.01:
        snapshot_index = (measure // 4) % snaps.get_size()
        if snaps.has_snapshot(snapshot_index):
            snaps.set_snapshot(snapshot_index)
            print(f"Recalled snapshot {snapshot_index}")
```

### Dynamic Module Creation

```python
created_modules = []

def on_note(pitch, velocity):
    # Create a new oscillator for each note
    x = 100 + len(created_modules) * 150
    y = 200

    osc = module.create("oscillator", x, y)
    created_modules.append(osc)

    # Configure it
    osc.set("freq", bespoke.pitch_to_freq(pitch))
    osc.set("vol", velocity / 127.0)

    # Clean up old modules
    if len(created_modules) > 5:
        old_module = created_modules.pop(0)
        old_module.delete()
```

### Algorithmic Composition

```python
import random

# Composition parameters
key = 60  # C
scale_intervals = [0, 2, 4, 5, 7, 9, 11]  # Major scale
phrase_length = 8
phrases = []

def generate_phrase():
    """Generate a new musical phrase"""
    phrase = []
    for i in range(phrase_length):
        # Choose scale degree
        degree = random.choice(range(len(scale_intervals)))
        octave = random.choice([0, 12, 24])
        pitch = key + scale_intervals[degree] + octave

        # Choose rhythm
        duration = random.choice([1.0/16, 1.0/8, 1.0/4])
        velocity = random.randint(80, 120)

        phrase.append((pitch, velocity, duration))

    return phrase

# Generate initial phrases
for i in range(4):
    phrases.append(generate_phrase())

current_phrase = 0
step = 0

def on_pulse():
    global current_phrase, step

    # Play current note
    pitch, velocity, duration = phrases[current_phrase][step]
    me.play_note(pitch, velocity, duration)

    step += 1

    # Move to next phrase
    if step >= phrase_length:
        step = 0
        current_phrase = (current_phrase + 1) % len(phrases)

        # Occasionally generate new phrase
        if random.random() < 0.2:
            phrases[current_phrase] = generate_phrase()
```

---

## See Also

- [API Reference](api-reference.md) - Complete API documentation
- [Python Scripting Overview](README.md) - Getting started guide
- [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ) - Share your scripts!


