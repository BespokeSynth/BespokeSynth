# User Guide

Complete guide to using BespokeSynth's interface and features.

## Overview

BespokeSynth is a modular synthesizer with a visual patching interface. This guide covers everything you need to know to work efficiently with the software.

## Guide Sections

### [Navigation](navigation.md)
Learn how to move around the canvas, zoom, and organize your workspace.

**Topics covered:**
- Canvas panning and zooming
- Bookmarks and locations
- Minimap usage
- Finding lost modules
- Workspace organization

### [Modules](modules.md)
Understand how to create, manage, and organize modules.

**Topics covered:**
- Creating modules (multiple methods)
- Deleting and duplicating modules
- Moving and arranging modules
- Module properties and settings
- Grouping and prefabs
- Module triangle menu

### [Patching](patching.md)
Master the art of connecting modules with patch cables.

**Topics covered:**
- Basic patching
- Advanced patching techniques
- Signal flow and routing
- Cable management
- Splitting and inserting
- Quick patching shortcuts

### [UI Controls](ui-controls.md)
Learn to work with sliders, buttons, and other interface elements.

**Topics covered:**
- Slider manipulation
- Fine-tuning controls
- Typing values
- Modulation and LFOs
- Range adjustment
- Keyboard navigation
- Control expressions

### [Saving & Loading](saving-loading.md)
Manage your projects and audio recordings.

**Topics covered:**
- Save states (.bsk files)
- Loading projects
- Auto-save and backups
- Audio recording
- Exporting audio
- Project organization

## Quick Reference

### Essential Shortcuts

| Action | Shortcut |
|--------|----------|
| Save | `Ctrl/Cmd + S` |
| Load | `Ctrl/Cmd + L` |
| Pan canvas | `Spacebar + Drag` or `Right-click Drag` |
| Zoom | `Spacebar + Scroll` or `Scroll` |
| Delete module | `Backspace` |
| Duplicate module | `Alt/Option + Drag title bar` |
| Reset view | `~` then type `home` |
| Pause audio | `Shift + P` |
| Console | `~` |

### Signal Types

BespokeSynth uses four main signal types:

| Type | Color | Description | Example Modules |
|------|-------|-------------|-----------------|
| **Notes** | 🟠 Orange | MIDI-style note messages | notesequencer, midicontroller |
| **Audio** | 🔵 Cyan | Audio waveforms | karplusstrong, effectchain |
| **Pulses** | 🟡 Yellow | Trigger/gate signals | pulser, notecounter |
| **Modulators** | 🟢 Green | Control voltage (CV) | lfo, envelope |

### Typical Signal Flow

```
┌─────────────┐     ┌──────────────┐     ┌───────┐     ┌────────────┐     ┌────────┐
│ Instrument  │────▶│ Note Effects │────▶│ Synth │────▶│   Effects  │────▶│ Output │
│ (notes)     │     │ (notes)      │     │(audio)│     │  (audio)   │     │        │
└─────────────┘     └──────────────┘     └───────┘     └────────────┘     └────────┘
```

**Example:**
```
notesequencer → arpeggiator → fmsynth → delay → reverb → output
```

### Module Categories

| Category | Purpose | Examples |
|----------|---------|----------|
| **Instruments** | Generate note sequences | notesequencer, drumsequencer, midicontroller |
| **Note Effects** | Process/transform notes | arpeggiator, chorder, notedelayer |
| **Synths** | Generate audio from notes | karplusstrong, fmsynth, sampler |
| **Audio Effects** | Process audio signals | effectchain, delay, reverb |
| **Modulators** | Create control signals | lfo, envelope, expression |
| **Pulse** | Timing and triggers | pulser, pulsegate, pulsetrain |
| **Other** | Utilities and special | transport, scale, script |

## Workflow Tips

### Starting a New Patch

1. **Plan your signal flow**: Think about what you want to create
2. **Start with output**: The output module is your destination
3. **Work backwards**: Add synth → instrument → sequencer
4. **Add effects last**: Get the basic sound first, then enhance
5. **Save early, save often**: Use `Ctrl/Cmd + S` frequently

### Organizing Complex Patches

1. **Use spatial organization**: Group related modules together
2. **Create prefabs**: Save reusable module groups
3. **Use bookmarks**: Save locations for different sections
4. **Label with comments**: Add comment modules for notes
5. **Color code**: Use different areas of canvas for different purposes

