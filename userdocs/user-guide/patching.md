# Patching

Master the art of connecting modules with patch cables in BespokeSynth.

## What is Patching?

Patching is connecting modules together with virtual cables to create signal flow. Like a hardware modular synthesizer, you route signals from sources to destinations.

## Basic Patching

### Method 1: Click and Click
1. Click the circle at bottom of source module
2. Cable appears, following your mouse
3. Click on destination module
4. Cable connects

**Best for**: Precise connections

### Method 2: Drag
1. Click and hold circle at bottom of source module
2. Drag to destination module
3. Release mouse button
4. Cable connects

**Best for**: Quick, fluid workflow

## Signal Types

BespokeSynth uses different signal types, shown by cable colors:

| Type | Color | Description | Flows From | Flows To |
|------|-------|-------------|------------|----------|
| **Notes** | 🟠 Orange | MIDI-style note messages | Instruments, Note Effects | Note Effects, Synths |
| **Audio** | 🔵 Cyan | Audio waveforms | Synths, Audio Effects | Audio Effects, Output |
| **Pulses** | 🟡 Yellow | Trigger/gate signals | Pulse modules | Instruments, Pulse modules |
| **Modulators** | 🟢 Green | Control voltage (CV) | Modulators | Any slider/control |

### Signal Flow Rules

**Notes flow**:
```
Instrument → Note Effect → Synth
```

**Audio flows**:
```
Synth → Audio Effect → Output
```

**Pulses trigger**:
```
Pulse → Instrument/Sequencer
```

**Modulators control**:
```
Modulator → Any parameter
```

## Disconnecting Cables

### Method 1: Drag and Backspace
1. Click and drag cable (from either end)
2. Press `Backspace` while dragging
3. Cable deleted

**Best for**: Quick disconnection

### Method 2: Drag to Empty Space
1. Click and drag cable
2. Drag far from any module
3. Release in empty space
4. Cable deleted

**Best for**: When backspace is inconvenient

## Advanced Patching

### Cable Splitting

Send one output to multiple destinations.

**How to split**:
1. Hold `Shift`
2. Click and drag existing cable's circle
3. Drag to new destination
4. Release

**Result**: One source feeds multiple destinations

**Example uses**:
- Send synth to multiple effects
- Trigger multiple sequencers
- Modulate multiple parameters

### Cable Inserting

Insert a module into an existing connection.

**How to insert**:
1. Hold `Shift`
2. Drag cable from source module
3. Release on destination module while holding Shift
4. Module inserted in signal chain

**Result**: Source → New Module → Original Destination

**Example uses**:
- Add effect to existing audio chain
- Insert note effect between instrument and synth
- Add processing to modulation signal

### Quick Patching

Connect modules while moving them.

**How to quick patch**:
1. Hold `Shift`
2. Drag module by title bar
3. Touch module's output circle to another module
   OR touch mouse pointer to another module's circle
4. Modules connect automatically

**Best for**: Fast workflow, live performance

## Multiple Outputs

Some modules have multiple output circles.

### Understanding Multiple Outputs

**Examples**:
- **send**: Left output (dry), right output (wet)
- **splitter**: Multiple audio outputs
- **noterouter**: Multiple note outputs

**How to use**:
1. Each circle is a separate output
2. Click desired output circle
3. Connect to destination
4. Repeat for other outputs

### Common Multi-Output Modules

| Module | Outputs | Purpose |
|--------|---------|---------|
| **send** | 2 | Dry/wet split for parallel processing |
| **splitter** | 8 | Split audio to multiple destinations |
| **noterouter** | Multiple | Route notes to different synths |
| **pulserouter** | Multiple | Route pulses to different targets |

## Modulation Connections

Modulators connect differently than other signals.

### Connecting Modulators to Parameters

**Method 1: Drag to slider**:
1. Click modulator's output circle
2. Drag to any slider on any module
3. Release on slider
4. Modulation connected (slider turns green)

**Method 2: Right-click slider**:
1. Right-click any slider
2. Select "add LFO"
3. Built-in LFO modulation added

### Adjusting Modulation

**Modulation range**:
- Click and drag green slider **vertically**: Adjust minimum value
- Click and drag green slider **horizontally**: Adjust maximum value

**Modulation amount**:
- Vertical drag sets how low modulation goes
- Horizontal drag sets how high modulation goes

### Removing Modulation

1. Right-click modulated slider
2. Select "remove modulation"
3. Slider returns to normal

## Signal Flow Patterns

### Basic Instrument Chain
```
notesequencer → karplusstrong → output
```

### With Note Effects
```
notesequencer → arpeggiator → chorder → fmsynth → output
```

### With Audio Effects
```
drumsequencer → drumplayer → delay → reverb → output
```

### Parallel Processing
```
                    ┌→ delay ──┐
synth → send ──────┤          ├→ output
                    └→ reverb ─┘
```

### Modulation
```
lfo ──→ filter cutoff
      └→ oscillator pitch
```

