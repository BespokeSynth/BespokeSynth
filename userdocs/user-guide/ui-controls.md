# UI Controls

Master BespokeSynth's interface controls for efficient workflow.

## Control Types

BespokeSynth modules contain various control types:

| Control | Description | Example |
|---------|-------------|---------|
| **Slider** | Continuous value adjustment | Volume, frequency, filter cutoff |
| **Button** | Trigger action or toggle state | Play, stop, randomize |
| **Dropdown** | Select from list of options | Waveform, scale type, MIDI device |
| **Text Entry** | Enter text or numbers | Module name, file path |
| **Grid** | 2D control surface | Step sequencer, drum pattern |
| **Canvas** | Visual editing area | Note canvas, sample editor |

## Sliders

Sliders are the most common control type.

### Basic Adjustment

**Click and drag**:
1. Click on slider
2. Drag left/right (horizontal) or up/down (vertical)
3. Release when desired value reached

**Click to set**:
1. Click at desired position on slider
2. Value jumps to that position

### Fine-Tuning

**Shift + drag**:
1. Hold `Shift`
2. Drag slider
3. Movement is much slower/more precise

**Best for**: Precise adjustments, small value changes

### Typing Values

**Enter exact value**:
1. Hover mouse over slider
2. Type desired number (e.g., `1.5`)
3. Press `Enter`
4. Slider jumps to that value

**Examples**:
- Type `440` for frequency
- Type `0.5` for halfway
- Type `1/16` for sixteenth note

### Expressions

**Math expressions**:
1. Hover over slider
2. Type expression
3. Press `Enter`

**Supported expressions**:
- `1/16` - Division (sixteenth note)
- `+=10` - Add to current value
- `-=5` - Subtract from current value
- `*=2` - Multiply current value
- `/=2` - Divide current value

### Keyboard Adjustments

While hovering over slider:

| Key | Action |
|-----|--------|
| `↑` | Increase by 1 |
| `↓` | Decrease by 1 |
| `Shift + ↑` | Increase by 0.01 |
| `Shift + ↓` | Decrease by 0.01 |
| `[` | Halve value |
| `]` | Double value |
| `\` | Reset to default |

### Mouse Wheel

**Scroll to adjust**:
1. Hover over slider
2. Scroll mouse wheel or trackpad
3. Value changes incrementally

**Fine-tune**:
- Hold `Shift` while scrolling for smaller increments

### Slew Amount

Add smoothing to slider changes.

**How to adjust slew**:
1. Hold `Alt` (Windows/Linux) or `Option` (macOS)
2. Drag slider
3. Slew amount changes (smoothing)

**What is slew?**
- Smooths abrupt value changes
- Creates gradual transitions
- Useful for automation and modulation

### Range Adjustment

Change a slider's minimum and maximum values.

**Adjust minimum**:
1. Hold `Ctrl` (Windows/Linux) or `Cmd` (macOS)
2. Click lower half of slider
3. Minimum value adjusts

**Adjust maximum**:
1. Hold `Ctrl/Cmd`
2. Click upper half of slider
3. Maximum value adjusts

**Warning**: Increasing range beyond intended values can cause unpredictable behavior!

## Modulation

Add dynamic movement to any slider.

### Adding LFO Modulation

**Right-click method**:
1. Right-click on slider
2. Select "add LFO"
3. Slider turns green
4. Built-in LFO modulates parameter

### Connecting External Modulator

**Cable method**:
1. Create modulator module (e.g., **lfo**, **envelope**)
2. Click modulator's output circle
3. Drag to target slider
4. Release on slider
5. Slider turns green

### Adjusting Modulation Range

**Modulated sliders** (green) have two adjustment modes:

**Vertical drag**: Adjust minimum value
- How low modulation goes
- Bottom of modulation range

**Horizontal drag**: Adjust maximum value
- How high modulation goes
- Top of modulation range

**Example**:
- Drag down: Modulation goes from 0 to current value
- Drag right: Modulation goes from current value to max

### Removing Modulation

1. Right-click modulated slider
2. Select "remove modulation"
3. Slider returns to normal (non-green)

## Buttons

Buttons trigger actions or toggle states.

### Click Buttons
- Single click to activate
- Examples: Randomize, clear, trigger

### Toggle Buttons
- Click to turn on/off
- Shows current state (lit/unlit)
- Examples: Enabled, mute, solo

### Hold Buttons
- Hold to activate
- Release to deactivate
- Examples: Some performance controls

## Dropdowns

Select from a list of options.

### Using Dropdowns

1. Click dropdown control
2. Menu appears with options
3. Click desired option
4. Menu closes, option selected

### Common Dropdowns

| Module | Dropdown | Options |
|--------|----------|---------|
| **midicontroller** | controller | Available MIDI devices |
| **oscillator** | type | Sine, saw, square, triangle |
| **scale** | scale | Major, minor, chromatic, etc. |
| **notesequencer** | notemode | Chromatic, scale, pentatonic |

## Text Entry

Enter text or numbers directly.

### How to Use

1. Click on text field
2. Type desired text/number
3. Press `Enter` to confirm
4. Or click outside field to confirm

### Common Text Fields

- Module names (triangle menu)
- File paths
- Script code
- Comments and labels

## Grids

2D control surfaces for patterns and sequences.

### Step Sequencer Grids

**Click to toggle**:
- Click cell to add/remove step
- Lit cells are active steps

**Shift + drag**:
- Adjust step velocity
- Drag vertically on cell

**Hover + arrows**:
- Move steps with arrow keys
- Precise positioning

### Grid Controllers

When using external grid controllers (e.g., Launchpad):
- Physical pads map to grid
- LED feedback shows state
- Tactile control

## Canvases

Visual editing areas for complex data.

### Note Canvas

**Add notes**:
- Shift + click to add note
- Shift + drag to duplicate

**Edit notes**:
- Drag to move
- Drag edges to resize
- Alt + drag for no snapping

**Navigate**:
- Shift + scroll to zoom
- Alt + drag to pan
- Ctrl + drag to zoom

### Sample Canvas

**Edit waveform**:
- Click and drag to edit
- Zoom in for precision
- Undo/redo available

## Control Navigation

Move between controls without mouse.

### Tab Navigation

**Cycle forward**:
- Press `Tab`
- Moves to next control on module

**Cycle backward**:
- Press `Shift + Tab`
- Moves to previous control

### Arrow Navigation

**Move between controls**:
- `Ctrl/Cmd + ↑↓←→`
- Navigate to adjacent controls
- Works within module

## ADSR Controls

Envelope controls can be displayed as sliders or graphical.

### Toggle Display Mode

**Press F2**:
- Switches between slider and graphical display
- Affects all ADSR controls globally
- Choose preferred workflow

**Slider mode**:
- Four separate sliders (A, D, S, R)
- Precise numeric control

**Graphical mode**:
- Visual envelope shape
- Drag points to adjust
- See envelope curve

## Tips and Tricks

### Efficient Control

1. **Learn keyboard shortcuts**: Faster than mouse for many operations
2. **Use Tab navigation**: Quick control access
3. **Type values**: Faster than dragging for exact values
4. **Use expressions**: Quick math without calculator
5. **Right-click explore**: Many hidden options

### Precision Techniques

1. **Shift for fine-tuning**: Always available
2. **Type exact values**: No guessing
3. **Use arrow keys**: Incremental adjustments
4. **Zoom in on canvases**: Better precision
5. **Reset with backslash**: Quick return to default

### Modulation Mastery

1. **Modulate everything**: Any slider can be modulated
2. **Adjust ranges carefully**: Set min/max for musical results
3. **Layer modulation**: Multiple LFOs on different parameters
4. **Use slew**: Smooth out modulation
5. **Experiment**: Try unexpected modulation targets

## Common Workflows

### Sound Design

1. Start with default values (press `\` on sliders)
2. Adjust main parameters (cutoff, resonance, etc.)
3. Add modulation for movement
4. Fine-tune with Shift + drag
5. Save preset when satisfied

### Performance Setup

1. Map important controls to MIDI controller
2. Use snapshots for preset changes
3. Keep frequently-used controls visible
4. Use Tab to navigate quickly
5. Practice control changes before performing

### Automation

1. Use modulators for dynamic changes
2. Set appropriate modulation ranges
3. Add slew for smooth transitions
4. Layer multiple modulators
5. Record automation with script module

## Keyboard Shortcuts Summary

### Slider Controls (while hovering)

| Shortcut | Action |
|----------|--------|
| `Click + Drag` | Adjust value |
| `Shift + Drag` | Fine-tune |
| `Type + Enter` | Set exact value |
| `↑` / `↓` | Increment/decrement by 1 |
| `Shift + ↑↓` | Increment/decrement by 0.01 |
| `[` | Halve value |
| `]` | Double value |
| `\` | Reset to default |
| `Scroll` | Adjust value |
| `Shift + Scroll` | Fine-tune |
| `Alt/Option + Drag` | Adjust slew |
| `Ctrl/Cmd + Click` | Adjust range |
| `Right-click` | Add LFO modulation |

### Navigation

| Shortcut | Action |
|----------|--------|
| `Tab` | Next control |
| `Shift + Tab` | Previous control |
| `Ctrl/Cmd + Arrows` | Navigate controls |
| `F2` | Toggle ADSR display |

## Troubleshooting

### Slider Won't Move

**Check**:
- Is module enabled?
- Is slider modulated? (green = modulated)
- Is value at min/max already?

### Can't Type Value

**Check**:
- Is mouse hovering over slider?
- Are you clicking first? (don't click, just hover and type)
- Is value within slider's range?

### Modulation Not Working

**Check**:
- Is modulator connected?
- Is modulation range set correctly?
- Is modulator actually outputting signal?
- Is slider green (modulated)?

### Control Not Responding

**Check**:
- Is module enabled?
- Is control visible? (some modules hide controls)
- Try clicking directly on control
- Restart BespokeSynth if persistent

## Next Steps

Learn about saving your work:
- **[Saving & Loading](saving-loading.md)** - Manage projects
- **[Module Reference](../modules/README.md)** - Explore module-specific controls
- **[Scripting](../scripting/README.md)** - Automate controls with Python

