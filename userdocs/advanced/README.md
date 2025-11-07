# Advanced Topics

Deep dive into advanced BespokeSynth features and techniques.

## Overview

This section covers advanced topics for experienced users who want to get the most out of BespokeSynth.

## Topics

### [MIDI Mapping](midi-mapping.md)
Learn to map MIDI controllers to BespokeSynth parameters.

**Covered topics**:
- MIDI controller setup
- CC mapping
- MIDI learn
- Advanced mapping techniques
- MPE (MIDI Polyphonic Expression)
- Multi-controller setups

### [OSC Control](osc-control.md)
Control BespokeSynth with OSC (Open Sound Control).

**Covered topics**:
- OSC basics
- Setting up OSC communication
- OSC address patterns
- Bidirectional OSC
- TouchOSC integration
- Max/MSP integration

### [VST Plugins](vst-plugins.md)
Host VST plugins within BespokeSynth.

**Covered topics**:
- VST setup and configuration
- Loading VST instruments
- Loading VST effects
- VST automation
- Troubleshooting VST issues
- Performance considerations

### [Ableton Link](ableton-link.md)
Sync BespokeSynth with other software using Ableton Link.

**Covered topics**:
- Ableton Link basics
- Setting up Link
- Syncing with Ableton Live
- Syncing with other Link apps
- Multi-device sync
- Performance tips

### [Performance Tips](performance-tips.md)
Optimize BespokeSynth for best performance.

**Covered topics**:
- CPU optimization
- Audio buffer settings
- Module optimization
- Patch organization
- Live performance setup
- Troubleshooting performance issues

## Advanced Techniques

### Modular Patching Techniques

**Feedback Loops**:
Create controlled feedback for interesting textures.

```
synth → delay → output
         ↑  │
         └──┘ (feedback)
```

**Warning**: Start with low feedback amounts!

**Parallel Processing**:
Process audio through multiple paths.

```
                ┌→ delay ──┐
synth → send ──┤          ├→ output
                └→ reverb ─┘
```

**Sidechain**:
Use one signal to control another.

```
kick → compressor (sidechain) ← bass
  │
  └→ output
```

### Generative Music

**Probability-Based**:
```
pulser → pulsechance → randomnote → quantizer → synth
```

**Euclidean Rhythms**:
```
euclideansequencer → synth
```

**Feedback Systems**:
```
script → notesequencer → synth → audiotopulse → script
```

### Live Coding

**Using Script Module**:
- Write code during performance
- Modify algorithms in real-time
- Create evolving patterns
- Interactive music systems

**Tips**:
- Test code before performing
- Have backup patches
- Use snapshots for safety
- Practice extensively

### Multi-Timbral Setups

**Layer Multiple Synths**:
```
notesequencer → [synth1, synth2, synth3] → output
```

**Split by Pitch**:
```
notesequencer → noterangefilter → synth1 (bass)
              → noterangefilter → synth2 (mid)
              → noterangefilter → synth3 (high)
```

**Split by Velocity**:
```
notesequencer → notefilter (low vel) → synth1
              → notefilter (high vel) → synth2
```

## Integration

### DAW Integration

**As Plugin** (if available):
- Load BespokeSynth as VST in DAW
- Sync to DAW tempo
- Automate parameters
- Record audio output

**MIDI Routing**:
- Send MIDI from DAW to BespokeSynth
- Send MIDI from BespokeSynth to DAW
- Use virtual MIDI cables

**Audio Routing**:
- Route audio to DAW via virtual cables
- Use BespokeSynth as sound source
- Process DAW audio in BespokeSynth

### External Hardware

**MIDI Controllers**:
- Map hardware controls
- Use grid controllers
- Integrate drum pads
- Use keyboard controllers

**Audio Interfaces**:
- Multi-channel audio
- Low-latency monitoring
- High-quality conversion
- Multiple outputs

**Modular Synths**:
- CV/Gate via DC-coupled interface
- MIDI-to-CV conversion
- Audio processing
- Hybrid setups

## Advanced Workflows

### Live Performance

**Setup**:
1. Organize modules spatially
2. Map MIDI controllers
3. Create snapshots for presets
4. Use bookmarks for navigation
5. Test thoroughly

**During Performance**:
- Use snapshots for changes
- Modulate parameters live
- Trigger patterns manually
- Improvise with scripts

