# Other Modules

Utility modules and special functions.

## Overview

These modules provide essential utilities, organization tools, and special functions that don't fit into other categories.

## Essential Modules

### output
**Audio output (required)**

Final destination for all audio. Cannot be deleted.

**Key controls**:
- **volume**: Master volume
- **limiter**: Prevent clipping
- **meter**: VU meter display
- **write audio**: Record last 30 minutes

**Tips**:
- Always connect audio here
- Use limiter to prevent clipping
- Monitor meter for levels
- Write audio to capture performances

### transport
**Global tempo and timing (required)**

Controls global tempo and time signature. Cannot be deleted.

**Key controls**:
- **bpm**: Tempo in beats per minute
- **timesigtop**: Beats per measure
- **timesigbottom**: Note value per beat
- **swing**: Timing swing amount

**Tips**:
- All musical timing references this
- Adjust BPM for tempo changes
- Swing adds groove
- Time signature affects measure length

### scale
**Global scale/key (required)**

Sets global musical scale and key. Cannot be deleted.

**Key controls**:
- **root**: Root note (C, D, E, etc.)
- **scale**: Scale type (major, minor, etc.)

**Tips**:
- Affects all scale-aware modules
- Use with notesequencer in scale mode
- Change for key changes
- Ensures musical coherence

## Organization

### comment
**Text comments**

Add text notes to your patch.

**Key controls**:
- **text**: Comment text
- **size**: Text size

**Common uses**: Documentation, notes, reminders

**Tips**:
- Document complex patches
- Add usage instructions
- Note parameter settings
- Explain signal flow

### label
**Text labels**

Visual text labels for organization.

**Key controls**:
- **text**: Label text

**Common uses**: Section labels, markers

### prefab
**Module groups**

Save and load groups of modules.

**Key controls**:
- **save**: Save current children
- **load**: Load saved prefab

**Common uses**: Reusable module groups, templates, sharing

**Tips**:
- Drag modules onto prefab
- Save effect chains
- Share with others
- Build library of prefabs

## State Management

### snapshots
**Save/recall presets**

Save and recall parameter states.

**Key controls**:
- **store**: Save current state
- **slots**: Recall saved states
- **blend**: Blend between snapshots

**Common uses**: Presets, live performance, A/B comparison

**Tips**:
- Save different sounds
- Use for live performance
- Blend for smooth transitions
- Map to MIDI controller

### savestateloader
**Load save states**

Load different save states without menu.

**Key controls**:
- **file**: Save state to load
- **load**: Load button

**Common uses**: Song sections, arrangements

## Scripting

### script
**Python scripting**

Run Python code for custom behavior.

**Key controls**:
- **code editor**: Write Python code
- **run**: Execute script
- **stop**: Stop script

**Common uses**: Custom logic, automation, generative music

**Tips**:
- See [Scripting Guide](../scripting/README.md)
- Powerful for custom behavior
- Access to full Python
- Can control any module

### scriptstatus
**Script status display**

Shows status of script modules.

**Common uses**: Monitoring scripts, debugging

## MIDI & OSC

### midioutput
**Send MIDI out**

Send MIDI to external devices.

**Key controls**:
- **device**: MIDI output device
- **channel**: MIDI channel

**Common uses**: Controlling external synths, DAW integration

### oscoutput
**Send OSC messages**

Send OSC (Open Sound Control) messages.

**Key controls**:
- **address**: OSC address
- **port**: OSC port

**Common uses**: Controlling external software, visuals, lighting

### clockin
**External clock input**

Receive external MIDI clock.

**Key controls**:
- **device**: MIDI clock source

**Common uses**: Sync to external gear, DAW sync

### clockout
**External clock output**

Send MIDI clock to external devices.

**Key controls**:
- **device**: MIDI clock destination

**Common uses**: Sync external gear to BespokeSynth

## Sync & Timing

### abletonlink
**Ableton Link sync**

Sync with other Ableton Link-enabled software.

**Key controls**:
- **enabled**: Enable Link
- **bpm**: Shared tempo

**Common uses**: Multi-app sync, collaboration, live performance

**Tips**:
- Sync with Ableton Live
- Sync with other Link apps
- Wireless sync
- Great for live performance

