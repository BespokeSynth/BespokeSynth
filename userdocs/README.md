# BespokeSynth Documentation

Welcome to the comprehensive documentation for BespokeSynth - a software modular synthesizer for creating music through live-patchable modules.

## 📖 Documentation Sections

### Getting Started
- **[Getting Started Guide](getting-started.md)** - Your first steps with BespokeSynth

### User Guide
- **[User Guide Overview](user-guide/README.md)** - Complete guide to using BespokeSynth
  - [Navigation](user-guide/navigation.md) - Moving around the canvas
  - [Modules](user-guide/modules.md) - Creating and managing modules
  - [Patching](user-guide/patching.md) - Connecting modules with cables
  - [UI Controls](user-guide/ui-controls.md) - Working with sliders, buttons, and controls
  - [Saving & Loading](user-guide/saving-loading.md) - Managing your projects

### Module Reference
- **[Module Reference](modules/README.md)** - Complete reference for all modules
  - [Instruments](modules/instruments.md) - Sequencers, keyboards, note generators
  - [Note Effects](modules/note-effects.md) - Note processing and transformation
  - [Synths](modules/synths.md) - Sound generators and synthesizers
  - [Audio Effects](modules/audio-effects.md) - Audio processing modules
  - [Modulators](modules/modulators.md) - CV and modulation sources
  - [Pulse](modules/pulse.md) - Pulse and trigger modules
  - [Other](modules/other.md) - Utility and special modules

### Python Scripting
- **[Scripting Guide](scripting/README.md)** - Python scripting in BespokeSynth
  - [Getting Started with Scripting](scripting/getting-started.md) - Your first script
  - [API Reference](scripting/api-reference.md) - Complete Python API documentation
  - [Examples](scripting/examples.md) - Example scripts and patterns

### Advanced Topics
- **[Advanced Usage](advanced/README.md)** - Advanced features and techniques
  - [MIDI Mapping](advanced/midi-mapping.md) - MIDI controller setup
  - [OSC Control](advanced/osc-control.md) - OSC protocol usage
  - [VST Plugins](advanced/vst-plugins.md) - Hosting VST/VST3/LV2 plugins
  - [Ableton Link](advanced/ableton-link.md) - Sync with Ableton Link
  - [Performance Tips](advanced/performance-tips.md) - Optimization techniques

### Development
- **[Development Guide](development/README.md)** - Contributing to BespokeSynth
  - [Building from Source](development/building.md) - Build instructions
  - [Architecture](development/architecture.md) - Code architecture overview
  - [Module Development](development/module-development.md) - Creating new modules
  - [Contributing](development/contributing.md) - Contribution guidelines
  - [Code Style](development/code-style.md) - Coding standards

### Troubleshooting
- **[Troubleshooting](troubleshooting.md)** - Common issues and solutions

## 🎵 What is BespokeSynth?

BespokeSynth is a software modular synthesizer that allows you to create music by patching together modules in a visual environment. Think of it as a virtual modular synthesizer where you can:

- **Build custom signal chains** by connecting modules with virtual patch cables
- **Live-code with Python** to create dynamic, generative music
- **Host VST plugins** alongside native modules
- **Map MIDI controllers** for hands-on control
- **Sync with other software** using Ableton Link
- **Create and save layouts** for different musical workflows

## 🚀 Quick Links

- [Official Website](https://www.bespokesynth.com)
- [GitHub Repository](https://github.com/BespokeSynth/BespokeSynth)
- [Discord Community](https://discord.gg/YdTMkvvpZZ)
- [Video Tutorial](https://www.youtube.com/watch?v=SYBc8X2IxqM)

## 📦 Installation

Download the latest release for your platform:
- **Windows**: [Download](https://www.bespokesynth.com)
- **macOS**: [Download](https://www.bespokesynth.com)
- **Linux**: [Download](https://www.bespokesynth.com)

Or build from source following the [Building Guide](development/building.md).

## 🎓 Learning Path

**New to BespokeSynth?** Follow this recommended learning path:

1. **[Getting Started](getting-started.md)** - Install and create your first patch
2. **[Navigation](user-guide/navigation.md)** - Learn to move around the canvas
3. **[Modules](user-guide/modules.md)** - Understand how to create modules
4. **[Patching](user-guide/patching.md)** - Connect modules together
5. **[Module Reference](modules/README.md)** - Explore available modules
6. **[Scripting](scripting/README.md)** - Add Python scripting (optional)

## 💡 Key Concepts

### Modules
Modules are the building blocks of BespokeSynth. Each module performs a specific function:
- **Instruments** generate notes
- **Synths** generate audio
- **Effects** process audio or notes
- **Modulators** create control signals

### Patching
Modules are connected using virtual patch cables. Signal flow is typically:
```
Instrument → Note Effects → Synth → Audio Effects → Output
```

### Signal Types
BespokeSynth uses different signal types:
- 🟠 **Notes** - MIDI-style note messages
- 🔵 **Audio** - Audio signals
- 🟡 **Pulses** - Trigger/gate signals
- 🟣 **Modulators** - Control voltage (CV) signals

## 🤝 Community & Support

- **Discord**: Join the [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ) for help and discussion
- **Issues**: Report bugs on [GitHub Issues](https://github.com/BespokeSynth/BespokeSynth/issues)
- **Discussions**: Share ideas on [GitHub Discussions](https://github.com/BespokeSynth/BespokeSynth/discussions)

## 📄 License

BespokeSynth is open source software licensed under the [GNU GPL v3](https://github.com/BespokeSynth/BespokeSynth/blob/main/LICENSE).

## 🙏 Credits

BespokeSynth is created and maintained by Ryan Challinor with contributions from the community.

---

**Ready to get started?** Head to the [Getting Started Guide](getting-started.md)!

