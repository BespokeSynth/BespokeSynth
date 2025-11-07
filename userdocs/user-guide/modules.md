# Modules

Learn how to create, manage, and organize modules in BespokeSynth.

## What are Modules?

Modules are the building blocks of BespokeSynth. Each module performs a specific function:
- **Instruments** generate note sequences
- **Synths** generate audio from notes
- **Effects** process audio or notes
- **Modulators** create control signals
- **Utilities** provide special functions

## Creating Modules

There are multiple ways to create modules in BespokeSynth.

### Method 1: Right-Click Menu
1. Right-click on empty canvas space
2. Select module from categorized menu
3. Drag module onto canvas
4. Release to place

**Best for**: Browsing available modules

### Method 2: Right-Click + Search
1. Right-click on empty canvas space
2. Start typing module name (e.g., "note")
3. Matching modules appear in menu
4. Drag desired module onto canvas

**Best for**: When you know what you want

### Method 3: Top Menu Bar
1. Click category in top menu (Instruments, Synths, etc.)
2. Drag module from dropdown
3. Place on canvas

**Best for**: Exploring module categories

### Method 4: Hold First Letter
1. Hold down first letter of module name (e.g., "n" for notesequencer)
2. Menu appears with modules starting with that letter
3. Drag module onto canvas

**Best for**: Speed when you know module names

### Method 5: Drag Cable into Space
1. Grab a patch cable from existing module
2. Drag into empty space
3. Menu appears with compatible modules
4. Select and place module

**Best for**: Quick workflow, automatic connection

## Module Anatomy

Every module has common elements:

```
┌─────────────────────────────┐
│ ▼ Module Name            [≡]│ ← Title bar (drag to move)
├─────────────────────────────┤
│                             │
│   Controls and displays     │ ← Module-specific UI
│                             │
├─────────────────────────────┤
│           ●                 │ ← Patch cable output
└─────────────────────────────┘
```

### Title Bar
- **Module name**: Identifies the module
- **Triangle menu** (▼): Access module settings
- **Hamburger menu** ([≡]): Additional options
- **Drag area**: Click and drag to move module

### Module Body
- Contains controls (sliders, buttons, displays)
- Module-specific interface
- Can be resized on some modules

### Patch Cable Output
- Circle at bottom of module
- Click to create patch cable
- Drag to connect to other modules

## Moving Modules

### Basic Movement
1. Click and hold module title bar
2. Drag to new position
3. Release to place

### Precise Positioning
- Use grid background for alignment
- Zoom in for fine adjustments
- Organize by signal flow or function

### Moving Multiple Modules
1. Lasso select modules (click and drag in empty space)
2. Drag any selected module's title bar
3. All selected modules move together

## Deleting Modules

### Method 1: Backspace Key
1. Click module to select (or lasso select multiple)
2. Press `Backspace`
3. Module(s) deleted

**Best for**: Quick deletion

### Method 2: Triangle Menu
1. Click triangle (▼) in module title bar
2. Select "delete module"
3. Module deleted

**Best for**: Single module deletion

### Deleting Connections
- Deleting a module automatically removes its patch cables
- No need to manually disconnect first

## Duplicating Modules

Create a copy of an existing module with all its settings.

### How to Duplicate
1. Hold `Alt` (Windows/Linux) or `Option` (macOS)
2. Click and drag module's title bar
3. Release to place duplicate

### What Gets Copied
- ✅ All parameter values
- ✅ Module settings
- ✅ Internal state
- ❌ Patch cable connections (not copied)

### When to Duplicate
- Creating variations of a sound
- Building parallel processing chains
- Copying complex module setups
- Creating multiple instances

## Module Triangle Menu

Click the triangle (▼) in the title bar to access:

### Common Options
- **delete module**: Remove this module
- **save preset**: Save current settings
- **load preset**: Load saved settings
- **enabled**: Enable/disable module processing
- **minimize**: Collapse module to title bar only

### Module-Specific Options
Different modules have different options:
- **grid steps** (sequencers): Set sequence length
- **voices** (synths): Set polyphony
- **show/hide controls**: Toggle UI elements
- **reset**: Reset to default values

## Resizable Modules

Some modules can be resized for better visibility.

### How to Resize
1. Look for resize handle (usually bottom-right corner)
2. Click and drag to resize
3. Release when desired size reached

### Resizable Module Examples
- **notecanvas**: Resize to see more notes
- **script**: Resize code editor
- **waveformviewer**: Resize display area
- **spectrum**: Resize analyzer display

## Module States

Modules can be in different states:

### Enabled
- Module is active and processing
- Normal appearance
- Consumes CPU

### Disabled
- Module is bypassed
- Grayed out appearance
- Minimal CPU usage
- Toggle via triangle menu → "enabled"