### timerdisplay
**Display current time**

Shows current playback time.

**Common uses**: Monitoring, timing reference

### timelinecontrol
**Timeline control**

Control playback timeline.

**Key controls**:
- **play/stop**: Playback control
- **position**: Playback position

**Common uses**: Arrangement, composition

## Controllers

### grid
**Grid controller interface**

Interface for grid controllers (Launchpad, etc.).

**Key controls**:
- **device**: Grid controller device
- **layout**: Button layout

**Common uses**: Performance, sequencing, control

**Tips**:
- Requires grid controller hardware
- LED feedback
- Multiple layouts
- Great for live performance

### push2control
**Ableton Push 2 control**

Dedicated control for Ableton Push 2.

**Key controls**:
- **device**: Push 2 device

**Common uses**: Performance, control, display

**Tips**:
- Requires Push 2 hardware
- Screen integration
- Comprehensive control
- Professional performance tool

## Utilities

### groupcontrol
**Control multiple parameters**

Control multiple parameters with one slider.

**Key controls**:
- **value**: Control value
- **targets**: Target parameters

**Common uses**: Macro controls, performance

### globalcontrols
**Global parameter controls**

Access to global settings.

**Common uses**: System settings, global parameters

### selector
**Select between inputs**

Choose between multiple inputs.

**Key controls**:
- **inputs**: Input options
- **select**: Selected input

**Common uses**: Switching, routing, selection

### valuestream
**Stream values**

Stream parameter values over time.

**Key controls**:
- **values**: Value sequence
- **interval**: Timing

**Common uses**: Automation, sequencing

## Recording & Playback

### looper
**Audio looper**

Record and loop audio.

**Key controls**:
- **record**: Start/stop recording
- **clear**: Clear loop
- **overdub**: Layer recordings
- **speed**: Playback speed

**Common uses**: Live looping, layering, performance

**Tips**:
- Build up layers
- Overdub for complexity
- Clear to start over
- Speed for effects

### multitrackrecorder
**Multi-track recorder**

Record multiple tracks simultaneously.

**Key controls**:
- **tracks**: Individual track controls
- **record**: Global record
- **play**: Playback

**Common uses**: Recording, arrangement, mixing

### samplebrowser
**Browse samples**

Browse and load audio samples.

**Key controls**:
- **folder**: Sample folder
- **preview**: Preview samples

**Common uses**: Finding samples, auditioning

## Visualization

### eventcanvas
**Event visualization**

Visualize events and timing.

**Common uses**: Debugging, monitoring, learning

### waveformviewer
**Waveform display**

Display audio waveform.

**Common uses**: Monitoring, visualization

### spectrum
**Spectrum analyzer**

Display frequency spectrum.

**Key controls**:
- **range**: Frequency range
- **resolution**: Analysis resolution

**Common uses**: Mixing, analysis, visualization

## Composition

### songbuilder
**Song arrangement**

Arrange song sections.

**Key controls**:
- **sections**: Song sections
- **arrangement**: Section order

**Common uses**: Song structure, arrangement, composition

### radiosequencer
**Radio-style sequencer**

Sequence with radio button selection.

**Key controls**:
- **options**: Sequence options
- **current**: Current selection

**Common uses**: Step selection, patterns

## Tips & Best Practices

### Organization
1. **Use comments**: Document your patches
2. **Use labels**: Mark sections
3. **Use prefabs**: Reusable groups
4. **Keep it clean**: Delete unused modules
5. **Organize spatially**: Group related modules

### Performance
1. **Use snapshots**: Quick preset changes
2. **Map controllers**: MIDI/OSC control
3. **Use grid**: Hands-on control
4. **Practice**: Test before performing
5. **Have backups**: Save multiple versions

### Workflow
1. **Save often**: Use snapshots and save states
2. **Test thoroughly**: Verify all connections
3. **Document**: Use comments
4. **Organize**: Keep patches clean
5. **Share**: Use prefabs to share

## Next Steps

- **[Scripting](../scripting/README.md)** - Python scripting guide
- **[Advanced Topics](../advanced/README.md)** - MIDI, OSC, and more
- **[User Guide](../user-guide/README.md)** - Interface and workflow

