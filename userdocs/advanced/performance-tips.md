# Performance Tips

Complete guide to optimizing BespokeSynth for best performance and stability.

## Overview

This guide covers techniques to:
- Reduce CPU usage
- Minimize latency
- Prevent audio glitches
- Optimize for live performance
- Troubleshoot performance issues

## CPU Optimization

### Monitor CPU Usage

**Check CPU meter:**
- Located in top-right corner of BespokeSynth
- Shows current CPU usage percentage
- Red = danger zone (>80%)
- Yellow = caution (60-80%)
- Green = safe (<60%)

**Identify heavy modules:**
- Modules with high CPU show in red
- Hover over module to see CPU usage
- Focus optimization on heaviest modules

### Reduce Polyphony

**Synth modules:**
- Lower **voices** parameter
- Reduce from 16 to 8 or 4
- Use monophonic when possible
- Disable unused voices

**Example:**
```
synth: voices 16 → 8  (50% CPU reduction)
```

**VST plugins:**
- Reduce polyphony in plugin settings
- Lower voice count
- Use voice limiting

### Optimize Effects

**Use efficient effects:**
- **Lighter:** eq, compressor, gain
- **Heavier:** granulator, vocoder, convolution reverb

**Disable unused effects:**
- Delete modules you're not using
- Bypass effects when not needed
- Use effectchain efficiently

**Reduce effect quality:**
- Lower FFT size in spectral effects
- Reduce oversampling
- Use simpler algorithms

### Simplify Patches

**Module count:**
- Fewer modules = better performance
- Combine functionality where possible
- Delete unused modules

**Cable complexity:**
- Simpler routing = better performance
- Avoid excessive feedback loops
- Minimize cable crossings (visual only)

**Script optimization:**
- Avoid heavy computation in callbacks
- Cache calculated values
- Use efficient algorithms

### Freeze/Bounce

**Render to audio:**
1. Record module output to **looper** or **sampleplayer**
2. Delete original modules
3. Play back recorded audio
4. Massive CPU savings

**When to freeze:**
- Complex synth patches
- Heavy VST plugins
- Finished parts
- CPU-intensive effects

## Audio Settings

### Buffer Size

**What it does:**
- Larger buffer = more stable, higher latency
- Smaller buffer = lower latency, less stable

**Recommended settings:**

**Live performance:**
```
Buffer size: 256-512 samples
Latency: ~6-12 ms
Stability: Good
```

**Studio/recording:**
```
Buffer size: 512-1024 samples
Latency: ~12-24 ms
Stability: Excellent
```

**Low-latency monitoring:**
```
Buffer size: 128-256 samples
Latency: ~3-6 ms
Stability: Requires powerful CPU
```

**Adjust buffer size:**
- Preferences → Audio Settings
- Increase if glitches occur
- Decrease for lower latency
- Find balance for your system

### Sample Rate

**Common rates:**
- **44.1 kHz** - CD quality, efficient
- **48 kHz** - Video standard, efficient
- **88.2/96 kHz** - High quality, 2x CPU
- **176.4/192 kHz** - Ultra quality, 4x CPU

**Recommendation:**
- Use 44.1 or 48 kHz
- Higher rates rarely audible
- Significant CPU cost
- Match your audio interface

### Audio Driver

**Windows:**
- **ASIO** - Best performance, lowest latency
- **WASAPI** - Good, built-in
- **DirectSound** - Avoid (high latency)

**macOS:**
- **CoreAudio** - Excellent, built-in
- No configuration needed

**Linux:**
- **JACK** - Best performance, flexible
- **ALSA** - Good, built-in
- **PulseAudio** - Avoid for music

## Live Performance Optimization

### Preparation

**Before the show:**
1. **Test thoroughly** - Run patch for extended time
2. **Monitor CPU** - Ensure headroom (stay <70%)
3. **Simplify** - Remove unnecessary modules
4. **Freeze tracks** - Bounce CPU-heavy parts
5. **Save multiple versions** - Have backups

**Optimize patch:**
- Delete unused modules
- Reduce polyphony
- Disable heavy effects
- Test all scenarios

### During Performance

**CPU management:**
- Monitor CPU meter constantly
- Have backup plan if CPU spikes
- Know which modules to disable
- Keep CPU below 70%

**Stability:**
- Avoid creating/deleting modules
- Don't load new patches mid-performance
- Minimize file I/O
- Use snapshots for changes

**Backup plan:**
- Have simplified version ready
- Know how to quickly reduce CPU
- Practice emergency procedures
- Have backup computer/patch

### Snapshots

**Use snapshots for:**
- Preset changes
- Effect on/off
- Parameter sets
- Scene changes

**Benefits:**
- Instant recall
- No CPU spike
- Reliable
- Rehearsable

**Setup:**
1. Create **snapshots** module
2. Store different states
3. Map to MIDI controller
4. Practice transitions

## Module-Specific Tips

### Synths

**Optimize:**
- Reduce voices (16 → 8 → 4)
- Lower filter quality
- Reduce oscillators
- Disable unused features

**Efficient synths:**
- **basicsynth** - Very light
- **karplus** - Light
- **fm** - Moderate
- **granulator** - Heavy

### Effects

**Light effects:**
- **eq** - Very light
- **compressor** - Light
- **gain** - Minimal
- **filter** - Light

**Heavy effects:**
- **granulator** - Very heavy
- **vocoder** - Heavy
- **convolution reverb** - Heavy
- **spectral effects** - Heavy

**Optimization:**
- Use lighter alternatives
- Reduce quality settings
- Bypass when not needed
- Freeze to audio

### Sequencers