**Safety**:
- Save multiple versions
- Have backup patches
- Test all mappings
- Monitor CPU usage

### Studio Production

**Composition**:
- Use BespokeSynth for ideas
- Record audio output
- Export stems
- Process in DAW

**Sound Design**:
- Create unique sounds
- Record samples
- Build presets
- Share prefabs

**Mixing**:
- Use individual outputs
- Process in DAW
- Automate parameters
- Bounce to audio

### Collaborative Music

**Ableton Link**:
- Sync multiple instances
- Collaborate remotely
- Jam with others
- Share tempo

**OSC**:
- Control from other software
- Sync with visuals
- Network collaboration
- Remote control

## Optimization

### CPU Optimization

**Reduce Polyphony**:
- Lower voice count on synths
- Use monophonic when possible
- Delete unused voices

**Simplify Effects**:
- Use effectchain efficiently
- Disable unused effects
- Choose lighter effects
- Avoid granulator unless needed

**Patch Organization**:
- Delete unused modules
- Organize spatially
- Use prefabs
- Keep it clean

### Audio Settings

**Buffer Size**:
- Larger = more stable, higher latency
- Smaller = lower latency, less stable
- Find balance for your system

**Sample Rate**:
- 44.1kHz or 48kHz recommended
- Higher rates = more CPU
- Match your audio interface

**Audio Driver**:
- ASIO (Windows) for best performance
- CoreAudio (macOS) built-in
- JACK (Linux) for flexibility

## Troubleshooting

### Advanced Issues

**Feedback Loop Too Loud**:
- Disconnect immediately
- Use feedback module
- Start with low amounts
- Add attenuation

**MIDI Timing Issues**:
- Check MIDI clock source
- Adjust buffer size
- Use Ableton Link
- Check for jitter

**VST Plugin Crashes**:
- Update plugin
- Check compatibility
- Blacklist problematic plugins
- Use alternative

**Performance Degradation**:
- Monitor CPU usage
- Identify heavy modules
- Optimize patch
- Increase buffer size

## Best Practices

### Patch Design

1. **Plan before building**: Sketch complex patches
2. **Start simple**: Add complexity gradually
3. **Test incrementally**: Verify each addition
4. **Document**: Use comments
5. **Save versions**: Keep backups

### Performance

1. **Practice**: Test before performing
2. **Have backups**: Multiple versions
3. **Map controls**: MIDI for hands-on
4. **Monitor CPU**: Watch usage
5. **Keep it simple**: Complex = risky

### Collaboration

1. **Share prefabs**: Reusable modules
2. **Document patches**: Explain usage
3. **Use standards**: Common practices
4. **Test thoroughly**: Verify compatibility
5. **Communicate**: Explain your work

## Resources

### Community

- [Discord](https://discord.gg/YdTMkvvpZZ) - Community chat
- [GitHub](https://github.com/BespokeSynth/BespokeSynth) - Source code
- [Forum](https://github.com/BespokeSynth/BespokeSynth/discussions) - Discussions

### Learning

- [Video Tutorials](https://www.youtube.com/results?search_query=bespoke+synth+tutorial)
- [Official Docs](https://bespokesynth.com/docs/)
- [Example Patches](https://github.com/BespokeSynth/BespokeSynth/tree/main/patches)

### Tools

- [Virtual MIDI Cables](https://www.tobias-erichsen.de/software/loopmidi.html) (Windows)
- [IAC Driver](https://support.apple.com/guide/audio-midi-setup/transfer-midi-information-between-apps-ams1013/mac) (macOS)
- [TouchOSC](https://hexler.net/products/touchosc) - OSC controller
- [JACK Audio](https://jackaudio.org/) - Audio routing (Linux)

## Next Steps

Explore specific advanced topics:

1. **[MIDI Mapping](midi-mapping.md)** - Controller integration
2. **[OSC Control](osc-control.md)** - Network control
3. **[VST Plugins](vst-plugins.md)** - Plugin hosting
4. **[Ableton Link](ableton-link.md)** - Multi-app sync
5. **[Performance Tips](performance-tips.md)** - Optimization

Or dive into:
- **[Scripting](../scripting/README.md)** - Python programming
- **[Development](../development/README.md)** - Build and contribute

