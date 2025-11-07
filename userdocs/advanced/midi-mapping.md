# MIDI Mapping

Complete guide to mapping MIDI controllers to BespokeSynth parameters.

## Overview

BespokeSynth supports comprehensive MIDI controller integration, allowing you to map hardware controllers to any parameter in your patch. This enables hands-on, tactile control of your music.

## Quick Start

### Basic MIDI Mapping

1. **Create a MIDI Controller Module**
   - Right-click canvas → **midicontroller**
   - Select your MIDI device from the dropdown

2. **Enable Bind Mode**
   - Click the **bind** checkbox on the midicontroller module
   - The module enters "MIDI learn" mode

3. **Map a Control**
   - Click on any parameter you want to control
   - Move a knob/fader/button on your MIDI controller
   - The mapping is created automatically!

4. **Test the Mapping**
   - Disable bind mode
   - Move the controller - the parameter should respond

## MIDI Controller Module

### Setup

**Creating the Module:**
```
Right-click → midicontroller
```

**Key Controls:**
- **device in** - Select MIDI input device
- **device out** - Select MIDI output device (for feedback)
- **bind** - Enable/disable MIDI learn mode
- **channel** - Filter by MIDI channel (or "any")

### Connection Types

The midicontroller module can map:
- **CC (Control Change)** - Knobs, faders, buttons
- **Notes** - Keyboard keys, pads
- **Pitch Bend** - Pitch wheel
- **Program Change** - Program select buttons

### Mapping Methods

#### Method 1: MIDI Learn (Recommended)

1. Enable **bind** mode
2. Click target parameter
3. Move MIDI control
4. Mapping created!

#### Method 2: Shift+Hover

1. Enable **bind** mode
2. Hold **Shift** key
3. Hover over target parameter
4. Move MIDI control
5. Mapping created without clicking!

#### Method 3: Manual Connection

1. Right-click midicontroller module
2. Select **add connection**
3. Configure:
   - Message type (CC, Note, etc.)
   - Control number
   - Target parameter path
   - Value range

## Advanced Mapping

### Velocity Multiplier

Control the sensitivity of incoming MIDI:

```
velocitymult: 1.0  (default)
velocitymult: 0.5  (half sensitivity)
velocitymult: 2.0  (double sensitivity)
```

Set in the module's properties or save file.

### Channel Filtering

**Filter by specific channel:**
- Set **channel** dropdown to 01-16
- Only messages on that channel are processed

**Use any channel:**
- Set **channel** to "any"
- All channels are processed

### Use Channel as Voice

Enable **usechannelasvoice** to:
- Route each MIDI channel to a separate voice
- Useful for MPE (MIDI Polyphonic Expression)
- Each channel gets independent pitch bend and modulation

### 14-bit CC Support

For high-resolution controllers:
- BespokeSynth automatically detects 14-bit CC pairs
- CC 0-31 (MSB) paired with CC 32-63 (LSB)
- Provides 16,384 steps instead of 128

### Mapping Ranges

**Control parameter range:**
- Most parameters map 0-127 to their full range
- Use **min** and **max** properties to limit range
- Example: Map CC to only affect 0.5-1.0 of a parameter

### Multiple Controllers

**Use multiple MIDI devices:**
1. Create multiple midicontroller modules
2. Each connects to a different device
3. Map different controls to different parameters
4. All work simultaneously

## Controller Layouts

### Custom Controller Definitions

BespokeSynth supports custom controller layouts for grid controllers.

**Location:**
```
resource/userdata_original/controllers/
```

**Supported Controllers:**
- Launchpad (all models)
- APC Key 25
- Novation Circuit
- Monome
- Push 2
- Custom controllers (JSON definition)

### Creating Custom Layouts

**JSON Format:**
```json
{
  "groups": [
    {
      "rows": 8,
      "cols": 8,
      "position": [0, 35],
      "dimensions": [28, 28],
      "spacing": [30, 30],
      "controls": [81, 82, 83, ...],
      "colors": [0, 52, 53, 20, 21, 8, 9],
      "messageType": "note",
      "drawType": "button"
    }
  ]
}
```

**Properties:**
- **rows/cols** - Grid dimensions
- **position** - UI position [x, y]
- **dimensions** - Button size [width, height]
- **spacing** - Gap between buttons [x, y]
- **controls** - MIDI note/CC numbers
- **colors** - Color palette indices
- **messageType** - "note", "control", "program", "pitchbend"
- **drawType** - "button", "slider"

## MPE (MIDI Polyphonic Expression)

### What is MPE?

MPE allows per-note control of:
- Pitch bend
- Pressure (aftertouch)
- Timbre (CC 74)

Each note gets its own MIDI channel for independent expression.

### Setup MPE

1. Create midicontroller module
2. Enable **usechannelasvoice**
3. Connect MPE controller (Roli Seaboard, Linnstrument, etc.)
4. Each note gets independent modulation

### MPE-Compatible Modules

- **synth** - Responds to per-voice pitch bend
- **vstplugin** - If VST supports MPE
- **sampler** - Per-voice modulation

