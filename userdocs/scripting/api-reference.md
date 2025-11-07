# Python Scripting API Reference

Complete reference for BespokeSynth's Python scripting API.

## Table of Contents

- [Callback Functions](#callback-functions)
- [The `me` Object](#the-me-object)
- [The `bespoke` Module](#the-bespoke-module)
- [Module-Specific APIs](#module-specific-apis)
  - [module](#module)
  - [notesequencer](#notesequencer)
  - [drumsequencer](#drumsequencer)
  - [basslinesequencer](#basslinesequencer)
  - [notecanvas](#notecanvas)
  - [sampleplayer](#sampleplayer)
  - [drumplayer](#drumplayer)
  - [grid](#grid)
  - [midicontroller](#midicontroller)
  - [osccontroller](#osccontroller)
  - [oscoutput](#oscoutput)
  - [envelope](#envelope)
  - [snapshots](#snapshots)
  - [vstplugin](#vstplugin)
  - [linnstrument](#linnstrument)

---

## Callback Functions

These functions are called automatically by BespokeSynth when events occur. Define them in your script to respond to events.

### `on_pulse()`

Called when the script module receives a pulse input.

**Example:**
```python
def on_pulse():
    print("Pulse received!")
    me.play_note(60, 100)
```

### `on_note(pitch, velocity)`

Called when the script module receives a note input.

**Parameters:**
- `pitch` (int): MIDI pitch (0-127)
- `velocity` (int): Note velocity (0-127, 0 = note off)

**Example:**
```python
def on_note(pitch, velocity):
    # Transpose up an octave
    me.play_note(pitch + 12, velocity)
```

### `on_grid_button(col, row, velocity)`

Called when a grid controller button is pressed (requires grid connection).

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index
- `velocity` (float): Button velocity (0.0-1.0)

**Example:**
```python
def on_grid_button(col, row, velocity):
    if velocity > 0:  # Button pressed
        pitch = 60 + col + (row * 12)
        me.play_note(pitch, 100)
```

### `on_osc(message)`

Called when an OSC message is received. Requires `me.connect_osc_input(port)` to be called first.

**Parameters:**
- `message` (str): OSC message as string (address and values)

**Example:**
```python
me.connect_osc_input(5000)  # Listen on port 5000

def on_osc(message):
    print("OSC received:", message)
```

### `on_midi(messageType, control, value, channel)`

Called when MIDI data is received from a MIDI controller. Requires the MIDI controller to register this script with `add_script_listener(me.me())`.

**Parameters:**
- `messageType` (int): MIDI message type (0=Note, 1=Control, 2=Program, 3=PitchBend)
- `control` (int): Control number or note number
- `value` (float): Value (0-127 for notes/CC, -1 to 1 for pitch bend)
- `channel` (int): MIDI channel (0-15)

**Example:**
```python
def on_midi(messageType, control, value, channel):
    if messageType == 1:  # Control Change
        print(f"CC {control} = {value}")
```

### `on_sysex(data)`

Called when SysEx data is received from a MIDI controller (requires `send_sysex` option enabled).

**Parameters:**
- `data` (bytes): SysEx data

---

## The `me` Object

The `me` object represents the script module itself. It provides methods for playing notes, scheduling events, and controlling parameters.

### Note Output

#### `me.play_note(pitch, velocity, length=1.0/16.0, pan=0, output_index=0)`

Play a note with automatic note-off.

**Parameters:**
- `pitch` (float): MIDI pitch (supports microtonal values)
- `velocity` (float): Note velocity (0-127)
- `length` (float): Note duration in measures (default: 1/16)
- `pan` (float): Pan position (-1 to 1, default: 0)
- `output_index` (int): Output index for multi-output (default: 0)

**Example:**
```python
me.play_note(60, 100, 1.0/8)  # Play middle C for 1/8 measure
me.play_note(60.5, 100)  # Microtonal pitch
```

#### `me.note_msg(pitch, velocity, pan=0, output_index=0)`

Send a note on/off message without automatic note-off.

**Parameters:**
- `pitch` (float): MIDI pitch
- `velocity` (float): Note velocity (0 = note off)
- `pan` (float): Pan position (-1 to 1, default: 0)
- `output_index` (int): Output index (default: 0)

**Example:**
```python
me.note_msg(60, 100)  # Note on
me.note_msg(60, 0)    # Note off
```

#### `me.schedule_note(delay, pitch, velocity, length=1.0/16.0, pan=0, output_index=0)`

Schedule a note to play after a delay.

**Parameters:**
- `delay` (float): Delay in measures
- `pitch` (float): MIDI pitch
- `velocity` (float): Note velocity (0-127)
- `length` (float): Note duration in measures (default: 1/16)
- `pan` (float): Pan position (-1 to 1, default: 0)
- `output_index` (int): Output index (default: 0)

**Example:**
```python
me.schedule_note(1.0/4, 60, 100, 1.0/8)  # Play after 1/4 measure
```

#### `me.schedule_note_msg(delay, pitch, velocity, pan=0, output_index=0)`

Schedule a note message after a delay (no automatic note-off).

**Parameters:**
- `delay` (float): Delay in measures
- `pitch` (float): MIDI pitch
- `velocity` (float): Note velocity
- `pan` (float): Pan position (-1 to 1, default: 0)
- `output_index` (int): Output index (default: 0)

### Scheduling

#### `me.schedule_call(delay, method)`

Schedule a function call after a delay.

**Parameters:**
- `delay` (float): Delay in measures
- `method` (str): Method name to call (as string with parentheses)

**Example:**
```python
def my_function():
    print("Called!")

me.schedule_call(1.0/4, "my_function()")
```

### Parameter Control

#### `me.set(path, value)`

Set a UI control value.

**Parameters:**
- `path` (str): Control path (relative to script module or absolute)
- `value` (float): Value to set

**Example:**
```python
me.set("slider1", 0.5)
me.set("oscillator~pw", 0.3)  # Control in another module
```

#### `me.get(path)`

Get a UI control value.

**Parameters:**
- `path` (str): Control path

**Returns:**
- `float`: Current value

**Example:**
```python
value = me.get("slider1")
```

#### `me.set_text(path, text)`

Set text entry value.

**Parameters:**
- `path` (str): Text entry control path
- `text` (str): Text to set

#### `me.get_text(path)`

Get text entry value.

**Parameters:**
- `path` (str): Text entry control path

**Returns:**
- `str`: Current text

#### `me.schedule_set(delay, path, value)`

Schedule a parameter change after a delay.

**Parameters:**
- `delay` (float): Delay in measures
- `path` (str): Control path
- `value` (float): Value to set

**Example:**
```python
me.schedule_set(1.0, "filter~cutoff", 1.0)
```

#### `me.adjust(path, amount)`

Adjust a parameter by a relative amount.

**Parameters:**
- `path` (str): Control path
- `amount` (float): Amount to add (respects min/max)

**Example:**
```python
me.adjust("slider1", 0.1)  # Increase by 0.1
```

### MIDI Output

#### `me.send_cc(control, value, output_index=0)`

Send a MIDI CC message.

**Parameters:**
- `control` (int): CC number (0-127)
- `value` (int): CC value (0-127)
- `output_index` (int): Output index (default: 0)

### Utility

#### `me.output(obj)`

Print to the script module's output console.

**Parameters:**
- `obj` (any): Object to print

**Example:**
```python
me.output("Hello world!")
me.output([1, 2, 3])
```

#### `me.stop()`

Stop the script and send all note-offs.

#### `me.me()`

Get reference to the script module itself (for passing to other modules).

**Returns:**
- `scriptmodule`: Reference to this script module

**Example:**
```python
controller = midicontroller.get("midicontroller")
controller.add_script_listener(me.me())
```

#### `me.get_caller()`

Get reference to the script module that called this one.

**Returns:**
- `scriptmodule`: Reference to calling script module

#### `me.set_num_note_outputs(num)`

Set the number of note outputs for multi-output routing.

**Parameters:**
- `num` (int): Number of outputs

#### `me.connect_osc_input(port)`

Connect to an OSC input port to receive OSC messages.

**Parameters:**
- `port` (int): OSC port number

#### `me.highlight_line(lineNum, scriptModuleIndex)`

Highlight a line in a script module (for debugging).

**Parameters:**
- `lineNum` (int): Line number to highlight
- `scriptModuleIndex` (int): Script module index

---

## The `bespoke` Module

The `bespoke` module provides global functions for timing, scales, and system control.

### Timing

#### `bespoke.get_measure_time()`

Get current time in measures (with fractional part).

**Returns:**
- `float`: Current measure time

**Example:**
```python
time = bespoke.get_measure_time()  # e.g., 4.75
```

#### `bespoke.get_measure()`

Get current measure number (integer).

**Returns:**
- `int`: Current measure

**Example:**
```python
measure = bespoke.get_measure()  # e.g., 4
```

#### `bespoke.get_step(subdivision)`

Get current step for a given subdivision.

**Parameters:**
- `subdivision` (int): Steps per measure (e.g., 16 for 16th notes)

**Returns:**
- `int`: Current step number

**Example:**
```python
step = bespoke.get_step(16)  # Current 16th note step
```

#### `bespoke.count_per_measure(subdivision)`

Get total number of steps per measure for a subdivision.

**Parameters:**
- `subdivision` (int): Steps per measure

**Returns:**
- `int`: Total steps per measure

**Example:**
```python
total = bespoke.count_per_measure(16)  # Returns 16
```

#### `bespoke.time_until_subdivision(subdivision)`

Get time until next subdivision boundary.

**Parameters:**
- `subdivision` (int): Subdivision to sync to

**Returns:**
- `float`: Time in measures until next subdivision

**Example:**
```python
# Schedule on next downbeat
delay = bespoke.time_until_subdivision(1)
me.schedule_call(delay, "on_downbeat()")
```

#### `bespoke.get_time_sig_ratio()`

Get time signature ratio (for non-4/4 time signatures).

**Returns:**
- `float`: Time signature ratio

#### `bespoke.get_tempo()`

Get current tempo in BPM.

**Returns:**
- `float`: Tempo in beats per minute

**Example:**
```python
bpm = bespoke.get_tempo()
```

#### `bespoke.reset_transport()`

Reset the transport to measure 0.

### Scale and Pitch

#### `bespoke.get_root()`

Get the current scale root note.

**Returns:**
- `int`: Root note (0-11, where 0=C)

#### `bespoke.get_scale()`

Get the current scale as a list of intervals.

**Returns:**
- `list[int]`: Scale intervals

**Example:**
```python
scale = bespoke.get_scale()  # e.g., [0, 2, 4, 5, 7, 9, 11] for major
```

#### `bespoke.get_scale_range(octave, count)`

Get a range of pitches from the current scale.

**Parameters:**
- `octave` (int): Starting octave
- `count` (int): Number of notes to return

**Returns:**
- `list[int]`: List of MIDI pitches

**Example:**
```python
notes = bespoke.get_scale_range(4, 8)  # 8 notes starting from octave 4
```

#### `bespoke.tone_to_pitch(index)`

Convert scale tone index to MIDI pitch.

**Parameters:**
- `index` (int): Scale tone index

**Returns:**
- `int`: MIDI pitch

**Example:**
```python
pitch = bespoke.tone_to_pitch(0)  # Root note
pitch = bespoke.tone_to_pitch(2)  # Third
```

#### `bespoke.name_to_pitch(noteName)`

Convert note name to MIDI pitch.

**Parameters:**
- `noteName` (str): Note name (e.g., "C4", "F#5", "Bb3")

**Returns:**
- `int`: MIDI pitch

**Example:**
```python
pitch = bespoke.name_to_pitch("C4")  # Returns 60
pitch = bespoke.name_to_pitch("F#5")  # Returns 78
```

#### `bespoke.pitch_to_name(pitch)`

Convert MIDI pitch to note name.

**Parameters:**
- `pitch` (int): MIDI pitch

**Returns:**
- `str`: Note name

**Example:**
```python
name = bespoke.pitch_to_name(60)  # Returns "C4"
```

#### `bespoke.pitch_to_freq(pitch)`

Convert MIDI pitch to frequency in Hz.

**Parameters:**
- `pitch` (float): MIDI pitch (supports microtonal)

**Returns:**
- `float`: Frequency in Hz

**Example:**
```python
freq = bespoke.pitch_to_freq(60)  # Returns 261.63 (middle C)
```

### Utility

#### `bespoke.random(seed, index)`

Generate deterministic random number.

**Parameters:**
- `seed` (int): Random seed
- `index` (int): Index for sequence

**Returns:**
- `float`: Random value (0.0-1.0)

**Example:**
```python
value = bespoke.random(42, 0)  # Always returns same value for same seed/index
```

#### `bespoke.set_background_text(str, size=50, xPos=150, yPos=250, red=1, green=1, blue=1)`

Display text on the canvas background.

**Parameters:**
- `str` (str): Text to display
- `size` (float): Font size (default: 50)
- `xPos` (float): X position (default: 150)
- `yPos` (float): Y position (default: 250)
- `red` (float): Red component 0-1 (default: 1)
- `green` (float): Green component 0-1 (default: 1)
- `blue` (float): Blue component 0-1 (default: 1)

**Example:**
```python
bespoke.set_background_text("Hello!", 100, 200, 300, 1, 0, 0)  # Red text
```

#### `bespoke.get_modules()`

Get list of all module paths in the patch.

**Returns:**
- `list[str]`: List of module paths

**Example:**
```python
modules = bespoke.get_modules()
for module in modules:
    print(module)
```

#### `bespoke.get_controls(path)`

Get list of all visible control paths for a module.

**Parameters:**
- `path` (str): Module path

**Returns:**
- `list[str]`: List of control paths

**Example:**
```python
controls = bespoke.get_controls("oscillator")
for control in controls:
    print(control)
```

### Location Management

#### `bespoke.location_recall(location)`

Recall a saved location/view.

**Parameters:**
- `location` (char): Location character ('a'-'z')

**Example:**
```python
bespoke.location_recall('a')
```

#### `bespoke.location_store(location)`

Store current view as a location.

**Parameters:**
- `location` (char): Location character ('a'-'z')

**Example:**
```python
bespoke.location_store('a')
```

---

## Module-Specific APIs

These modules provide specialized APIs for controlling specific BespokeSynth modules.

### module

Generic module control for any BespokeSynth module.

#### `module.get(path)`

Get reference to a module by path.

**Parameters:**
- `path` (str): Module path

**Returns:**
- `module`: Module reference

**Example:**
```python
osc = module.get("oscillator")
```

#### `module.create(moduleType, x, y)`

Create a new module dynamically.

**Parameters:**
- `moduleType` (str): Type of module to create
- `x` (float): X position
- `y` (float): Y position

**Returns:**
- `module`: Created module reference

**Example:**
```python
new_osc = module.create("oscillator", 100, 200)
```

#### Module Methods

Once you have a module reference, you can call these methods:

##### `module.set_position(x, y)`

Set module position.

**Parameters:**
- `x` (float): X position
- `y` (float): Y position

##### `module.get_position_x()`

Get module X position.

**Returns:**
- `float`: X position

##### `module.get_position_y()`

Get module Y position.

**Returns:**
- `float`: Y position

##### `module.get_width()`

Get module width.

**Returns:**
- `float`: Width in pixels

##### `module.get_height()`

Get module height.

**Returns:**
- `float`: Height in pixels

##### `module.set_target(target)`

Set module's cable target.

**Parameters:**
- `target` (module or str): Target module or path

##### `module.get_target()`

Get module's primary cable target.

**Returns:**
- `module`: Target module

##### `module.get_targets()`

Get all cable targets.

**Returns:**
- `list[module]`: List of target modules

##### `module.set_name(name)`

Set module name.

**Parameters:**
- `name` (str): New name

##### `module.delete()`

Delete the module.

##### `module.set(path, value)`

Set a control value on the module.

**Parameters:**
- `path` (str): Control name
- `value` (float): Value to set

##### `module.get(path)`

Get a control value from the module.

**Parameters:**
- `path` (str): Control name

**Returns:**
- `float`: Control value

##### `module.adjust(path, amount)`

Adjust a control by a relative amount.

**Parameters:**
- `path` (str): Control name
- `amount` (float): Amount to adjust

##### `module.set_focus(zoom)`

Focus the view on this module.

**Parameters:**
- `zoom` (bool): Whether to zoom in

---

### notesequencer

Control note step sequencer modules.

#### `notesequencer.get(path)`

Get reference to a note sequencer.

**Parameters:**
- `path` (str): Sequencer module path

**Returns:**
- `notesequencer`: Sequencer reference

**Example:**
```python
seq = notesequencer.get("notesequencer")
```

#### `notesequencer.set_step(step, row, velocity=127, length=1.0)`

Set a step in the sequencer.

**Parameters:**
- `step` (int): Step index
- `row` (int): Row index (0 = bottom)
- `velocity` (int): Note velocity (0-127, 0 = off)
- `length` (float): Note length multiplier (default: 1.0)

**Example:**
```python
seq = notesequencer.get("notesequencer")
seq.set_step(0, 0, 127)  # Set first step, bottom row
seq.set_step(4, 2, 100, 0.5)  # Step 4, row 2, half length
```

#### `notesequencer.set_pitch(step, pitch, velocity=127, length=1.0)`

Set a step by MIDI pitch.

**Parameters:**
- `step` (int): Step index
- `pitch` (int): MIDI pitch
- `velocity` (int): Note velocity (0-127, 0 = off)
- `length` (float): Note length multiplier (default: 1.0)

**Example:**
```python
seq = notesequencer.get("notesequencer")
seq.set_pitch(0, 60, 127)  # Middle C on first step
```

---

### drumsequencer

Control drum step sequencer modules.

#### `drumsequencer.get(path)`

Get reference to a drum sequencer.

**Parameters:**
- `path` (str): Sequencer module path

**Returns:**
- `drumsequencer`: Sequencer reference

**Example:**
```python
drums = drumsequencer.get("drumsequencer")
```

#### `drumsequencer.set(step, pitch, velocity)`

Set a drum step.

**Parameters:**
- `step` (int): Step index
- `pitch` (int): Drum pitch/row
- `velocity` (int): Velocity (0-127, 0 = off)

**Example:**
```python
drums = drumsequencer.get("drumsequencer")
drums.set(0, 36, 127)  # Kick on step 0
drums.set(4, 38, 100)  # Snare on step 4
```

#### `drumsequencer.get(step, pitch)`

Get velocity of a drum step.

**Parameters:**
- `step` (int): Step index
- `pitch` (int): Drum pitch/row

**Returns:**
- `int`: Velocity (0-127)

**Example:**
```python
vel = drums.get(0, 36)
```

---

### basslinesequencer

Control bassline sequencer modules (TB-303 style).

#### `basslinesequencer.get(path)`

Get reference to a bassline sequencer.

**Parameters:**
- `path` (str): Sequencer module path

**Returns:**
- `basslinesequencer`: Sequencer reference

#### StepVelocityType Enum

- `basslinesequencer.StepVelocityType.Off` - Step is off
- `basslinesequencer.StepVelocityType.Ghost` - Ghost note
- `basslinesequencer.StepVelocityType.Normal` - Normal velocity
- `basslinesequencer.StepVelocityType.Accent` - Accented note

#### `basslinesequencer.set_step(step, tone, velocity, tie)`

Set a bassline step.

**Parameters:**
- `step` (int): Step index
- `tone` (int): Scale tone index
- `velocity` (StepVelocityType): Velocity type
- `tie` (bool): Whether step is tied to next

**Example:**
```python
bass = basslinesequencer.get("bassline")
bass.set_step(0, 0, basslinesequencer.StepVelocityType.Accent, False)
bass.set_step(1, 2, basslinesequencer.StepVelocityType.Normal, True)
```

---

### notecanvas

Control note canvas modules (piano roll).

#### `notecanvas.get(path)`

Get reference to a note canvas.

**Parameters:**
- `path` (str): Canvas module path

**Returns:**
- `notecanvas`: Canvas reference

**Example:**
```python
canvas = notecanvas.get("notecanvas")
```

#### `notecanvas.add_note(measurePos, pitch, velocity, length)`

Add a note to the canvas.

**Parameters:**
- `measurePos` (float): Position in measures
- `pitch` (int): MIDI pitch
- `velocity` (int): Note velocity (0-127)
- `length` (float): Note length in measures

**Example:**
```python
canvas = notecanvas.get("notecanvas")
canvas.add_note(0.0, 60, 100, 0.25)  # Add quarter note at start
canvas.add_note(0.5, 64, 100, 0.25)  # Add note at half measure
```

#### `notecanvas.clear()`

Clear all notes from the canvas.

**Example:**
```python
canvas.clear()
```

#### `notecanvas.fit()`

Fit the view to show all notes.

**Example:**
```python
canvas.fit()
```

---

### sampleplayer

Control sample player modules.

#### `sampleplayer.get(path)`

Get reference to a sample player.

**Parameters:**
- `path` (str): Sample player module path

**Returns:**
- `sampleplayer`: Sample player reference

**Example:**
```python
sampler = sampleplayer.get("sampleplayer")
```

#### `sampleplayer.set_cue_point(pitch, startSeconds, lengthSeconds, speed)`

Set a cue point for a MIDI pitch.

**Parameters:**
- `pitch` (int): MIDI pitch to trigger cue
- `startSeconds` (float): Start position in seconds
- `lengthSeconds` (float): Length in seconds
- `speed` (float): Playback speed multiplier

**Example:**
```python
sampler = sampleplayer.get("sampleplayer")
sampler.set_cue_point(60, 0.0, 1.0, 1.0)  # C4 plays first second
sampler.set_cue_point(62, 1.0, 0.5, 1.5)  # D4 plays next half second at 1.5x speed
```

#### `sampleplayer.fill(data)`

Fill sample player with audio data.

**Parameters:**
- `data` (list[float]): Audio sample data (-1.0 to 1.0)

**Example:**
```python
import math
# Generate a sine wave
samples = [math.sin(i * 0.1) for i in range(44100)]
sampler.fill(samples)
```

#### `sampleplayer.play_cue(cue, speedMult=1, startOffsetSeconds=0)`

Play a cue point directly.

**Parameters:**
- `cue` (int): Cue index
- `speedMult` (float): Speed multiplier (default: 1)
- `startOffsetSeconds` (float): Start offset in seconds (default: 0)

**Example:**
```python
sampler.play_cue(0, 1.0, 0.0)
```

#### `sampleplayer.get_length_seconds()`

Get total sample length in seconds.

**Returns:**
- `float`: Length in seconds

**Example:**
```python
length = sampler.get_length_seconds()
```

---

### drumplayer

Control drum player modules.

#### `drumplayer.get(path)`

Get reference to a drum player.

**Parameters:**
- `path` (str): Drum player module path

**Returns:**
- `drumplayer`: Drum player reference

#### `drumplayer.import_sampleplayer_cue(samplePlayer, srcCueIndex, destHitIndex)`

Import a cue point from a sample player.

**Parameters:**
- `samplePlayer` (sampleplayer): Source sample player
- `srcCueIndex` (int): Source cue index
- `destHitIndex` (int): Destination hit index

**Example:**
```python
sampler = sampleplayer.get("sampleplayer")
drums = drumplayer.get("drumplayer")
drums.import_sampleplayer_cue(sampler, 0, 0)
```

---

### grid

Control grid modules (for grid controllers like Launchpad).

#### `grid.get(path)`

Get reference to a grid module.

**Parameters:**
- `path` (str): Grid module path

**Returns:**
- `grid`: Grid reference

**Example:**
```python
g = grid.get("grid")
```

#### `grid.set(col, row, value)`

Set a grid cell value.

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index
- `value` (float): Cell value (0.0-1.0)

**Example:**
```python
g = grid.get("grid")
g.set(0, 0, 1.0)  # Set top-left cell to max
```

#### `grid.get(col, row)`

Get a grid cell value.

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index

**Returns:**
- `float`: Cell value (0.0-1.0)

**Example:**
```python
value = g.get(0, 0)
```

#### `grid.set_grid(cols, rows)`

Set grid dimensions.

**Parameters:**
- `cols` (int): Number of columns
- `rows` (int): Number of rows

**Example:**
```python
g.set_grid(8, 8)
```

#### `grid.set_label(row, label)`

Set row label.

**Parameters:**
- `row` (int): Row index
- `label` (str): Label text

**Example:**
```python
g.set_label(0, "Kick")
g.set_label(1, "Snare")
```

#### `grid.set_color(colorIndex, r, g, b)`

Set a color in the grid's palette.

**Parameters:**
- `colorIndex` (int): Color index
- `r` (float): Red (0.0-1.0)
- `g` (float): Green (0.0-1.0)
- `b` (float): Blue (0.0-1.0)

**Example:**
```python
g.set_color(1, 1.0, 0.0, 0.0)  # Red
g.set_color(2, 0.0, 1.0, 0.0)  # Green
```

#### `grid.highlight_cell(col, row, delay, duration, colorIndex=1)`

Highlight a cell temporarily.

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index
- `delay` (float): Delay before highlight (measures)
- `duration` (float): Highlight duration (measures)
- `colorIndex` (int): Color index (default: 1)

**Example:**
```python
g.highlight_cell(0, 0, 0, 0.25, 1)  # Flash cell
```

#### `grid.set_division(division)`

Set grid timing division.

**Parameters:**
- `division` (int): Division (e.g., 16 for 16th notes)

#### `grid.set_momentary(momentary)`

Set momentary mode (buttons spring back).

**Parameters:**
- `momentary` (bool): Momentary mode enabled

#### `grid.set_cell_color(col, row, colorIndex)`

Set a cell's color.

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index
- `colorIndex` (int): Color index

**Example:**
```python
g.set_cell_color(0, 0, 1)
```

#### `grid.get_cell_color(col, row)`

Get a cell's color index.

**Parameters:**
- `col` (int): Column index
- `row` (int): Row index

**Returns:**
- `int`: Color index

#### `grid.add_listener(script)`

Add a script as a listener for grid events.

**Parameters:**
- `script` (scriptmodule): Script module to receive events

**Example:**
```python
g = grid.get("grid")
g.add_listener(me.me())

def on_grid_button(col, row, velocity):
    print(f"Button {col},{row} = {velocity}")
```

#### `grid.clear()`

Clear all grid cells.

---

### midicontroller

Control MIDI controller modules.

#### `midicontroller.get(path)`

Get reference to a MIDI controller.

**Parameters:**
- `path` (str): MIDI controller module path

**Returns:**
- `midicontroller`: MIDI controller reference

#### Message Type Constants

- `midicontroller.Note` - Note message
- `midicontroller.Control` - Control Change message
- `midicontroller.Program` - Program Change message
- `midicontroller.PitchBend` - Pitch Bend message

#### Control Type Constants

- `midicontroller.Slider` - Slider control
- `midicontroller.SetValue` - Set value directly
- `midicontroller.Toggle` - Toggle control
- `midicontroller.Direct` - Direct control
- `midicontroller.SetValueOnRelease` - Set on release
- `midicontroller.Default` - Default behavior

#### `midicontroller.set_connection(messageType, control, controlPath, controlType=Default, value=0, channel=-1, page=0, midi_off=0, midi_on=127, scale=False, blink=False, increment=0, twoway=True, feedbackControl=-1, isPageless=False)`

Set up a MIDI controller connection.

**Parameters:**
- `messageType`: Message type constant
- `control` (int): MIDI control number
- `controlPath` (str): Path to control
- `controlType`: Control type constant (default: Default)
- `value` (float): Initial value (default: 0)
- `channel` (int): MIDI channel, -1 for all (default: -1)
- `page` (int): Controller page (default: 0)
- `midi_off` (int): MIDI value for off (default: 0)
- `midi_on` (int): MIDI value for on (default: 127)
- `scale` (bool): Scale value (default: False)
- `blink` (bool): Blink LED (default: False)
- `increment` (float): Increment amount (default: 0)
- `twoway` (bool): Two-way communication (default: True)
- `feedbackControl` (int): Feedback control number (default: -1)
- `isPageless` (bool): Pageless control (default: False)

**Example:**
```python
controller = midicontroller.get("midicontroller")
controller.set_connection(midicontroller.Control, 1, "oscillator~volume")
```

#### `midicontroller.send_note(pitch, velocity, forceNoteOn=False, channel=-1, page=0)`

Send a MIDI note message.

**Parameters:**
- `pitch` (int): MIDI pitch
- `velocity` (int): Velocity
- `forceNoteOn` (bool): Force note on (default: False)
- `channel` (int): MIDI channel (default: -1)
- `page` (int): Page (default: 0)

#### `midicontroller.send_cc(ctl, value, channel=-1, page=0)`

Send a MIDI CC message.

**Parameters:**
- `ctl` (int): CC number
- `value` (int): CC value
- `channel` (int): MIDI channel (default: -1)
- `page` (int): Page (default: 0)

#### `midicontroller.send_program_change(program, channel=-1, page=0)`

Send a program change message.

**Parameters:**
- `program` (int): Program number
- `channel` (int): MIDI channel (default: -1)
- `page` (int): Page (default: 0)

#### `midicontroller.send_pitchbend(bend, channel=-1, page=0)`

Send a pitch bend message.

**Parameters:**
- `bend` (int): Pitch bend value
- `channel` (int): MIDI channel (default: -1)
- `page` (int): Page (default: 0)

#### `midicontroller.send_data(a, b, c, page=0)`

Send raw MIDI data.

**Parameters:**
- `a` (int): First byte
- `b` (int): Second byte
- `c` (int): Third byte
- `page` (int): Page (default: 0)

#### `midicontroller.send_sysex(data, page=0)`

Send SysEx data.

**Parameters:**
- `data` (bytes): SysEx data
- `page` (int): Page (default: 0)

#### `midicontroller.add_script_listener(script)`

Add a script to receive MIDI events.

**Parameters:**
- `script` (scriptmodule): Script module

**Example:**
```python
controller = midicontroller.get("midicontroller")
controller.add_script_listener(me.me())

def on_midi(messageType, control, value, channel):
    print(f"MIDI: {messageType} {control} {value} {channel}")
```

#### `midicontroller.resync_controller_state()`

Resync controller state (send all current values).

---

### osccontroller

Control OSC controller modules.

#### `osccontroller.get(path)`

Get reference to an OSC controller.

**Parameters:**
- `path` (str): OSC controller module path

**Returns:**
- `osccontroller`: OSC controller reference

#### `osccontroller.add_control(address, isFloat)`

Add an OSC control.

**Parameters:**
- `address` (str): OSC address
- `isFloat` (bool): Whether value is float (vs int)

**Example:**
```python
osc = osccontroller.get("osccontroller")
osc.add_control("/filter/cutoff", True)
```

---

### oscoutput

Control OSC output modules.

#### `oscoutput.get(path)`

Get reference to an OSC output.

**Parameters:**
- `path` (str): OSC output module path

**Returns:**
- `oscoutput`: OSC output reference

#### `oscoutput.send_float(address, val)`

Send a float value via OSC.

**Parameters:**
- `address` (str): OSC address
- `val` (float): Float value

**Example:**
```python
osc_out = oscoutput.get("oscoutput")
osc_out.send_float("/synth/volume", 0.8)
```

#### `oscoutput.send_int(address, val)`

Send an integer value via OSC.

**Parameters:**
- `address` (str): OSC address
- `val` (int): Integer value

#### `oscoutput.send_string(address, val)`

Send a string value via OSC.

**Parameters:**
- `address` (str): OSC address
- `val` (str): String value

---

### envelope

Control envelope modulator modules.

#### `envelope.get(path)`

Get reference to an envelope.

**Parameters:**
- `path` (str): Envelope module path

**Returns:**
- `envelope`: Envelope reference

#### `envelope.start(stages)`

Start an envelope with specified stages.

**Parameters:**
- `stages` (list): List of [time, value] pairs

**Example:**
```python
env = envelope.get("envelope")
env.start([[0, 0], [0.1, 1], [0.5, 0.5], [1.0, 0]])
```

#### `envelope.schedule(delay, stages)`

Schedule an envelope to start after a delay.

**Parameters:**
- `delay` (float): Delay in measures
- `stages` (list): List of [time, value] pairs

**Example:**
```python
env.schedule(0.25, [[0, 0], [0.1, 1], [0.5, 0]])
```

---

### snapshots

Control snapshot modules (preset management).

#### `snapshots.get(path)`

Get reference to a snapshots module.

**Parameters:**
- `path` (str): Snapshots module path

**Returns:**
- `snapshots`: Snapshots reference

#### `snapshots.get_size()`

Get number of snapshot slots.

**Returns:**
- `int`: Number of slots

#### `snapshots.get_current_snapshot()`

Get current snapshot index.

**Returns:**
- `int`: Current snapshot index

#### `snapshots.has_snapshot(index)`

Check if a snapshot exists at index.

**Parameters:**
- `index` (int): Snapshot index

**Returns:**
- `bool`: True if snapshot exists

#### `snapshots.set_snapshot(index)`

Recall a snapshot.

**Parameters:**
- `index` (int): Snapshot index

**Example:**
```python
snaps = snapshots.get("snapshots")
snaps.set_snapshot(0)
```

#### `snapshots.store_snapshot(index, label="")`

Store current state as a snapshot.

**Parameters:**
- `index` (int): Snapshot index
- `label` (str): Optional label (default: "")

**Example:**
```python
snaps.store_snapshot(0, "My Preset")
```

#### `snapshots.delete_snapshot(index)`

Delete a snapshot.

**Parameters:**
- `index` (int): Snapshot index

---

### vstplugin

Control VST plugin modules.

#### `vstplugin.get(path)`

Get reference to a VST plugin.

**Parameters:**
- `path` (str): VST plugin module path

**Returns:**
- `vstplugin`: VST plugin reference

#### `vstplugin.send_cc(ctl, value, channel)`

Send CC to VST plugin.

**Parameters:**
- `ctl` (int): CC number
- `value` (int): CC value
- `channel` (int): MIDI channel

#### `vstplugin.send_program_change(program, channel)`

Send program change to VST plugin.

**Parameters:**
- `program` (int): Program number
- `channel` (int): MIDI channel

#### `vstplugin.send_data(a, b, c)`

Send raw MIDI data to VST plugin.

**Parameters:**
- `a` (int): First byte
- `b` (int): Second byte
- `c` (int): Third byte

---

### linnstrument

Control Linnstrument modules.

#### `linnstrument.get(path)`

Get reference to a Linnstrument.

**Parameters:**
- `path` (str): Linnstrument module path

**Returns:**
- `linnstrument`: Linnstrument reference

#### Color Constants

- `linnstrument.Off` - Off
- `linnstrument.Red` - Red
- `linnstrument.Yellow` - Yellow
- `linnstrument.Green` - Green
- `linnstrument.Cyan` - Cyan
- `linnstrument.Blue` - Blue
- `linnstrument.Magenta` - Magenta
- `linnstrument.Black` - Black
- `linnstrument.White` - White
- `linnstrument.Orange` - Orange
- `linnstrument.Lime` - Lime
- `linnstrument.Pink` - Pink

#### `linnstrument.set_color(x, y, color)`

Set LED color on Linnstrument.

**Parameters:**
- `x` (int): X position
- `y` (int): Y position
- `color`: Color constant

**Example:**
```python
linn = linnstrument.get("linnstrument")
linn.set_color(0, 0, linnstrument.Red)
linn.set_color(1, 0, linnstrument.Green)
```

---

## See Also

- [Python Scripting Overview](README.md)
- [Scripting Examples](examples.md)
- [Getting Started Guide](../getting-started.md)


