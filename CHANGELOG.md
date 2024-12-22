# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).


## [1.3.0] - 2024-12-22

### Added

- Added a callback for sysex messages in scripts
- Added level meters to `gain` and `audiometer` modules
- Added settings for `minimap` margin and corner
- Added a new dropdown mode for setting step velocity in the `drumsequencer` module
- Added a new "list" mode to the `snapshots` module, and make that the default mode
- Added the ability to set a separate samples path
- Added Akai LPD8 midi mapping
- Added the `audiosplitter` module
- Added an option to turn off auto correlation on the background Lissajous
- Added the `pulselimit` module
- Added the ability to pin modules with F3 so they stay in view
- Added a "home" button to move the viewport home in case you get lost
- Added various controller layouts
- Added the `dataprovider` module
- Added support for command line options
- Added the `savestateloader` module, for loading into another savestate file
- Added controller layouts for the M-Audio Oxygen Pro 49
- Added environment variable for setting bespoke data dir
- Added support for template projects (.bskt)
- Added 14-bit midi CC support, and added integration for custom "bespoke turn" controller
- Added the `euclideansequencer` module
- Added a new loop capture method to `looperrecorder`, so you can start recording/stop recording and it rounds to the nearest number of bars
- Added voiceId to the `notedisplayer`
- Added "pan" to `modulationvisualizer` and made the module resizable
- Added `get_name` python method to get the name of modules
- Added automatic octave rescaling to the `keyboarddisplay` module
- Added hover based keyboard capture to the `keyboarddisplay` module
- Added octave labels to the `keyboarddisplay` module
- Added the ability to jump octaves via keyboard keybinds to the `keyboarddisplay` module
- Added an option in the settings to hide labels to the `keyboarddisplay` module
- Added another option that enforces a fixed number of octaves regardless of the keyboard size to the `keyboarddisplay` module
- Added the `controlrecorder` module
- Added major thirds layout to the `launchpadkeyboard` module
- Added mouse click based velocity support to the `keyboarddisplay` module
- Added the ability for the user to configure their target refresh rate
- Added the `pulserouter` module
- Added the `voicesetter` module
- Added controller layout for music thing modular m0 controller
- Added the `label` module
- Added a new `set_target` script method with an additional `cableSourceIndex` parameter
- Added a checkbox to toggle the resetting of the transport on `songbuilder` scene change
- Added the ability to disable spacemouse control from the command line
- Added a mode to `oscillator` and `signalgenerator` sync to allow the frequency to be a ratio
- Added a system for saving module presets, and spawning them from quickspawn
- Added the `pitchtovalue` module
- Added the `dotsequencer` module, a polyphonic note sequencer
- Added the `rhythmsequencer` note effect
- Added controller definition for Code 25 USB MIDI
- Added previewing to `samplebrowser`
- Added sync pulse support to `m185sequencer`


### Changed