### Performance Optimization

1. **Delete unused modules**: They still consume CPU
2. **Minimize polyphony**: Reduce voice count on synths
3. **Use simpler effects**: Reverb and granular effects are CPU-heavy
4. **Increase buffer size**: If you hear crackling
5. **Freeze tracks**: Render complex chains to audio

### Creative Techniques

1. **Experiment with modulation**: Right-click any slider to add LFO
2. **Layer sounds**: Use multiple synths for richness
3. **Parallel processing**: Split signals for parallel effects
4. **Feedback loops**: Create interesting textures (use with caution!)
5. **Random elements**: Use random modules for generative music

## Common Workflows

### Live Performance Setup

```
midicontroller → synth → effects → output
     ↓
  snapshots (for preset changes)
```

### Generative Music

```
pulser → notecounter → notesequencer → synth → effects → output
  ↓
 lfo (modulating various parameters)
```

### Drum Programming

```
drumsequencer → drumplayer → effects → output
     ↓
  grid controller (for live pattern editing)
```

### Sample Manipulation

```
sampleplayer → granulator → effects → output
     ↓
  script (for dynamic sample control)
```

## Learning Resources

### Video Tutorials
- [Official Overview Video](https://www.youtube.com/watch?v=SYBc8X2IxqM) - Comprehensive introduction
- Search YouTube for "BespokeSynth tutorial" for community tutorials

### Community Resources
- [Discord Community](https://discord.gg/YdTMkvvpZZ) - Ask questions and share patches
- [Community Wiki](https://github.com/BespokeSynth/BespokeSynthDocs/wiki) - User-contributed documentation
- [GitHub Discussions](https://github.com/BespokeSynth/BespokeSynth/discussions) - Feature requests and ideas

### Example Patches
- Check the `savestate` folder in your BespokeSynth data directory
- Download patches from the Discord community
- Explore the example patches included with BespokeSynth

## Best Practices

### Do's ✅
- Save frequently with meaningful names
- Use the "write audio" button to capture performances
- Experiment and explore - BespokeSynth is designed for it
- Join the Discord community for help and inspiration
- Start simple and add complexity gradually
- Use prefabs for reusable module groups

### Don'ts ❌
- Don't create feedback loops without understanding them (can be loud!)
- Don't forget to set up your audio device before starting
- Don't be afraid to delete and start over
- Don't skip the official video tutorial
- Don't ignore CPU usage - optimize when needed
- Don't forget to back up important patches

## Keyboard Shortcuts Reference

### File Operations
- `Ctrl/Cmd + S` - Save state
- `Ctrl/Cmd + L` - Load state

### Canvas Navigation
- `Spacebar + Drag` - Pan canvas
- `Spacebar + Scroll` - Zoom
- `~` - Open console
- `Shift + 1-9` - Store bookmark
- `1-9` - Recall bookmark

### Module Operations
- `Backspace` - Delete selected modules
- `Alt/Option + Drag` - Duplicate module
- `Tab` - Cycle through module controls
- `Ctrl/Cmd + Arrows` - Navigate controls

### Playback
- `Shift + P` - Pause/unpause audio

### Display
- `F2` - Toggle ADSR slider display
- `F3` - Pin/unpin module

### Slider Controls (when hovering)
- `Click + Drag` - Adjust value
- `Shift + Drag` - Fine-tune
- `Type number + Enter` - Set exact value
- `Up/Down` - Increment by 1
- `Shift + Up/Down` - Increment by 0.01
- `[` - Halve value
- `]` - Double value
- `\` - Reset to default
- `Right-click` - Add LFO modulation

## Next Steps

Ready to dive deeper? Continue with:

1. **[Navigation](navigation.md)** - Master canvas movement
2. **[Modules](modules.md)** - Learn module management
3. **[Patching](patching.md)** - Connect modules like a pro
4. **[UI Controls](ui-controls.md)** - Control everything efficiently
5. **[Saving & Loading](saving-loading.md)** - Manage your projects

Or jump to:
- **[Module Reference](../modules/README.md)** - Explore all available modules
- **[Scripting Guide](../scripting/README.md)** - Add Python scripting
- **[Advanced Topics](../advanced/README.md)** - MIDI, OSC, and more

