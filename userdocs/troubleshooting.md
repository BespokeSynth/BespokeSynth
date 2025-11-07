# Troubleshooting

This guide covers common issues and their solutions when using BespokeSynth.

## Table of Contents
- [Audio Issues](#audio-issues)
- [Performance Issues](#performance-issues)
- [MIDI Issues](#midi-issues)
- [VST Plugin Issues](#vst-plugin-issues)
- [Installation Issues](#installation-issues)
- [Crashes and Stability](#crashes-and-stability)
- [Python Scripting Issues](#python-scripting-issues)
- [File and Save Issues](#file-and-save-issues)
- [Display and UI Issues](#display-and-ui-issues)

## Audio Issues

### No Sound Output

**Symptoms**: BespokeSynth is running but you hear no audio.

**Solutions**:
1. **Check audio device settings**:
   - Click Settings (gear icon)
   - Verify correct audio output device is selected
   - Try different sample rates (44100 or 48000 Hz)
   - Adjust buffer size (try 512 samples)

2. **Check module connections**:
   - Ensure your audio chain connects to the **output** module
   - Verify cables are properly connected (they should be visible)

3. **Check volume levels**:
   - Look at the **output** module's meter - is it showing signal?
   - Check your system volume
   - Check if audio is paused (Shift+P to unpause)

4. **Restart audio engine**:
   - Close and reopen Settings
   - Or restart BespokeSynth

### Audio Crackling or Dropouts

**Symptoms**: Audio plays but has clicks, pops, or dropouts.

**Solutions**:
1. **Increase buffer size**:
   - Settings → Buffer Size → Try 512 or 1024 samples
   - Larger buffers = more latency but more stable

2. **Reduce CPU load**:
   - Delete unused modules
   - Disable expensive effects (reverb, granulators)
   - See [Performance Tips](advanced/performance-tips.md)

3. **Close other applications**:
   - Close CPU-intensive programs
   - Disable background processes

4. **Check sample rate**:
   - Ensure BespokeSynth sample rate matches your audio interface
   - Common rates: 44100 or 48000 Hz

### High Latency

**Symptoms**: Noticeable delay between input and sound output.

**Solutions**:
1. **Decrease buffer size**:
   - Settings → Buffer Size → Try 128 or 256 samples
   - Smaller buffers = less latency but less stable

2. **Use ASIO (Windows)**:
   - Install ASIO drivers for your audio interface
   - Select ASIO device in Settings

3. **Optimize system**:
   - Close unnecessary applications
   - Disable Wi-Fi/Bluetooth if not needed

## Performance Issues

### High CPU Usage

**Symptoms**: BespokeSynth uses excessive CPU, causing slowdowns.

**Solutions**:
1. **Identify CPU-heavy modules**:
   - Modules with many voices (polyphonic synths)
   - Reverb and delay effects
   - Granular processors
   - Multiple VST plugins

2. **Optimize your patch**:
   - Reduce polyphony on synths
   - Use simpler effects
   - Freeze/bounce audio tracks
   - Delete unused modules

3. **Increase buffer size**:
   - Settings → Buffer Size → 512 or 1024

4. **Disable visual features**:
   - Minimize spectrum analyzers
   - Reduce waveform displays

### Slow UI / Laggy Interface

**Symptoms**: Canvas navigation or module interaction is slow.

**Solutions**:
1. **Reduce module count**:
   - Delete unused modules
   - Use prefabs to organize complex patches

2. **Disable minimap**:
   - Settings → Disable minimap if enabled

3. **Update graphics drivers**:
   - Ensure your GPU drivers are current

4. **Reduce window size**:
   - Smaller window = less rendering overhead

## MIDI Issues

### MIDI Controller Not Detected

**Symptoms**: Your MIDI controller doesn't appear in BespokeSynth.

**Solutions**:
1. **Check physical connection**:
   - Ensure MIDI device is plugged in
   - Try a different USB port
   - Check device power (if required)

2. **Verify system recognition**:
   - Windows: Device Manager → Sound, video and game controllers
   - macOS: Audio MIDI Setup → MIDI Studio
   - Linux: `aconnect -l` in terminal

3. **Restart BespokeSynth**:
   - Close and reopen BespokeSynth after connecting MIDI device

4. **Check midicontroller module**:
   - Create a **midicontroller** module
   - Click the controller dropdown
   - Your device should appear in the list

### MIDI Notes Not Playing

**Symptoms**: MIDI controller is connected but notes don't play.

**Solutions**:
1. **Check MIDI routing**:
   - Ensure **midicontroller** is connected to a synth or instrument
   - Verify cable connections

2. **Check MIDI channel**:
   - In **midicontroller**, verify the channel setting
   - Try setting to "all channels"

3. **Test with keyboarddisplay**:
   - Create a **keyboarddisplay** module
   - Click notes to verify audio chain works

### MIDI Mapping Not Working

**Symptoms**: MIDI controls don't affect parameters.

**Solutions**:
1. **Enable bind mode**:
   - In **midicontroller**, enable "bind (hold shift)"
   - Hover over target control
   - Hold Shift and move your MIDI control

2. **Check mapping list**:
   - Verify mapping appears in midicontroller's mapping list
   - Check control numbers match

3. **Verify MIDI message type**:
   - Ensure you're using CC (Control Change) messages
   - Some controllers send different message types

## VST Plugin Issues

### VST Plugin Not Found

**Symptoms**: Your VST plugins don't appear in BespokeSynth.

**Solutions**:
1. **Set VST paths**:
   - Settings → VST Paths
   - Add folders containing your VST plugins
   - Click "Rescan"

2. **Check plugin format**:
   - Ensure plugins are VST2, VST3, or LV2 format
   - Match architecture (64-bit BespokeSynth needs 64-bit plugins)

3. **Verify plugin installation**:
   - Ensure plugins work in other DAWs
   - Reinstall problematic plugins

### VST Plugin Crashes BespokeSynth

**Symptoms**: BespokeSynth crashes when loading certain plugins.

**Solutions**:
1. **Update plugin**:
   - Check for plugin updates from manufacturer
   - Some older plugins may be incompatible

2. **Blacklist problematic plugin**:
   - Note which plugin causes crashes
   - Remove from VST path or don't use it

3. **Check plugin compatibility**:
   - Some plugins require specific system configurations
   - Check manufacturer's compatibility list

## Installation Issues

### macOS: "App is damaged" Error

**Symptoms**: macOS prevents BespokeSynth from opening.

**Solutions**:
1. **Remove quarantine attribute**:
   ```bash
   xattr -cr /Applications/BespokeSynth.app
   ```

2. **Allow in Security Settings**:
   - System Preferences → Security & Privacy
   - Click "Open Anyway" for BespokeSynth

### Windows: Missing DLL Errors

**Symptoms**: Windows shows missing DLL errors on launch.

**Solutions**:
1. **Install Visual C++ Redistributables**:
   - Download from Microsoft's website
   - Install both x86 and x64 versions

2. **Reinstall BespokeSynth**:
   - Uninstall completely
   - Download fresh installer
   - Reinstall

### Linux: Permission Errors

**Symptoms**: BespokeSynth won't run due to permissions.

**Solutions**:
1. **Make executable**:
   ```bash
   chmod +x BespokeSynth
   ```

2. **Check audio group membership**:
   ```bash
   sudo usermod -a -G audio $USER
   ```
   - Log out and back in

## Crashes and Stability

### Random Crashes

**Symptoms**: BespokeSynth crashes unexpectedly.

**Solutions**:
1. **Update BespokeSynth**:
   - Download latest version from bespokesynth.com
   - Check [GitHub releases](https://github.com/BespokeSynth/BespokeSynth/releases)

2. **Check save file**:
   - Try loading a different save state
   - Corrupted saves can cause crashes

3. **Disable problematic modules**:
   - Note what you were doing when crash occurred
   - Avoid that module/action temporarily

4. **Report the bug**:
   - File an issue on [GitHub](https://github.com/BespokeSynth/BespokeSynth/issues)
   - Include crash logs and steps to reproduce

### Crash on Startup

**Symptoms**: BespokeSynth crashes immediately when launched.

**Solutions**:
1. **Delete preferences**:
   - Windows: `%APPDATA%/BespokeSynth/`
   - macOS: `~/Library/Preferences/BespokeSynth/`
   - Linux: `~/.config/BespokeSynth/`
   - Rename or delete the folder

2. **Reset to defaults**:
   - Delete `userprefs.json` from data folder

3. **Check graphics drivers**:
   - Update GPU drivers
   - Try different graphics settings

## Python Scripting Issues

### Python Scripts Not Running

**Symptoms**: Script module shows errors or doesn't execute.

**Solutions**:
1. **Check Python installation**:
   - BespokeSynth requires Python 3.x
   - Verify Python is in system PATH

2. **Check script syntax**:
   - Look for syntax errors in script
   - Check indentation (Python is whitespace-sensitive)

3. **View error messages**:
   - Script module shows errors in red
   - Read error messages for clues

### "Untrusted Script" Warning

**Symptoms**: Script won't run due to security warning.

**Solutions**:
1. **Review script carefully**:
   - Ensure script is safe to run
   - Check for malicious code

2. **Trust the script**:
   - If script is safe, click to trust it
   - BespokeSynth remembers trusted scripts

### Import Errors

**Symptoms**: Python imports fail in scripts.

**Solutions**:
1. **Install required packages**:
   ```bash
   pip install <package-name>
   ```

2. **Check Python environment**:
   - Ensure packages are installed for correct Python version
   - BespokeSynth uses system Python

## File and Save Issues

### Can't Load Save File

**Symptoms**: Save file won't load or shows errors.

**Solutions**:
1. **Check file format**:
   - Ensure file is `.bsk` or `.bskt` format
   - Don't rename file extensions

2. **Try older version**:
   - Save files from newer versions may not work in older BespokeSynth
   - Update to latest version

3. **Recover from backup**:
   - Check for auto-save files in data folder
   - Look for `.bsk.backup` files

### Save File Corruption

**Symptoms**: Save file is corrupted or won't open.

**Solutions**:
1. **Check backup files**:
   - BespokeSynth creates backups automatically
   - Look in save folder for `.backup` files

2. **Edit save file manually**:
   - Save files are JSON format
   - Can be edited in text editor (advanced users only)

## Display and UI Issues

### Modules Off-Screen

**Symptoms**: Can't find modules, they're off the visible canvas.

**Solutions**:
1. **Reset view**:
   - Press `~` to open console
   - Type `home` and press Enter

2. **Use minimap**:
   - Enable minimap in Settings
   - Click on minimap to navigate

3. **Search for module**:
   - Press `~` to open console
   - Type module name to highlight it

### Text Too Small/Large

**Symptoms**: UI text is difficult to read.

**Solutions**:
1. **Adjust zoom**:
   - Hold Spacebar + scroll to zoom
   - Or scroll in empty space

2. **Change display scaling**:
   - Adjust OS display scaling settings
   - Restart BespokeSynth

### Colors Look Wrong

**Symptoms**: Module colors or UI appears incorrect.

**Solutions**:
1. **Check graphics drivers**:
   - Update GPU drivers

2. **Try different renderer**:
   - Settings → Graphics options
   - Try different rendering modes if available

## Getting More Help

If your issue isn't covered here:

1. **Search existing issues**: [GitHub Issues](https://github.com/BespokeSynth/BespokeSynth/issues)
2. **Ask on Discord**: [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ)
3. **File a bug report**: [New Issue](https://github.com/BespokeSynth/BespokeSynth/issues/new)

When reporting issues, include:
- BespokeSynth version
- Operating system and version
- Steps to reproduce the problem
- Error messages or crash logs
- Save file if relevant (and safe to share)