- Made it so you can target dropdown lists with pulse cables
- Updated ExprTk to 0.0.3
- Changed the behavior, with regards to velocity, of the `notesequencer` and `dotsequencer` to be in line with the `drumsequencer`
- Made it so you can target RadioButtons with pulse cables
- Made hovering over a placed step and pressing up or down, cycle between normal, accent, and ghost notes in the `drumsequencer` module
- Tweaked appearance of `drumsequencer` grid and change default step velocity to 80%
- Changed AllowLookahead default to on in the `notelooper` module
- Added a stop button when the sample is playing in the `samplebrowser` module
- Allowed displaying the full note range in the `keyboarddisplay` module
- Don't smooth delay time when there's a modulator controlling delay, to allow for interesting LFO-driven delay effects
- Made the `pulsetrain`'s first pulse in sync with the transport instead of whenever the activating pulse comes in
- Made `pulsetrain` resizable
- Increased the limit of the `pulsetrain` to 128
- Added the advancement buttons back to the `pulsesequencer`
- Increased the maximum length from 32 to 128 in the `pulsesequencer`
- Made the `pulsesequencer` resizable
- Made it so the `pitchsetter` does not flush the notes when the pitch slider was changed but tries to preserve polyphony as much as possible instead
- Increase midi pages to 32 from 10
- Allow a single `notetable` module to span the entire note range
- Make the key that opens the console also close the console
- Made the `circlesequencer` support up to 32nd notes
- Show the sample filename in the `seaofgrain` module
- Allow for snapping to the center of the module while holding `shift` as well as `control` as opposed to only the top left corner
- Minor rendering optimization when the modules `lissajous`, `spectrum` and `waveformviewer` are disabled/bypassed
- Changed the following modules to use a bypass mechanism when disabled: `gain`, `audiometer`, `fftvocoder`, `dcoffset`, `feedback`, `inverter`, `lissajous`, `looper`, `looperrecorder`, `multitapdelay`, `pulsechance`, `pulsedelayer`, `pulseflag`, `samplecapturer`, `samplergrid`, `seaofgrains`, `send`, `spectrum`, `signalclamp`, `vocoder`, `waveformviewer` and `waveshaper`
- Made `spectrum` and `waveformviewer` function when not connected
- Made it possible to use a PC keyboard as a `midicontroller` input
- Made it possible to lock keyboard focus to the `keyboarddisplay` module (and other future keyboard takeover modules)
- Updated quickspawn search sorting to prefer things that start with what's been typed so far
- Made it possible for pulse cables to directly connect to buttons and checkboxes
- Made the `pulser` "free" mode timer restartable
- Made `pulsesequence` step default to aligning to global transport, like other similar devices
- Adjusted transparency of midi slider interaction popup
- Made spawned presets use the filename as their default name
- Made it possible for rotary encoders to use "set" mode to increment and decrement in the `midicontroller` module
- Changed `beats` module to only use a single column
- Made it possible to set the target using scripts for the modules: `audiotocv`, `envelope`, `leveltocv`, and `vinylcontrol`
- Made it possible for `snapshots` to target grids and canvases
- Added the ability to manually lay out the module grid view using push2, so you can curate the modules you want quick access to
- Added a second grid view to `notesequencer` and `notetable` modules
- Don't include input audio when in "input*" mode in `karplusstrong`, so you only hear the feedback


### Fixed