**Optimize:**
- Reduce step count
- Lower update rate
- Simplify patterns
- Use fewer lanes

**Efficient sequencing:**
- **notesequencer** - Light
- **drumsequencer** - Light
- **script** - Depends on code
- **grid** - Moderate

### VST Plugins

**Reduce CPU:**
- Lower plugin quality settings
- Reduce oversampling
- Disable unused features
- Use native modules when possible

**Freeze plugins:**
- Bounce to audio
- Delete plugin module
- Massive CPU savings

## Scripting Optimization

### Efficient Code

**Avoid in callbacks:**
```python
# BAD - Heavy computation every pulse
def on_pulse():
    for i in range(1000):
        result = complex_calculation()
```

```python
# GOOD - Cache results
results = [complex_calculation() for i in range(1000)]

def on_pulse():
    result = results[step]
```

**Use efficient algorithms:**
```python
# BAD - O(n²) complexity
for i in range(len(notes)):
    for j in range(len(notes)):
        compare(notes[i], notes[j])

# GOOD - O(n) complexity
for note in notes:
    process(note)
```

### Reduce Callback Frequency

**Limit updates:**
```python
# BAD - Update every pulse (could be 16th notes)
def on_pulse():
    update_display()

# GOOD - Update every 4 pulses
count = 0
def on_pulse():
    global count
    count += 1
    if count % 4 == 0:
        update_display()
```

### Cache Calculations

**Reuse results:**
```python
# BAD - Recalculate every time
def on_pulse():
    scale = bespoke.get_scale_range(4, 8)
    pitch = random.choice(scale)

# GOOD - Calculate once
scale = bespoke.get_scale_range(4, 8)

def on_pulse():
    pitch = random.choice(scale)
```

## System Optimization

### Operating System

**Windows:**
- Disable Windows Defender real-time scanning (for audio folder)
- Disable Windows Update during performance
- Close unnecessary background apps
- Use High Performance power plan

**macOS:**
- Disable Spotlight indexing (for audio folder)
- Close unnecessary apps
- Disable automatic updates
- Use Energy Saver settings appropriately

**Linux:**
- Use real-time kernel (if available)
- Configure JACK for low latency
- Disable unnecessary services
- Use performance CPU governor

### Hardware

**CPU:**
- Faster CPU = better performance
- More cores = more polyphony
- Modern CPU recommended

**RAM:**
- 8GB minimum
- 16GB recommended
- 32GB+ for large sample libraries

**Storage:**
- SSD recommended
- Faster loading
- Better streaming
- Reduced latency

**Audio Interface:**
- Quality interface = better drivers
- Lower latency
- More stable
- Better sound quality

## Troubleshooting

### Audio Glitches/Dropouts

**Causes:**
- CPU overload
- Buffer size too small
- Disk I/O issues
- Driver problems

**Solutions:**
1. Increase buffer size
2. Reduce CPU usage
3. Close other apps
4. Update audio drivers
5. Use SSD for samples

### High CPU Usage

**Identify culprit:**
1. Check CPU meter
2. Hover over modules
3. Find heaviest modules
4. Optimize or remove

**Solutions:**
- Reduce polyphony
- Freeze tracks
- Simplify effects
- Delete unused modules

### Latency Issues

**Causes:**
- Large buffer size
- Slow audio interface
- USB issues
- Driver problems

**Solutions:**
1. Decrease buffer size
2. Use ASIO/CoreAudio
3. Update drivers
4. Use USB 2.0/3.0 port (not hub)
5. Upgrade audio interface

### Crashes

**Causes:**
- VST plugin crash
- Out of memory
- Driver issues
- Software bug

**Solutions:**
1. Blacklist problematic plugins
2. Reduce memory usage
3. Update drivers
4. Report bug to developers
5. Use stable version

## Best Practices

### Patch Design

1. **Start simple** - Add complexity gradually
2. **Test frequently** - Monitor CPU as you build
3. **Optimize early** - Don't wait until performance
4. **Document** - Note CPU-heavy modules
5. **Save versions** - Keep optimized versions

### Workflow

1. **Build in stages** - Don't do everything at once
2. **Test on target system** - Performance varies by computer
3. **Have headroom** - Keep CPU below 70%
4. **Freeze when done** - Bounce finished parts
5. **Backup** - Save multiple versions

### Live Performance

1. **Rehearse** - Practice with actual patch
2. **Monitor** - Watch CPU constantly
3. **Simplify** - Remove non-essential elements
4. **Backup** - Have plan B ready
5. **Test** - Run for extended time before show

## Benchmarking

### Test Your System

**CPU test:**
1. Create multiple synth modules
2. Set high polyphony
3. Play dense chords
4. Monitor CPU usage
5. Find your limit

**Latency test:**
1. Set small buffer size
2. Play notes
3. Listen for glitches
4. Find minimum stable buffer

**Stress test:**
1. Build complex patch
2. Run for 30+ minutes
3. Monitor stability
4. Check for memory leaks

## Resources

### Tools

- **CPU Monitor** - Built into BespokeSynth
- **Task Manager** (Windows) - System monitoring
- **Activity Monitor** (macOS) - System monitoring
- **htop** (Linux) - System monitoring

### Community

- [Discord](https://discord.gg/YdTMkvvpZZ) - Performance tips
- [GitHub](https://github.com/BespokeSynth/BespokeSynth) - Report issues
- [Forum](https://github.com/BespokeSynth/BespokeSynth/discussions) - Discuss optimization

## Next Steps

- **[VST Plugins](vst-plugins.md)** - Optimize plugin usage
- **[MIDI Mapping](midi-mapping.md)** - Efficient control
- **[Ableton Link](ableton-link.md)** - Network sync optimization
