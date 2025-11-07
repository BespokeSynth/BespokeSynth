# Getting Started with BespokeSynth

Welcome to BespokeSynth! This guide will help you install BespokeSynth and create your first musical patch.

## Installation

### Windows
1. Download the latest Windows installer from [bespokesynth.com](https://www.bespokesynth.com)
2. Run the installer and follow the prompts
3. Launch BespokeSynth from the Start Menu

### macOS
1. Download the latest macOS .dmg from [bespokesynth.com](https://www.bespokesynth.com)
2. Open the .dmg file
3. Drag BespokeSynth to your Applications folder
4. Launch BespokeSynth from Applications
5. If you see a security warning, go to System Preferences → Security & Privacy and click "Open Anyway"

### Linux
1. Download the latest Linux package from [bespokesynth.com](https://www.bespokesynth.com)
2. Install using your package manager or extract the archive
3. Run BespokeSynth from your applications menu or terminal

### Building from Source
See the [Building Guide](development/building.md) for instructions on building from source.

## First Launch

When you first launch BespokeSynth, you'll see:
- A blank canvas with a grid background
- A menu bar at the top with module categories
- An **output** module (your audio output)
- A **transport** module (controls tempo and playback)

## Audio Setup

Before making sound, configure your audio settings:

1. Click the **Settings** button (gear icon) in the top-right
2. Select your audio device under **Audio Output**
3. Choose an appropriate **Sample Rate** (44100 or 48000 Hz recommended)
4. Set **Buffer Size** (256 or 512 samples is a good starting point)
5. Click **OK** to apply settings

## Your First Patch: Simple Synth

Let's create a simple synthesizer that you can play with your computer keyboard.

### Step 1: Create a Keyboard
1. Right-click on the canvas
2. Type "keyboard" in the search box
3. Click and drag **keyboarddisplay** onto the canvas
4. Release to place it

### Step 2: Create a Synth
1. Right-click on the canvas again
2. Type "karplusstrong"
3. Drag **karplusstrong** onto the canvas
4. Place it to the right of the keyboard

### Step 3: Connect to Output
1. Find the **output** module (should already be on the canvas)
2. Click the circle at the bottom of **karplusstrong**
3. Click on the **output** module to connect them

### Step 4: Patch the Keyboard to the Synth
1. Click the circle at the bottom of **keyboarddisplay**
2. Click on the **karplusstrong** module to connect them

### Step 5: Play!
- Click on the keyboard display to play notes
- Or use your computer keyboard (ASDF row plays notes)

**Congratulations!** You've created your first BespokeSynth patch! 🎉

## Your Second Patch: Drum Beat

Let's create a simple drum pattern.

### Step 1: Create a Drum Sequencer
1. Right-click on the canvas
2. Type "drumsequencer"
3. Drag **drumsequencer** onto the canvas

### Step 2: Create a Drum Synth
1. Right-click on the canvas
2. Type "drumplayer"
3. Drag **drumplayer** onto the canvas
4. Place it to the right of the sequencer

### Step 3: Connect the Modules
1. Click the circle at the bottom of **drumsequencer**
2. Click on **drumplayer** to connect them
3. Click the circle at the bottom of **drumplayer**
4. Click on **output** to connect to audio output

### Step 4: Program a Beat
1. Click on the grid in the **drumsequencer** to add steps
2. Try creating a simple pattern:
   - Add kicks on steps 1, 5, 9, 13 (first row)
   - Add snares on steps 5 and 13 (second row)
   - Add hi-hats on every other step (third row)

### Step 5: Play
- The pattern should start playing automatically
- Adjust the tempo using the **transport** module

## Understanding the Interface

### Canvas Navigation
- **Pan**: Hold spacebar + drag mouse, or right-click drag
- **Zoom**: Hold spacebar + scroll, or scroll in empty space
- **Reset view**: Press `~` to open console, type "home", press Enter

### Module Management
- **Create**: Right-click and search, or drag from top menu
- **Delete**: Select module and press Backspace, or use triangle menu → Delete
- **Duplicate**: Hold Alt/Option and drag module's title bar
- **Move**: Click and drag the title bar

### Patching
- **Connect**: Click source circle → click destination module
- **Disconnect**: Drag cable and press Backspace
- **Split**: Hold Shift while grabbing a cable
- **Insert**: Hold Shift while releasing cable on target

### UI Controls
- **Adjust slider**: Click and drag
- **Fine-tune**: Hold Shift while dragging
- **Type value**: Hover and type a number, press Enter
- **Reset**: Hover and press `\`
- **Modulate**: Right-click slider to add LFO

## Saving Your Work

### Save State
1. Click the **save state** button in the top-right
2. Choose a location and filename
3. Click Save

Your entire patch layout is saved as a `.bsk` file.

### Load State
1. Click the **load state** button
2. Select a `.bsk` file
3. Click Open

### Write Audio
The **write audio** button saves the last 30 minutes of audio output to the recordings folder - perfect for capturing happy accidents!

## Key Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl/Cmd + S` | Save state |
| `Ctrl/Cmd + L` | Load state |
| `Shift + P` | Pause audio processing |
| `F2` | Toggle ADSR slider display |
| `F3` | Pin/unpin selected module |
| `~` | Open console |
| `Tab` | Cycle through module controls |
| `Backspace` | Delete selected modules |

## Signal Flow Basics

BespokeSynth uses different types of signals:

### Note Signals (Orange 🟠)
- Carry MIDI-style note information (pitch, velocity, duration)
- Flow from instruments through note effects to synths
- Example: `notesequencer → arpeggiator → fmsynth`

### Audio Signals (Cyan 🔵)
- Carry audio waveforms
- Flow from synths through audio effects to output
- Example: `karplusstrong → delay → reverb → output`

### Pulse Signals (Yellow 🟡)
- Carry trigger/gate signals
- Used for timing and synchronization
- Example: `pulser → notesequencer`

### Modulation Signals (Green 🟢)
- Carry control voltage (CV) for parameter modulation
- Can be connected to any slider
- Example: `lfo → filter cutoff`

## Common Module Categories

### Instruments
Generate note sequences:
- **notesequencer** - Step sequencer for melodies
- **drumsequencer** - Step sequencer for drums
- **midicontroller** - Use external MIDI devices
- **keyboarddisplay** - On-screen keyboard

### Synths
Generate audio from notes:
- **karplusstrong** - Plucked string synthesis
- **fmsynth** - FM synthesis
- **sampler** - Sample playback
- **drumplayer** - Drum sample player

### Effects
Process audio or notes:
- **effectchain** - Chain multiple effects
- **delay** - Echo effect
- **reverb** - Reverb effect
- **arpeggiator** - Note arpeggiator

### Modulators
Create control signals:
- **lfo** - Low-frequency oscillator
- **envelope** - Envelope generator
- **expression** - Math expressions for modulation

## Next Steps

Now that you've created your first patches, explore:

1. **[User Guide](user-guide/README.md)** - Learn all the interface features
2. **[Module Reference](modules/README.md)** - Discover all available modules
3. **[Patching Guide](user-guide/patching.md)** - Master advanced patching techniques
4. **[Scripting](scripting/README.md)** - Add Python scripting to your patches

## Getting Help

- **Discord**: Join the [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ)
- **Video Tutorial**: Watch the [overview video](https://www.youtube.com/watch?v=SYBc8X2IxqM)
- **Community Docs**: Check the [community wiki](https://github.com/BespokeSynth/BespokeSynthDocs/wiki)

## Tips for Success

1. **Start Simple**: Begin with basic patches and gradually add complexity
2. **Experiment**: BespokeSynth is designed for exploration - try things!
3. **Save Often**: Use `Ctrl/Cmd + S` frequently
4. **Use Write Audio**: Capture your jams with the write audio button
5. **Watch the Video**: The official tutorial video covers many hidden features
6. **Join the Community**: The Discord is friendly and helpful

Happy patching! 🎵