- Fixed `rhythmsequencer` crash when trying to set length above 8
- Prevent quickspawn menu from popping up if you move the mouse between pressing shift twice
- Fix parsing of Python script files in CR+LF format
- Fixed several bugs in the rendering of child modules in the `minimap`
- Fixed a bug where the `minimap` could be obscured by the titlebar menu
- Fixed an issues related to saving a `seaofgrain` at one sample rate and loading it at another sample rate
- Fixed a bug where singleton modules could be added to `prefab`'s which caused all kinds of visual duplication and incorrect behavior
- Fixed a bug where the `scale` values in the `midicontroller` weren't being saved
- Fixed an issue where long sample names prevented you from clicking the play button in the `samplebrowser` module
- Fixed `capo` not working properly when adjusting held notes while in diatonic mode
- Fixed LFO's not properly being recorded and set when using the snapshots module
- Several fixes to the biquad filter to better handle NaN values
- Fixed a bug where the LFO would output NaN values when the length was set to zero
- Fixed a bug when loading old savestates when oversampling was set to anything larger than 1
- Fixed a crash in `keyboarddisplay` module when notes greater than 127 in value where drawn
- Fixed the octave note text being off by 2 octaves in the `keyboarddisplay` module
- Fixed a bug where the lowest octave displayed in the `keyboarddisplay` module wasn't the lowest possible
- Fixed a bug where the `keyboarddisplay` module was using float instead of double for time calculations
- Fixed an issue where some old savestates couldn't be loaded due to an issue in the past where the saving of the names of submodules of the effectchain module weren't saving their name
- Enhanced `midicontroller` deviceOutName creation to handle midi device names having " In " and " Out "
- Don't display the help text when a savestate is loaded through the command line arguments
- Fixed a bug where all canvas like modules wouldn't save and load note modulations
- Don't render the finished step in the `pulsetrain` module
- Fixed a crash in the `pulsesequencer` when going beyond the maximum length
- Fixed a bug where the shift buttons on the `pulsesequencer` weren't shifting
- Fixed a bug in the `drumsequencer` when the numrows triangle menu option was set lower than a previously saved snapshot causing "disabled" rows to still send notes
- Fixed the default for modwheel
- Increased internal work buffer size so we don't read and write beyond this when using oversampling and large buffer sizes
- Hidden the `drumplayer` controls better when not in edit mode and having single outputs
- Fixed a crash in the `songbuilder` module when a new connection cable was canceled
- Changed the order of input handling in module containers so that the menu UI is handled first
- Resolved an issue in the `oscillator` module that would cause the module to continuously propagate NaN values
- Forced a poll to properly rebuild the buffers when the `multitrackrecorder` is cleared
- Correctly set the number of channels within `seaofgrain` so that mono input is processed correctly
- Made the minimap less blurry with certain aspect ratios
- Rewrote `pitchtovalue` through valuesetter in order to set verbatim values for drop-down lists
- Fixed a crash on switch from "Jump" to "Switch" mode in the `songbuilder` module
- Fixed an issue where some offscreen sliders wouldn't compute their modulation
- Fixed `circlesequencer` playnote issues when using offsets
- Simplify bespoke turn string communication, and fix issue where value string wasn't updating frequently enough
- Fixed a crash when the interval in the `curvelooper` is forcibly set to 0
- Set `curvelooper` size on load
- Random note buttons would set all notes to 3 when the random octave range is 0-0. Now notes map to drum notes 0-3 again if octave range is 0-0
- Fixed an issue where holding space when bespoke isn't focused would cause mouse movements to pan the canvas
- Fixed an issue where mouse wrap wouldn't pan on the 0 side if the window is fully maximized
- Don't use file path hash for plugins lookup
- When the `valuesetter` is disabled also don't allow sending updates when the button is pressed
- Fixed a minor bug related to visualization of cables after a gain module
- Fixed a bug that inhibited removing the voicelimit in the `oscillator`, `fmsynth` and `karplusstrong` modules
- Fixed a bug in the `ringmodulator` module where it would not compute the sliders for every sample correctly
- Fixed a bug with the visualization of the output cable on a `looper`
- Refresh the parameter dropdown list on ParamInfo change so that plugins with less than 30 params and changing params work correctly
- Fixed a crash when setting the values of dropdown menus out of bounds on the `label` module
- Fixed `pitchtovalue` not accepting notes
- Made `dotsequencer` contents savable in `snapshots`
- Propagate keypress in `eq` module so that cables can be deleted with the key commands
- Fixed an issue where `notecanvas` did not import correctly for most tempos
- Fixed a vstplugin assert when adjusting preset dropdown from push2
- Fixed an issue with `snapshot` cable sources overlapping each other when multiple modules are connected
- Made monome grid refresh lights when reconnected
- Fixed a crash caused by a `snapshot` module being in a weird state
- Use temp file to make sure save files don't get corrupted if a crash ever happens mid-save
- Fixed an issue where a step is skipped after reset when the first step has no pulses in the `m185sequencer` module
- Fixed an issue where cables within prefabs can't be grabbed when `push2control` is being used


### Removed

- Removed the enable/bypass checkbox on the `splitter`
- Removed hidden main patchcablesource from the modules: `audiotocv`, `envelope`, `leveltocv`, and `vinylcontrol`


## [1.2.1] - 2023-09-16

### Added

- Added total duration to songbuilder sequencer
- Added corner radius setting to the `globalcontrols` module
- Added the ability to execute "console" commands directly from OSC using the `/bespoke/console` address
- Added `buffershuffler` features: fourtet effect and ability to only play when triggered
- Added new "cable_alpha" option in settings and in the `globalcontrols` module that allows you to reduce cables opacity to nearly invisible
- Added an option to wrap mouse around when panning canvas


### Changed

- Made `valuesetter` force value updates
- Adjusted transport minimum tempo to 20 to sidestep weird issues when tempo is lower
- Allow toggling enabling state of prefabs, this will attempt to toggle the enabled state of all modules contained within
- Note cables have been made less thick
- Audio, note, and pulse cables now draw behind modules
- Changed how settings menu is positioned so it always spawns on-screen
- Adjusted background color alpha of `prefab` to make it less imposing-looking
- Changed how pulse width parameter affects sawtooth wave