### Minimized
- Module collapsed to title bar only
- Saves screen space
- Still processes normally
- Click title bar to expand

## Organizing Modules

### Spatial Organization

**By Signal Flow** (recommended):
```
[Source] → [Processing] → [Destination]
```

**By Function**:
```
┌─────────┐  ┌─────────┐
│ Drums   │  │ Melody  │
└─────────┘  └─────────┘
```

**By Layer**:
```
[Drums]
[Bass]
[Melody]
[Effects]
```

### Alignment Tips
1. Use grid background as guide
2. Align modules horizontally or vertically
3. Leave consistent spacing
4. Group related modules
5. Keep signal flow clear

## Prefabs

Save groups of modules as reusable prefabs.

### Creating a Prefab
1. Create a **prefab** module
2. Drag modules onto the prefab
3. Click "save" in prefab module
4. Name and save your prefab

### Loading a Prefab
1. Right-click canvas
2. Navigate to "prefabs" menu
3. Select your saved prefab
4. Place on canvas

### When to Use Prefabs
- Reusable effect chains
- Common instrument setups
- Template patches
- Performance rigs
- Sharing with others

## Module Selection

### Single Selection
- Click module to select
- Selected module highlighted
- Deselect by clicking empty space

### Multiple Selection (Lasso)
1. Click and drag in empty space
2. Rectangle appears
3. Release to select all modules inside
4. Selected modules highlighted

### Selection Operations
- **Move**: Drag any selected module
- **Delete**: Press Backspace
- **Deselect**: Click empty space

## Module Properties

Access module properties via triangle menu.

### Common Properties
- **enabled**: On/off toggle
- **name**: Custom module name
- **position**: X/Y coordinates
- **connections**: View patch cables

### Saving Module State
- Module settings saved with patch
- Presets can be saved separately
- Some modules have internal save/load

## Special Modules

### Output Module
- **Cannot be deleted**: Required for audio
- **Always needed**: Final destination for audio
- **Settings**: Volume, limiter, VU meter

### Transport Module
- **Controls tempo**: Global BPM
- **Time signature**: Beats per measure
- **Swing**: Timing feel
- **Cannot be deleted**: Required for timing

### Scale Module
- **Sets global scale**: All modules use this
- **Root note**: Key of your music
- **Scale type**: Major, minor, etc.
- **Affects**: Note quantization, sequencers

## Module Categories

### Instruments 🎹
Generate note sequences:
- notesequencer, drumsequencer
- midicontroller, keyboarddisplay
- notecreator, randomnote

### Note Effects 🎵
Process note messages:
- arpeggiator, chorder
- notedelayer, notegate
- velocityscaler, pitchbender

### Synths 🔊
Generate audio from notes:
- karplusstrong, fmsynth
- sampler, drumplayer
- oscillator, beats

### Audio Effects 🎚️
Process audio signals:
- effectchain, delay, reverb
- eq, compressor, distortion
- looper, granulator

### Modulators 📈
Create control signals:
- lfo, envelope
- expression, curve
- audioleveltocv, pitchtocv

### Pulse ⚡
Timing and triggers:
- pulser, pulsegate
- pulsetrain, pulsedelayer
- audiotopulse, notetopulse

### Other 🔧
Utilities and special:
- script, comment, label
- snapshots, prefab
- midioutput, oscoutput

## Tips and Best Practices

### Do's ✅
- Name modules descriptively (use triangle menu)
- Organize spatially by function
- Delete unused modules (saves CPU)
- Use prefabs for common setups
- Keep signal flow clear
- Leave space between modules

### Don'ts ❌
- Don't overcrowd the canvas
- Don't create modules you won't use
- Don't forget to save presets
- Don't ignore module organization
- Don't delete output or transport modules

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Delete module | `Backspace` |
| Duplicate module | `Alt/Option + Drag` |
| Cycle controls | `Tab` |
| Pin/unpin module | `F3` |

## Common Issues

### Module Won't Create
- **Check compatibility**: Some modules require specific builds
- **Check category**: Make sure you're looking in right place
- **Update BespokeSynth**: Newer modules need newer versions

### Module Disappeared
- **Pan around**: May be off-screen
- **Reset view**: Press `~`, type `home`
- **Check minimap**: If enabled, shows all modules

### Module Not Working
- **Check enabled state**: Triangle menu → enabled
- **Check connections**: Verify patch cables
- **Check settings**: Review module parameters

## Next Steps

Learn how to connect modules:
- **[Patching](patching.md)** - Connect modules with cables
- **[UI Controls](ui-controls.md)** - Work with module controls
- **[Module Reference](../modules/README.md)** - Explore all modules