## MIDI Output / Feedback

### Sending MIDI Out

**Use cases:**
- Light up controller LEDs
- Send feedback to controller
- Control external hardware

**Setup:**
1. Select **device out** on midicontroller
2. Controller automatically sends feedback
3. LEDs/displays update to match parameter values

### Supported Feedback

- **CC values** - Fader/knob positions
- **Note on/off** - Button states
- **Program change** - Preset selection
- **SysEx** - Custom messages (advanced)

## Grid Controllers

### Grid Module Integration

Grid controllers work with the **grid** module:

```
midicontroller → grid → (your patch)
```

**Features:**
- Visual feedback on controller
- Color-coded buttons
- Step sequencing
- Performance control

### Launchpad Example

1. Create **midicontroller** (select Launchpad)
2. Create **grid** module
3. Connect midicontroller to grid
4. Grid automatically maps to Launchpad layout
5. Use for step sequencing, clip launching, etc.

## Troubleshooting

### Controller Not Detected

**Check:**
- Controller is powered on
- USB cable connected
- Driver installed (Windows)
- Device appears in OS MIDI settings

**macOS:**
- Open Audio MIDI Setup
- Window → Show MIDI Studio
- Verify device appears

**Windows:**
- Check Device Manager
- Verify MIDI device listed
- Install manufacturer drivers

### Mapping Not Working

**Check:**
- Bind mode is enabled
- Correct MIDI channel selected
- Parameter is mappable (some aren't)
- MIDI messages are being received (check console)

### Wrong Parameter Responding

**Fix:**
- Delete incorrect mapping
- Re-map in bind mode
- Check for duplicate mappings

### Feedback Not Working

**Check:**
- **device out** is selected
- Controller supports MIDI input
- Correct output device selected
- Controller is in correct mode

### Latency Issues

**Solutions:**
- Use ASIO drivers (Windows)
- Reduce audio buffer size
- Close other MIDI applications
- Use USB 2.0/3.0 port (not hub)

## Best Practices

### Organization

1. **One controller per module** - Easier to manage
2. **Name your modules** - "launchpad_drums", "knobs_filter"
3. **Group related mappings** - Keep similar controls together
4. **Save often** - Mappings are saved with patch

### Performance

1. **Map only what you need** - Don't map everything
2. **Use appropriate message types** - CC for continuous, Note for triggers
3. **Limit feedback** - Only send necessary updates
4. **Test before performing** - Verify all mappings work

### Workflow

1. **Build patch first** - Map controls after
2. **Map in logical order** - Left to right, top to bottom
3. **Test each mapping** - Verify before moving on
4. **Document mappings** - Note what controls what

## Common Setups

### DJ-Style Controller

**Map:**
- Faders → Volume controls
- Knobs → Filter cutoff/resonance
- Buttons → Effect on/off
- Crossfader → Mix between sources

### Drum Pad Controller

**Map:**
- Pads → drumsequencer steps
- Knobs → Sample parameters
- Buttons → Mute/solo tracks

### Keyboard Controller

**Map:**
- Keys → Note input
- Mod wheel → Filter modulation
- Pitch bend → Pitch
- Knobs → Synth parameters

### Grid Controller (Launchpad)

**Map:**
- Grid → Step sequencer
- Side buttons → Pattern selection
- Top buttons → Mute/solo
- Bottom buttons → Scene launch

## Advanced Techniques

### Macro Controls

**Create macro knobs:**
1. Map one MIDI control to multiple parameters
2. Use **valuemultiplier** module
3. Control multiple parameters with one knob

### Conditional Mapping

**Use scripts for complex mapping:**
```python
def on_midi(messageType, control, value, channel):
    if control == 1:  # Mod wheel
        if value > 64:
            me.set("filter~cutoff", value / 127.0)
        else:
            me.set("filter~resonance", value / 127.0)
```

### MIDI Learn Shortcuts

**Keyboard shortcuts in bind mode:**
- **Click** - Map to clicked parameter
- **Shift+Hover** - Map to hovered parameter
- **Esc** - Cancel bind mode

### Saving Mappings

**Mappings are saved:**
- In patch file (.bsk)
- With midicontroller module
- Automatically on save

**Share mappings:**
- Save patch
- Share .bsk file
- Others can use same mappings

## Resources

### Example Patches

Check `resource/userdata_original/` for:
- Example controller setups
- Pre-mapped patches
- Controller templates

### Controller Definitions

Located in `resource/userdata_original/controllers/`:
- Launchpad layouts
- APC layouts
- Custom controller JSON files

### Community

- [Discord](https://discord.gg/YdTMkvvpZZ) - Share controller setups
- [GitHub](https://github.com/BespokeSynth/BespokeSynth) - Submit controller definitions
- [Forum](https://github.com/BespokeSynth/BespokeSynth/discussions) - Ask questions

## Next Steps

- **[OSC Control](osc-control.md)** - Network-based control
- **[Performance Tips](performance-tips.md)** - Optimize for live use
- **[Scripting](../scripting/README.md)** - Advanced MIDI processing