### Fixed

- Fixed an issue where savestates with `effectchain` modules saved prior to the 2022-11-25 build would not load
- Fixed an issues with the absence of modulation using incorrect defaults
- Fixed NaN when adjusting slider with out-of-range value
- Fixed velocity not working in drumplayer
- Fixed an issue where `effectchain` mix sliders couldn't be targeted by modulators
- Fixed a bug in the cosine window of the granulators that caused clicky noises at the end of grains
- Fixed a deadlock by upgrading JUCE that was causing startup hangs on MacOS 13


## [1.2.0] - 2023-07-13

### Added 

- Added button to the `abletonlink` module to allow users to align their measure count phase with peers
- Added checkboxes to enable deterministic modes on `notechance` and `pulsechance`
- Added a setting that allows changing the behavior of pasting onto UI controls
- Added drum pad mode to `gridkeyboard`
- Added `buffershuffler` module for slicing live input
- Added Push 2 integrations to many modules
- Added option to keep VST preset from resetting exposed param sliders
- Added polyphony to `notetable`
- Added interface to allow better control over storing/deleting snapshots via `midicontroller`
- Added `extend` button to `looper`
- Added option to snapshot module to auto-store entries when switching between snapshots
- Added ability to delete snapshots by holding alt and clicking
- Added the ability to send notes and pulses directly to modules through OSC
- Added the ability to toggle minimize and enabled state of modules through OSC
- Added a way to move to a module as well as zoom to it through OSC
- Added outline stroke to held modules
- Added `looperrecorder` option to disable auto-advancing to the next `looper`
- Added `pulseflag` module to explicitly control pulse types
- Added setting to `radiosequencer` to disable connected controls when it is disabled
- Added "skip first" option to `noteratchet`
- Added option to the `notestepper` module to toggle allow chord mode
- Added ability for OSC to set and recall the viewport/minimap "bookmarks"
- Added the ability for OSC messages to interact directly with UI controls
- Added user documentation for the OSC features
- Added triangle menu option to allow `notelooper` to work with lookahead sequencers
- Added (computationally expensive) mode to recalculate `EQ` params per-sample
- Added ability to choose cable drop behavior in user preferences
- Warn about and visualize circular dependencies amongst audio modules
- Added triangle menu option to hide inactive cables for `udiorouter`and `oterouter`
- Added ability to release a cable into empty space to display the quickspawn menu
- Make `timelinecontrol` a real module
- Added `songbuilder` module
- Added timestamps to all control updates for sample-accurate actions
- Added "diatonic" checkboxes to `capo` and `scaledegree`
- Added num_rows option to `radiosequencer`
- Added retrigger option to `noteoctaver`
- Added `boundstopulse` module
- New quickspawn features: right click to browse, right click + type to search, press shift twice quickly to launch quick spawn menu
- Added slider mode to `controlsequencer`
- Added deterministic mode to `velocitytochance`
- Added ability to remove samples from `beats` module
- Added `portamento` glide mode
- Added modes to allow `radiosequencer` to run slower
- Added deterministic modes to notechance and `pulsechance`
- Added option for hockets to be repeatedly random
- Added aesthetic touch to cables, to fade them out in the middle
- Added a setting that allows the disabling of the module highlights when they receive an event
- Added `velocitycurve` module
- Added `noteecho` module, to make note delay effect workflow clearer
- Added option to `midicontroller` triangle menu to suppress UI popup
- Added left/right buttons to browse drum sample directory
- Added the ability for scale and transport to randomize on BSK load
- Added `velocitytochance` module
- Added midi import and export to `notecanvas`
- Snap to grid: hold control while dragging a module, grid size is configurable in the settings menu
- New midi sync modules: `clockin` and `clockout`
- Added basic ableton link support
- Added get_targets() method to script
- Added ability to send arbitrary midi messages to VST plugins via the `script` module
- Added slider mode to valuesetter, for easier modulation
- Added more parameters to script's midicontroller.set_connection()


