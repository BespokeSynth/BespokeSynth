# Saving & Loading

Learn how to manage your BespokeSynth projects and recordings.

## Save States

BespokeSynth saves your entire patch as a "save state" file.

### What Gets Saved

A save state includes:
- ✅ All modules and their positions
- ✅ All patch cable connections
- ✅ All parameter values
- ✅ Module settings and configurations
- ✅ Snapshots
- ✅ Bookmarks

### What Doesn't Get Saved

- ❌ Audio output (use "write audio" to record)
- ❌ Current playback position
- ❌ Temporary UI states
- ❌ External MIDI/OSC mappings (saved separately)

## Saving Your Work

### Quick Save

**Keyboard shortcut**:
- Press `Ctrl + S` (Windows/Linux)
- Press `Cmd + S` (macOS)

**What happens**:
- If file already has a name: Overwrites existing file
- If new file: Opens save dialog

### Save As

**Using menu**:
1. Click **File** menu (or equivalent)
2. Select **Save As**
3. Choose location
4. Enter filename
5. Click **Save**

**File format**: `.bsk` (BespokeSynth save state)

### Save Dialog

When saving for the first time:

1. **Choose location**: Navigate to desired folder
2. **Enter filename**: Give your patch a descriptive name
3. **Click Save**: File is saved

**Naming tips**:
- Use descriptive names: `ambient_pad_patch.bsk`
- Include version numbers: `drum_beat_v2.bsk`
- Organize by project: `project_name/patch_name.bsk`

## Loading Patches

### Quick Load

**Keyboard shortcut**:
- Press `Ctrl + L` (Windows/Linux)
- Press `Cmd + L` (macOS)

**What happens**:
- Opens file browser
- Navigate to `.bsk` file
- Select and open

### Load Dialog

1. Press `Ctrl/Cmd + L`
2. Navigate to save file location
3. Select `.bsk` file
4. Click **Open**
5. Patch loads

**Warning**: Loading a patch replaces your current patch. Save first if needed!

### Drag and Drop

**Alternative method**:
1. Open file browser (Windows Explorer, Finder, etc.)
2. Locate `.bsk` file
3. Drag file onto BespokeSynth window
4. Patch loads

## Auto-Save

BespokeSynth automatically creates backup saves.

### How Auto-Save Works

- Saves periodically while you work
- Creates `.bsk.backup` files
- Doesn't interrupt your workflow
- Helps recover from crashes

### Finding Auto-Save Files

**Location**:
- Same folder as your save file
- Named: `yourpatch.bsk.backup`

**To recover**:
1. Find `.bsk.backup` file
2. Rename to `.bsk`
3. Load normally

## File Organization

Keep your patches organized for easy access.

### Recommended Structure

```
BespokeSynth/
├── Projects/
│   ├── Project1/
│   │   ├── main_patch.bsk
│   │   ├── variation1.bsk
│   │   └── variation2.bsk
│   └── Project2/
│       └── project2_patch.bsk
├── Templates/
│   ├── drum_template.bsk
│   ├── synth_template.bsk
│   └── effect_chain_template.bsk
├── Experiments/
│   └── test_patches.bsk
└── Performances/
    ├── live_set1.bsk
    └── live_set2.bsk
```

### Organization Tips

1. **Use folders**: Group related patches
2. **Name descriptively**: Know what patch does from name
3. **Version control**: Keep old versions (v1, v2, etc.)
4. **Separate templates**: Reusable starting points
5. **Archive old work**: Move completed projects to archive folder

## Recording Audio

Save your audio output to file.

### Using "Write Audio" Button

**On output module**:
1. Click **write audio** button
2. Recording starts immediately
3. Play your patch
4. Click **write audio** again to stop
5. Audio file saved

### Audio File Location

**Default location**:
- Windows: `Documents/BespokeSynth/recordings/`
- macOS: `~/Documents/BespokeSynth/recordings/`
- Linux: `~/BespokeSynth/recordings/`

**File format**: WAV (uncompressed, high quality)

**Filename**: Automatically generated with timestamp

### Recording Tips

1. **Set levels first**: Check output meter, avoid clipping
2. **Record longer**: Easier to trim than re-record
3. **Use limiter**: Output module has built-in limiter
4. **Monitor while recording**: Listen for issues
5. **Save patch first**: So you can recreate if needed

## Looper Recording

Record loops within BespokeSynth.

### Using Looper Module

1. Create **looper** module
2. Connect audio source to looper
3. Connect looper to output
4. Click **record** button
5. Play audio to record
6. Click **record** again to stop
7. Loop plays back automatically

### Looper Features

- **Overdub**: Record additional layers
- **Clear**: Erase loop
- **Save**: Export loop to file
- **Load**: Import audio file as loop

## Snapshots

Save and recall different parameter states.

### Creating Snapshots

1. Create **snapshots** module
2. Adjust parameters to desired state
3. Click **store** button in snapshots module
4. Snapshot saved to slot

### Recalling Snapshots

- Click snapshot slot to recall
- Parameters smoothly transition to saved state
- Use for preset changes, live performance

### Snapshot Uses

