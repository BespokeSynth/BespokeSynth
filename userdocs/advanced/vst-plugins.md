# VST Plugins

Complete guide to hosting VST/VST3/LV2 plugins in BespokeSynth.

## Overview

BespokeSynth can host third-party plugins, allowing you to:
- Use your favorite VST instruments and effects
- Combine VST plugins with native BespokeSynth modules
- Process BespokeSynth audio through VST effects
- Trigger VST instruments from BespokeSynth sequencers
- Automate VST parameters with BespokeSynth modulators

## Supported Plugin Formats

### VST2
- **Windows:** .dll files
- **macOS:** .vst bundles
- **Linux:** .so files
- **Status:** Widely supported, legacy format

### VST3
- **Windows:** .vst3 files
- **macOS:** .vst3 bundles
- **Linux:** .vst3 files
- **Status:** Modern format, recommended

### LV2
- **Linux:** .lv2 bundles
- **Status:** Open-source format, Linux-focused

### Audio Unit (AU)
- **macOS only:** .component files
- **Status:** macOS native format

## Quick Start

### Loading a Plugin

1. **Create VST Plugin Module**
   ```
   Right-click → vstplugin
   ```

2. **Select Plugin**
   - Click the **plugin** dropdown
   - Browse available plugins
   - Select your desired plugin

3. **Open Editor**
   - Click **open editor** button
   - Plugin's native GUI appears
   - Adjust parameters as needed

4. **Connect Audio/MIDI**
   - Connect note sources to vstplugin input
   - Connect vstplugin output to audio destination
   - Play!

## Plugin Scanning

### Initial Scan

**First launch:**
- BespokeSynth scans default plugin folders
- This may take several minutes
- Progress shown in console

**Default scan locations:**

**Windows:**
```
C:\Program Files\VSTPlugins
C:\Program Files\Common Files\VST3
C:\Program Files\Steinberg\VSTPlugins
```

**macOS:**
```
/Library/Audio/Plug-Ins/VST
/Library/Audio/Plug-Ins/VST3
/Library/Audio/Plug-Ins/Components (AU)
~/Library/Audio/Plug-Ins/VST
~/Library/Audio/Plug-Ins/VST3
```

**Linux:**
```
~/.vst
~/.vst3
/usr/lib/vst
/usr/lib/vst3
/usr/lib/lv2
~/.lv2
```

### Manual Scan

**Rescan plugins:**
1. Menu → **Plugins** → **Plugin Manager**
2. Click **Options** → **Scan for new or updated plugins**
3. Wait for scan to complete

**Add custom folder:**
1. Open Plugin Manager
2. Click **Options** → **Edit the list of plugin folders**
3. Add your custom folder
4. Click **Scan**

### Blacklisting Plugins

**If a plugin crashes:**
1. Plugin is automatically blacklisted
2. Listed in `vst/deadmanspedal.txt`
3. Won't be loaded on next scan

**Manually blacklist:**
1. Open `data/vst/deadmanspedal.txt`
2. Add plugin name or ID
3. Save file
4. Restart BespokeSynth

## VST Plugin Module

### Basic Controls

**Plugin Selection:**
- **plugin** - Dropdown to select loaded plugin
- **open editor** - Open plugin's native GUI
- **panic** - Send all-notes-off (emergency stop)

**Audio:**
- **volume** - Output volume
- **enabled** - Bypass plugin

**MIDI:**
- **channel** - MIDI channel (1-16)
- **usevoiceaschannel** - Use voice index as MIDI channel (MPE)
- **pitchbendrange** - Pitch bend range in semitones
- **modwheelcc** - CC number for mod wheel (1 or 74)

### Parameter Control

**Expose parameters:**
1. Click **load parameter** button
2. Parameters appear in dropdown
3. Click parameter name to show slider
4. Slider appears on module

**Show parameter slider:**
- Click parameter name in dropdown
- Slider appears below
- Can be automated/modulated

**Hide parameter slider:**
- Click **x** next to slider
- Slider hidden but mapping remains