### Changed

- Don't clear `looperrecorder` input buffer when committing to a `looper`
- Made canvas contents snapshottable across all modules with canvases
- Improved midi `clockin` smoothness by switching to "delay-locked loop" setup
- Made the `transport` increase/decrease buttons jump to integer tempos, as a convenience to hit an exact integer tempo
- Allow disabling of `output` limiter
- In the `tranpsort` module made nudge finer-grained, and made reset start a stopped transport
- Added ability to stop passthrough on `looper`
- Make `looperrecorder` not temporarily silence after committing a loop, added a triangle menu checkbox to bring the old behavior back
- Listen to midi clock start/stop/reset messages, make start also resume playback
- Improved the `chorddisplayer` module
- Allow the scroll wheel to adjust the Q slider while hovering over a dot in the `eq` module
- Make it possible to enable/disable the `quantizer` module
- Changed how grid controllers edit `drumsequencer`, making it more responsive
- Feedback is now given when `write audio` completes
- Made the room size slider non linear in the `freeverb` effect
- Allow the `freeverb` roomsize value to go to `1` since this also works perfectly without feedback
- When doing a "set" from the `midicontroller`, always force a value update
- Massive RAM savings by changing how global record buffer saves
- Sort the file list in the vstplugin's preset dropdown
- Renamed `presets` to `snapshots`, to better reflect what the module does
- Allow the OSC control messages to work in percentage mode
- Select all text when tabbing through text entries 
- Make dropdowns close if you click the dropdown again
- If the current savestate filename is known use it in the prefix for multitrack recorder bounces
- Allowed changing the number of filter options in the notesorter module
- Made playsequencer resizable and fixed a grid size bug
- When hovering over an ADSR control, highlight the part of the curve that dragging would adjust
- Made the `pulsebutton` send pulses a sample later so that things like the `notecreator` work with it
- Make notedisplayer re-sizable
- Patch cables that carry NaN values are made visible and colored red
- Make ctrl-C copy hovered UI control value to clipboard
- Typing while holding a patch cable now behaves like clicking then typing
- Change up ADSR evaluation paradigm to handle thread safety better
- `Script` module: only automatically turn on scheduling lookahead when a note cable is connected
- Don't enforce `looper` target when it's associated with a `looperrecorder`
- Change default zoom level and ui scale of bespoke to 1.3
- Tweak that allows scripts to set modulator targets
- Change up `notesequencer` grid to make it easier to vary note length
- Allow modulators to directly connect to multiple UI controls, no macroslider necessary
- Queue all non-audio thread notes
- Make modulators send non-normalized values to non-floatslider controls
- Make ? help display scrollable
- Don't display duplicate plugins user has in different formats
- Added an option to have `velocitytocv` output 0 when a note is released
- Make `notecounter` module have deterministic random setting
- Improve scrolling on quickspawn menu
- Don't allow two modules to have the same name
- Distinguish "value setter"-style modulators from the continuous ones
- Make deterministic hocket lengths defined by sliders
- Added buttons to make it more straightforward to increment/decrement seed for `notehocket`, `pulsehocket`, `notechance` and `pulsechance`
- Change how `notesequencer` pitch randomization works, to make it sound more musical by default
- ADSR updates: make decay/release snappier by default, improve envelope editor
- Use gestures to detect when VST controls have been touched
- Optimizations to cable drawing
- Make `chorder` emit notes from lowest to highest
- Sort script files in dropdown
- Change `karplusstrong` feedback range
- Added pitch color reference to `chorder` grid
- Change up `drumsequencer` per-row randomization
- `Compressor` has new "drive" parameter
- `Clockout` has new "send start" button and multiplier
- `Sampleplayer` accepts pulse input
- Show raw values when showing last `midicontroller` input


### Fixed