- **Live performance**: Quick preset changes
- **A/B comparison**: Compare different settings
- **Variations**: Save different versions of same patch
- **Automation**: Trigger snapshots with MIDI/OSC

## Prefabs

Save reusable module groups.

### Creating Prefabs

1. Create **prefab** module
2. Drag modules onto prefab
3. Modules become children of prefab
4. Click **save** in prefab module
5. Name and save prefab

### Loading Prefabs

1. Right-click canvas
2. Navigate to **prefabs** menu
3. Select saved prefab
4. Place on canvas

### Prefab Uses

- **Effect chains**: Reusable effect combinations
- **Instrument templates**: Pre-configured instruments
- **Utility groups**: Common module combinations
- **Sharing**: Share prefabs with others

## Templates

Create starting points for new patches.

### Creating Templates

1. Build a patch with common modules
2. Remove project-specific elements
3. Save as template (e.g., `drum_template.bsk`)
4. Store in Templates folder

### Using Templates

1. Load template file
2. Immediately "Save As" with new name
3. Modify for current project
4. Original template remains unchanged

### Template Ideas

- **Drum kit setup**: Drumsequencer + drumplayer + effects
- **Synth voice**: Oscillator + filter + envelope + effects
- **Effect chain**: Common effect combinations
- **Performance rig**: MIDI controller + snapshots + instruments

## Backup Strategies

Protect your work from loss.

### Regular Backups

1. **Save frequently**: `Ctrl/Cmd + S` often
2. **Version incrementally**: patch_v1, patch_v2, etc.
3. **External backup**: Copy to cloud storage or external drive
4. **Export audio**: Record important performances

### Cloud Storage

**Recommended**:
- Dropbox, Google Drive, OneDrive
- Automatic sync
- Version history
- Access from multiple computers

**Setup**:
1. Create BespokeSynth folder in cloud storage
2. Save patches there
3. Automatic backup

### Version Control (Advanced)

**For developers**:
- Use Git for patch version control
- Track changes over time
- Collaborate with others
- `.bsk` files are JSON (text-based, Git-friendly)

## File Formats

### .bsk Files

**BespokeSynth save state**:
- JSON format (human-readable text)
- Contains entire patch
- Can be edited in text editor (advanced users)
- Shareable with other BespokeSynth users

### .bskt Files

**BespokeSynth template**:
- Same format as .bsk
- Indicates it's a template
- Optional, not required

### Audio Files

**Recordings**:
- WAV format (uncompressed)
- 44.1kHz or 48kHz sample rate
- 16-bit or 24-bit depth
- Stereo

## Sharing Patches

Share your creations with others.

### Preparing to Share

1. **Test thoroughly**: Ensure patch works
2. **Remove personal paths**: Check for hardcoded file paths
3. **Document**: Add comment modules explaining patch
4. **Save clean**: Remove unused modules
5. **Include samples**: If using samples, include them or note requirements

### Sharing Methods

**Discord community**:
- Upload `.bsk` file
- Describe what it does
- Include audio demo if possible

**GitHub**:
- Create repository
- Include `.bsk` file
- Add README with description
- Include audio examples

**Personal website**:
- Host `.bsk` files
- Provide download links
- Include documentation

### Receiving Patches

**When loading others' patches**:
1. **Scan for scripts**: Check script modules for safety
2. **Check file paths**: May need to update sample paths
3. **Verify VST plugins**: Ensure you have required plugins
4. **Read documentation**: Understand how patch works

## Troubleshooting

### Can't Load Patch

**Possible causes**:
- File corrupted
- Created in newer BespokeSynth version
- Missing required modules/plugins

**Solutions**:
1. Try loading `.bsk.backup` file
2. Update BespokeSynth to latest version
3. Check error messages for clues
4. Ask for help on Discord

### Patch Sounds Different After Loading

**Possible causes**:
- Different audio settings (sample rate, buffer size)
- Missing VST plugins
- Different BespokeSynth version
- External MIDI/OSC mappings not saved

**Solutions**:
1. Check audio settings match original
2. Install missing plugins
3. Recreate MIDI/OSC mappings
4. Save MIDI mappings separately

### Recording Not Working

**Check**:
- Is output module connected?
- Is audio playing?
- Is write audio button enabled?
- Check recordings folder exists
- Verify disk space available

## Best Practices

### Do's ✅

- Save frequently (`Ctrl/Cmd + S`)
- Use descriptive filenames
- Organize patches in folders
- Keep backups (cloud storage)
- Version your patches (v1, v2, etc.)
- Record important performances
- Test patches after loading

### Don'ts ❌

- Don't rely on auto-save alone
- Don't use generic names ("patch1.bsk")
- Don't forget to save before loading
- Don't ignore backup files
- Don't share patches without testing
- Don't forget to document complex patches

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Save | `Ctrl/Cmd + S` |
| Load | `Ctrl/Cmd + L` |

## Next Steps

Explore more features:
- **[Module Reference](../modules/README.md)** - Learn about all modules
- **[Scripting](../scripting/README.md)** - Automate with Python
- **[Advanced Topics](../advanced/README.md)** - MIDI, OSC, and more