### Pulse-Driven
```
pulser → notecounter → notesequencer → synth → output
```

## Cable Management

Keep your patch organized with good cable routing.

### Best Practices

1. **Minimize crossings**: Route cables to avoid tangles
2. **Use short paths**: Place connected modules near each other
3. **Organize spatially**: Arrange modules by signal flow
4. **Split intentionally**: Only split when needed
5. **Delete unused cables**: Clean up as you work

### Visual Organization

**Left to right flow**:
```
[Source] ──→ [Process] ──→ [Destination]
```

**Top to bottom flow**:
```
[Source]
   ↓
[Process]
   ↓
[Destination]
```

**Grouped by function**:
```
┌─────────────┐
│   Drums     │──┐
└─────────────┘  │
                 ├──→ [Output]
┌─────────────┐  │
│   Melody    │──┘
└─────────────┘
```

## Common Patching Patterns

### Layered Synths
```
notesequencer ──┬→ synth1 ──┐
                ├→ synth2 ──┼→ output
                └→ synth3 ──┘
```

### Effect Chain
```
synth → delay → reverb → eq → compressor → output
```

### Feedback Loop (use carefully!)
```
synth → delay ──→ output
         ↑  │
         └──┘ (feedback)
```

### Sidechain
```
kick ──→ output
   └──→ compressor (sidechain) ←── bass
```

### Generative System
```
pulser → random → notesequencer → synth → output
           ↓
         lfo (modulating random)
```

## Troubleshooting Patches

### No Sound

**Check**:
1. Is audio chain connected to **output**?
2. Are all modules enabled?
3. Is volume turned up?
4. Are cables connected correctly?

### Wrong Signal Type

**Symptoms**: Cable won't connect or no effect

**Solution**: Match signal types:
- Notes to note inputs
- Audio to audio inputs
- Pulses to pulse inputs

### Feedback Loop Too Loud

**Symptoms**: Loud, harsh sound or distortion

**Solution**:
1. Disconnect feedback cable immediately
2. Add attenuation (gain module)
3. Use feedback module for controlled feedback
4. Start with low feedback amounts

### Modulation Not Working

**Check**:
1. Is modulator connected to slider?
2. Is slider green (modulated)?
3. Is modulation range set correctly?
4. Is modulator actually outputting signal?

## Tips and Tricks

### Efficient Patching

1. **Plan before patching**: Think about signal flow
2. **Use cable splitting**: One source, multiple destinations
3. **Insert modules**: Add processing to existing chains
4. **Quick patch**: Hold Shift while moving modules
5. **Organize as you go**: Don't wait until patch is messy

### Creative Patching

1. **Experiment with feedback**: Controlled feedback creates interesting textures
2. **Cross-modulation**: Modulate modulators for complex movement
3. **Parallel processing**: Split signals for layered effects
4. **Unusual routings**: Try unexpected connections
5. **Modulate everything**: Any slider can be modulated

### Performance Patching

1. **Keep it simple**: Complex patches are hard to perform
2. **Use snapshots**: Save different routing configurations
3. **Group controls**: Put performance controls together
4. **Minimize cable changes**: Design for stability
5. **Test thoroughly**: Verify all connections before performing

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Disconnect cable | Drag + `Backspace` |
| Split cable | `Shift` + Drag circle |
| Insert module | `Shift` + Release on module |
| Quick patch | `Shift` + Drag module |

## Common Mistakes

### Don't Do This ❌

1. **Uncontrolled feedback loops**: Can damage speakers/ears
2. **Too many splits**: Creates messy, hard-to-follow patches
3. **Ignoring signal types**: Won't work and wastes time
4. **Random connections**: Plan your signal flow
5. **Forgetting to connect to output**: No sound!

### Do This Instead ✅

1. **Use feedback module**: Controlled, safe feedback
2. **Split intentionally**: Only when needed
3. **Match signal types**: Notes to notes, audio to audio
4. **Plan signal flow**: Think before patching
5. **Always end at output**: Complete the audio chain

## Practice Exercises

### Exercise 1: Basic Chain
Create: `notesequencer → synth → output`
- Practice connecting modules
- Verify sound output
- Try different synths

### Exercise 2: Add Effects
Extend to: `notesequencer → synth → delay → reverb → output`
- Insert effects into existing chain
- Adjust effect parameters
- Listen to the difference

### Exercise 3: Splitting
Create: One synth feeding two different effects
- Use cable splitting
- Compare parallel vs serial effects
- Mix wet/dry signals

### Exercise 4: Modulation
Add: `lfo → synth parameter`
- Connect modulator to slider
- Adjust modulation range
- Try different modulation targets

## Next Steps

Master module controls:
- **[UI Controls](ui-controls.md)** - Work with sliders and buttons
- **[Module Reference](../modules/README.md)** - Learn what each module does
- **[Advanced Topics](../advanced/README.md)** - Complex patching techniques