- Fixed `basiceq` not saving state
- Fixed issue where oversampling caused global record buffer to save at wrong rate
- Fixed some modulation inconsistencies
- Fixed a crash on load caused by effectchains with deleted prior effects of the same type
- Move the timestamp check so that `MultitrackRecorder` bounces have a timestamp even when the savestate wasn't saved yet
- Clean up data when removing a mapping in the `macroslider`
- Fixed `notecanvas` arrows causing notes to double-move
- Remove 0 from ADSR sliders to avoid the ability to cause NaNs
- Initialize default values for modulation curves, fixes issue where `notecanvas` or `notelooper` notes sounded different from notes played from `keyboarddisplay`, `midicontroller`, etc
- Made the `groupcontrol` module apply to all connections instead of only the first
- Fixed crash when clicking `scale`'s "load SCL" or "load KBM" buttons via a `midicontroller`
- Fixed issue where opening a plugin with a modal popup will endlessly spam the popup
- Fixed a bug where `LFO` and `EnvelopeEditor` modules would not restore their minimized state
- Fixed a bug where the `midioutput` would not correctly handle the UseVoiceAsChannel setting for modulation
- Fixed a bug where the ramper would never reach its target value
- Fixed push2control not saving bookmarks and favorites 
- Fixed that prefabs (or containers) were saving twice
- Fixed EnvelopeEditor modulation when minimized
- Hard limit the number of `effectchain` modules instead of a crashing
- Made the `effectchain` effect controls untargetable by modulation cables since this crashes when modules are deleted
- Fixed cables not moving to the correct location when a module that has direct children is minimized
- Fixed notecreator not disabling correctly
- Remove cables targeting controls on a deleting module
- Fixed the `circlesequencer` not triggering a note when a circle is set to `1`
- Made file extensions mostly case insensitive on all platforms
- Refresh the file list in a `vstplugin` so that MIDI/OSC and other modulation work on load
- Fixed a bug where note off's would turn off all notes with the same pitch even when specific voice id's were used
- Don't light up `notefilter` when velocity 0 is received 
- Fixed issues clicking cables inside `prefab`s
- Fixed 0 velocity note input setting preset
- Fixed `pumper` envelope not attacking properly
- `controlsequencer` wasn't saving self-advance mode setting
- Fixed `notegate` can fail to turn off notes
- Fixed a bug where Canvas Controls could be modulated
- Fixed incorrect location of grid icon on patch sources when modules are inside a `prefab`
- `EQ`: Keep the dots in view and minimal size
- Allow multiple instances of `sampleplayer`s to download different youtube videos at the same time
- Fixed null dereference when unhooking grid controller in some scenarios
- Fixed a division by zero if Pitch Per Octave was set to 0 (either by hand or through modulation)
- Improved IntSlider and FloatSlider accuracy especially when zoomed in
- Make the `notedelayer` module pass through the notes if it is disabled
- Fixed `expression` width
- Added a check in the `grid` module to see if the target isn't itself
- Allow scrolling in `prefab`s to zoom
- Fixed that you could drag `prefab` members from one prefab directly into another `prefab`
- Fixed using negative "widen" `panner` with mono input puts audio in left channel only
- Fixed issue where `prefab`s couldn't save to symlinked folders
- Fixed randomly-occurring dc offset on modules that use switch-and-ramp
- Don't allow `prefab` recursion (allowing it does not work correctly in many ways)
- Fixed issue where send's second target could get auto-assigned
- Fixed issue where you can use grid-snapping to drag a `prefab` into itself
- Fixed `notesequencer` sending note-off at wrong time in lookahead mode 
- Fixed input audio not working correctly when using oversampling
- Fixed issue where `presets` didn't load properly if pointed at large grids
- Fixed crash when saving a `sampler` at one sample rate and loading the project at a different sample rate
- Fixed issue where some minimized modules would hide their cables
- When loading a save file, restore the canvas location/zoom level
- Fixed saved samples playing back at the wrong speed when loading in other sample rates
- Fixed global oversampling issues
- Fixed syntax highlighting issue where comments not drawing in script
- Fixed basiceqs grid, which was not accepting clicks
- Fixed bug where `midicontroller` grid cables couldn't be connected 
- Handle param rename events from plugins
- Fixed deadlock when loading VCV Rack plugin and opening GUI
- Fixed modulator range not being set when attaching a modulator to a non-floatslider control
- Fixed modulator range not loading properly from a savestate
- Fixed issue with script-driven notes playing later than desired
- Fixed issue where `radiosequencer` steps can't be turned off via grid
- Fixed simple issue where key repeat wasn't being respected in quickspawn
- Make preset changes happen in main thread (to avoid audio thread hitch)
- Eliminate off-by-a-pixel discrepancies between mouse hover & mouse click
- Fixed crash when connecting a modulator a slider with a non-pinned LFO
- Fixed issue where intslider wasn't saving range
- Fixed issue where duplicating a modulator screws up the connection
- Fixed issue where resizing a module could also click controls
- Fixed potential crash if VST parameters mismatch
- Fixed modulators not connecting to VST parameters on load
- Fixed crash when spawning `basiceq` effect
- Fixed issue where directly spawning `effectchain` effects from quickspawn was giving the `effectchain` the name of the effect
- Fixed crash when spawning `oscoutput`
- Fixed `groupcontrol` loading improperly
- Fixed freeverb width range
- Various fixes to the `LFO` module
- Fixed the next and previous sample buttons in the `sampleplayer` pads on windows
- Fixed up some behavior to restore `midicontroller` "hotbind" functionality
- Fixed crash when midi output device is already in use
- Fixed loading of json when there is no device selected yet
- Sample `valuesetter`s modulation more accurately when pulsing
- Fixed modules disappearing when lassoing onto a `prefab`
- Fixed some of the juce leakdetector crashes on exit
- Fixed `notecanvas` and textentry issues
- Module title bar takes precedence over hovered UI control, to avoid pressing UI control when you intend to drag module
- Fixed sample drag n drop on `sampleplayer` failing when you're hovered over a control
- When `abletonlink` module is disabled, don't respond to link tempo changes
- Fixed `drumsequencer` copying incorrectly when increasing measure count
- Improve ableton link stability
- Fixed `m185sequencer` playing notes when all lengths are zero
- Fixed relative paths being busted when using `presets` within `prefab`s
- Fixed crash when clicking "add" on `midicontroller` in list mode in certain scenario
- Improve `midicontroller` infinite encoder support 