**Load all parameters:**
- Hold **Shift** + click **load parameter**
- All parameters loaded at once
- Useful for complex plugins

### Multiple Outputs

**Some plugins have multiple stereo outputs:**

**Add output:**
1. Click **add output** button
2. New output cable appears
3. Connect to separate destination

**Remove output:**
1. Click **remove output** button
2. Last output removed

**Use case:**
- Multi-timbral instruments
- Separate drum outputs
- Parallel processing

## Using VST Instruments

### Basic Setup

```
notesequencer → vstplugin → output
```

**Steps:**
1. Create vstplugin module
2. Load instrument plugin
3. Create note source (notesequencer, keyboard, etc.)
4. Connect note source to vstplugin
5. Connect vstplugin audio to output
6. Play notes!

### Polyphony

**Most VST instruments handle polyphony internally:**
- No need for BespokeSynth voice management
- Plugin manages its own voices
- Polyphony set in plugin

**Voice-per-channel (MPE):**
- Enable **usevoiceaschannel**
- Each voice gets separate MIDI channel
- Allows per-note pitch bend/modulation

### Automation

**Automate VST parameters:**

**Method 1: Modulators**
```
lfo → vstplugin parameter
```

**Method 2: Scripts**
```python
vst = vstplugin.get("vstplugin")
vst.set("parameter_name", 0.5)
```

**Method 3: MIDI CC**
```
midicontroller → vstplugin
```
Map MIDI CC to plugin parameters.

## Using VST Effects

### Basic Setup

```
synth → vstplugin (effect) → output
```

**Steps:**
1. Create vstplugin module
2. Load effect plugin
3. Connect audio source to vstplugin
4. Connect vstplugin to output
5. Process audio!

### Sidechain

**Some effects support sidechain:**

**Setup:**
1. Load effect with sidechain input
2. Connect main audio to vstplugin
3. Connect sidechain source to vstplugin (if supported)
4. Configure sidechain in plugin

**Note:** Sidechain support depends on plugin.

### Parallel Processing

**Process audio in parallel:**

```
synth → send → vstplugin (reverb) → output
         ↓
       output (dry)
```

**Use send module for wet/dry mix.**

## Advanced Features

### Preset Management

**Save preset:**
1. Open plugin editor
2. Configure parameters
3. Save preset in plugin
4. Preset saved with plugin

**Load preset:**
1. Open plugin editor
2. Load preset from plugin's preset browser
3. Parameters update

**BespokeSynth preset handling:**
- **preset_file_sets_params** (default: true)
  - Loading preset updates parameter sliders
- Set to false to prevent slider updates

### Program Change

**Switch presets via MIDI:**
1. Send MIDI Program Change to vstplugin
2. Plugin switches to that program
3. Useful for live performance

**From script:**
```python
controller = midicontroller.get("midicontroller")
controller.send_program_change(5)  # Load program 5
```

### State Saving

**Plugin state saved with patch:**
- All parameter values
- Current preset
- Plugin-specific state
- Restored on patch load

**Ensure compatibility:**
- Use same plugin version
- Plugin must be installed
- Plugin ID must match

### Latency Compensation

**Some plugins report latency:**
- BespokeSynth automatically compensates
- Keeps audio in sync
- Transparent to user

**High-latency plugins:**
- Linear-phase EQs
- Lookahead limiters
- Some analyzers

## Troubleshooting

### Plugin Not Found

**Check:**
- Plugin is installed
- Plugin is in scanned folder
- Plugin is not blacklisted
- Correct plugin format for OS

**Solution:**
- Rescan plugins
- Add plugin folder to scan paths
- Check `deadmanspedal.txt`

### Plugin Crashes BespokeSynth

**Immediate:**
- Plugin automatically blacklisted
- Restart BespokeSynth
- Plugin won't load again

**Fix:**
- Update plugin to latest version
- Check plugin compatibility
- Contact plugin developer
- Use alternative plugin

### Plugin GUI Not Appearing

**Check:**
- Plugin supports GUI
- GUI not hidden behind other windows
- Try closing and reopening editor