### Removed

- Removed the `drumsequencer` presets, and replace them with a "clear" button
- Removed a few options that were completely ignored in the `oscillator` module


### Security

- Security fix: allow users to review python scripts before running them


## [1.1.0] - 2021-11-16

### Added

- Added new example patches
- Added user preferences to disable/increase motion trails
- Added macos signed build
- Allow mouse offset customization
- Added script controls to get and set module info 
- Added user prefs to allow users to increase max channel count
- Added a VSTPlugin "Panic" button
- Added "copy build info" button to help menu
- Added a deterministic "random" number generator to script
- Added "reset" button to notestepper for better control
- Added paging to dropdown lists that are too big to fit on-screen
- Added `notetable` module
- Added `notetoggle` module
- Allow `radiosequencer`s to enable multiple outputs at a time
- Added checkbox to allow `sampleplayer` cues to stop on note-off 
- Added ability to lock `drumsequencer` rows to avoid randomness
- Hint that modulators can be dragged vertically to adjust min value
- Added category pages to settings menu
- Graphics options to disable lissajous and set background color
- Added ability to spawn and move modules by clicking rather than dragging
- Added the ability to hot load scripts when an external editor changes them
- Allow viewport panning with middle mouse button
- Added `minimap`
- Added a `play/pause` button to the `transport` module
- Added the ability to reset the phase of the signal generator
- Allow `*.bsk` files to be opened directly, or dropped onto the app window
- Added `NoteExpression` Router
- Show `effectchain` effects in "audio effects" dropdown on title bar
- Added support for SCL, oddsound and KBM
- Added script styles to the `script` module

### Changed

- Change `notetable` to have ascending default layout instead of random
- Make autopatch functionality more visible
- Make user preferences do "save and exit" if you've launched with an error
- Hide tooltips after typing or adjust canvas until the mouse moves again
- Updated `notesequencer` script interface
- Midi Controller sends CC out the MIDI pipe
- Updated to new random to fix warnings
- Use std::uniform_random rather than hand crafted
- Update slider smoother UI
- Tweak to how user data is copied over, and update user data version file
- `Drumsynth` quality updates
- Increase sequencing flexibility for `controlsequencer` and `radiosequencer`
- Make it possible to grab samples from `drumplayer` pads
- Increase patch cable click radius
- Made it possible to save without quitting, and just warn on preferences that require a restart
- Delete held cable if you right-click
- Overflow grid x values in the `drumplayer` to subsequent rows
- Use a project file stem as the saved recording prefix
- Don't allow out of range pitches through the `notefilter`
- Make textentry modulatable when it represents a number
- Make it possible to spawn LFOs directly
- Rename "reset layout" to "new patch" and add a confirmation popup
- Changes to dropdownlist and quickspawn to handle massive VST collections
- Using a more precise "center" value for endless encoders
- VST manager window with scanning as a sub process (prevents crashing Bespoke while scanning)
- Limit slider value entry to min and max range of the slider
- Make "delete" function as "backspace" everywhere that it makes sense
- "Chording" for quickspawn menu if you hold multiple keys, it will narrow down the quickspawn list
- Build a hard limiter into the output module, and worn users more explicitly when they're clipping
- VST Bus management (better handling of plugins that have more channels)
- Various improvements to the sample player user experience

### Fixed

- Fixed `midicontroller` control cable being inaccessible in list view
- Fixed `chordholder` sending wrong velocity when pulsed
- Draw `midicontroller` overlay in the ui layer
- Fixed tooltip flicker when using quickspawn menu
- Fixed typo where script's get_width() was returning height
- Fixed VST panic button using wrong channel
- Fixed issue where sliders with smoothers always ramped in from 0 on load
- Fixed `gridkeyboard` stuck notes issue that can happen with push2
- Fixed crash when copying `controlsequencer`, `radiosequencer`, or `script` modules
- Don't overwrite drums.json if a user has customized it
- Fixed up issue with user preference editor and ASIO
- Fixed python get_target would crash if the module has NO target
- Fixed file chooser being broken on some Linux configs
- Fixed `presets` module
- Fixed synth modules spitting out a bunch of notes all at once if they receive input while disconnected
- Fixed a number of `prefab` issues
- Fixed bespoke starting up with a window size larger than the display
- Fixed multiple `notecanvas` issues
- Fixed control cable disconnects via backspace 
- When there is a hovered UI control, force the mouse to interact with it
- Fixed deadlock on mac startup from yesterday's user preferences change
- Fixed dropdown drawing issues on push2
- Fixed crash when user preferences has unexpected JSON
- Fixed issue where clicking the triangle menu caused the module to get dragged around
- Fixed up some weirdness in settings UI with prefs that require restart
- Fixed recent mistake that was preventing old script BSKs from loading 
- Make scripts properly reload multiple note outputs 
- Fixed missing first notes on `notecanvas` when resetting the `transport`
- Fixed sliders on `notesequencer` overlapping
- Fixed mousewheel scroll behavior being broken on textentry
- Fixed `*.bsk` saving and loading on 32-bit
- Fixed issues with VSTs sorting incorrectly and not appearing in quickspawn
- Display bespoke version on error screen
- Fixed issue where triangle menu items could be targeted by modulators
- Fixed silly error that was keeping sustain pedal from working
- Fixed issue where quickspawn would place modules in the wrong position
- Fixed UI strangeness's when working while paused
- Fixed Locrian mode
- Fixed `pulser` not pulsing in "free" mode
- Fixed Shift+Hover to connect modules

### Removed

- Get rid of functionality where you could click a title bar to stick it to the cursor
- Remove old smoothing from `signalgenerator` so now it can be modulated sharply and accurately


## [1.0.0] - 2021-09-14

### Initial release

- First full public release

## Commit log diffs

[unreleased]: https://github.com/BespokeSynth/BespokeSynth/compare/v1.2.1...main
[1.2.1]: https://github.com/BespokeSynth/BespokeSynth/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/BespokeSynth/BespokeSynth/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/BespokeSynth/BespokeSynth/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/BespokeSynth/BespokeSynth/tree/v1.0.0