**macOS specific:**
- Check System Preferences → Security
- Allow BespokeSynth to control computer

### Audio Glitches

**Causes:**
- Plugin too CPU-intensive
- Buffer size too small
- Plugin has bugs

**Solutions:**
- Increase buffer size
- Freeze/bounce plugin to audio
- Use lighter plugin
- Reduce polyphony

### Parameters Not Responding

**Check:**
- Parameter is exposed (click load parameter)
- Parameter is not automated by plugin
- Parameter is not locked in plugin

**Solution:**
- Reload parameter list
- Check plugin documentation
- Try different parameter

### Preset Not Loading

**Check:**
- Preset file exists
- Preset compatible with plugin version
- Preset in correct format

**Solution:**
- Re-save preset
- Update plugin
- Use plugin's native preset browser

## Performance Optimization

### CPU Usage

**Reduce CPU load:**
1. **Freeze tracks** - Bounce to audio when done editing
2. **Reduce polyphony** - Lower voice count in plugin
3. **Disable unused plugins** - Bypass when not needed
4. **Use efficient plugins** - Some plugins are lighter than others
5. **Increase buffer size** - Reduces CPU load, increases latency

### Memory Usage

**Reduce memory:**
1. **Unload unused plugins** - Delete vstplugin modules
2. **Use smaller sample libraries** - Reduce sample size
3. **Limit plugin instances** - Don't load too many at once

### Latency

**Reduce latency:**
1. **Decrease buffer size** - Lower latency, higher CPU
2. **Avoid high-latency plugins** - Check plugin latency
3. **Use zero-latency mode** - If plugin supports it

## Best Practices

### Organization

1. **Name modules** - "vstplugin_serum", "vstplugin_reverb"
2. **Group by type** - Instruments together, effects together
3. **Use prefabs** - Save common plugin setups
4. **Document settings** - Note important parameter values

### Workflow

1. **Load plugins last** - Build patch with native modules first
2. **Test before saving** - Verify plugin loads correctly
3. **Save often** - Plugin state saved with patch
4. **Backup presets** - Export plugin presets separately

### Performance

1. **Monitor CPU** - Watch CPU meter
2. **Freeze when possible** - Bounce to audio
3. **Use native modules** - When equivalent exists
4. **Limit instances** - Don't overload system

## Common Plugins

### Instruments

**Synths:**
- Serum, Massive, Sylenth1
- Diva, Repro, Monark
- Omnisphere, Kontakt

**Samplers:**
- Kontakt, UVI Falcon
- HALion, Play

**Drums:**
- Battery, Addictive Drums
- Superior Drummer, BFD

### Effects

**Dynamics:**
- FabFilter Pro-C, Pro-MB
- Waves CLA-76, SSL Comp

**EQ:**
- FabFilter Pro-Q
- Waves SSL E-Channel

**Reverb:**
- Valhalla Room, VintageVerb
- Lexicon, Eventide

**Delay:**
- Echoboy, H-Delay
- Valhalla Delay

**Modulation:**
- Soundtoys PhaseMistress
- Valhalla Freq Echo

## Resources

### Plugin Formats

- [VST SDK](https://www.steinberg.net/vst-developer/) - VST development
- [LV2 Specification](https://lv2plug.in/) - LV2 format
- [JUCE Framework](https://juce.com/) - Plugin development framework

### Plugin Directories

- [KVR Audio](https://www.kvraudio.com/) - Plugin database
- [Plugin Boutique](https://www.pluginboutique.com/) - Plugin store
- [VST4Free](https://www.vst4free.com/) - Free plugins

### Troubleshooting

- [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ) - Community support
- [GitHub Issues](https://github.com/BespokeSynth/BespokeSynth/issues) - Bug reports

## Next Steps

- **[MIDI Mapping](midi-mapping.md)** - Control VST parameters with MIDI
- **[Performance Tips](performance-tips.md)** - Optimize plugin performance
- **[Scripting](../scripting/README.md)** - Automate VST parameters with Python
