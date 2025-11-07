# Navigation

Learn how to efficiently navigate the BespokeSynth canvas and organize your workspace.

## Canvas Basics

The BespokeSynth canvas is an infinite 2D workspace where you place and connect modules. Think of it as a virtual modular synthesizer case that can grow as large as you need.

### Canvas Elements

- **Grid background**: Helps with visual alignment
- **Modules**: Rectangular boxes containing controls
- **Patch cables**: Lines connecting modules
- **Minimap** (optional): Small overview in corner showing your position

## Panning the Canvas

Move around the canvas to view different areas of your patch.

### Method 1: Spacebar + Mouse
1. Hold `Spacebar`
2. Move mouse to pan
3. Release spacebar

**Best for**: Quick, precise movements

### Method 2: Right-Click Drag
1. Right-click in empty space
2. Drag to pan
3. Release mouse button

**Best for**: When your other hand is busy

### Method 3: Middle Mouse Button (if available)
1. Click and hold middle mouse button
2. Drag to pan
3. Release

**Best for**: Users with 3-button mice

## Zooming

Adjust your view to see more or less of the canvas.

### Method 1: Spacebar + Scroll
1. Hold `Spacebar`
2. Scroll mouse wheel or trackpad
3. Release spacebar

**Best for**: Precise zoom control

### Method 2: Scroll in Empty Space
1. Move mouse over empty canvas area
2. Scroll mouse wheel or trackpad

**Best for**: Quick zooming while working

### Zoom Levels

- **Zoomed out**: See entire patch layout, good for organization
- **Normal**: Default working zoom level
- **Zoomed in**: See details, good for precise control adjustments

**Tip**: Use zoom to focus on specific areas while working, then zoom out to see the big picture.

## Resetting Your View

Lost on the canvas? Reset to the default view.

### Using the Console
1. Press `~` (tilde key) to open console
2. Type `home`
3. Press `Enter`

This returns you to the initial position and zoom level.

### When to Reset
- You've panned far from your modules
- Canvas feels "lost" or disoriented
- Starting a new session

## Bookmarks and Locations

Save and recall specific canvas positions for quick navigation.

### Storing Bookmarks

**Using Minimap** (if enabled):
1. Navigate to desired location
2. Hold `Shift`
3. Click one of the 9 bookmark buttons at bottom of minimap

**Using Keyboard**:
1. Navigate to desired location
2. Press `Shift + [1-9]` to store bookmark

### Recalling Bookmarks

**Using Minimap**:
- Click bookmark button (without Shift)

**Using Keyboard**:
- Press `1-9` to recall bookmark

**Using Console**:
1. Press `~` to open console
2. Type `bespoke.location_recall(1)` (or any number 1-9)
3. Press `Enter`

### Bookmark Strategies

**Organize by function**:
- Bookmark 1: Drums section
- Bookmark 2: Melody section
- Bookmark 3: Effects section
- Bookmark 4: Master output

**Organize by workflow**:
- Bookmark 1: Performance view (main controls)
- Bookmark 2: Sequencing view (patterns)
- Bookmark 3: Sound design view (synth parameters)
- Bookmark 4: Mixing view (levels and effects)

## Minimap

The minimap provides a bird's-eye view of your entire patch.

### Enabling the Minimap
1. Click **Settings** (gear icon)
2. Enable **Minimap** option
3. Click **OK**

### Using the Minimap

**Navigate**:
- Click anywhere on minimap to jump to that location
- Your current view is shown as a rectangle

**Bookmarks**:
- 9 bookmark buttons appear at bottom
- Click to recall, Shift+click to store

**Overview**:
- See all modules at once
- Identify empty areas
- Plan layout organization

### When to Use Minimap

**Useful for**:
- Large, complex patches
- Finding specific modules quickly
- Organizing spatial layout
- Performance setups with multiple sections

**May disable for**:
- Simple patches
- Performance (saves CPU)
- Small screens (takes up space)

## Finding Modules

Lost a module? Several ways to locate it.

### Method 1: Search via Console
1. Press `~` to open console
2. Type the module name
3. The module will be highlighted
4. Pan to see it

### Method 2: Use Minimap
1. Enable minimap
2. Look for module on overview
3. Click to navigate

### Method 3: Zoom Out
1. Zoom out far enough to see all modules
2. Pan to locate module
3. Zoom back in

### Method 4: Reset and Search
1. Press `~`, type `home`, press Enter
2. Pan around from default position
3. Most modules should be near origin

## Workspace Organization

Organize your canvas for efficient workflow.

### Spatial Organization Strategies

**By Signal Flow** (left to right):
```
[Instruments] → [Note Effects] → [Synths] → [Audio Effects] → [Output]
```

**By Function** (grouped areas):
```
┌─────────────┐  ┌─────────────┐
│   Drums     │  │   Melody    │
└─────────────┘  └─────────────┘

┌─────────────┐  ┌─────────────┐
│   Bass      │  │   Effects   │
└─────────────┘  └─────────────┘
```

**By Layer** (vertical stacking):
```
[Layer 1: Drums]
[Layer 2: Bass]
[Layer 3: Melody]
[Layer 4: Pads]
[Layer 5: Effects]
```

### Organization Tips

1. **Leave space**: Don't cram modules together
2. **Align modules**: Use grid for visual clarity
3. **Group related modules**: Keep signal chains together
4. **Use consistent layout**: Develop your own style
5. **Plan before building**: Sketch complex patches first

### Cable Management

Keep cables organized for clarity:

1. **Minimize crossings**: Route cables to avoid tangles
2. **Use short paths**: Place connected modules near each other
3. **Vertical vs horizontal**: Choose consistent routing style
4. **Split strategically**: Use cable splits intentionally
5. **Delete unused cables**: Clean up as you work

## Pinning Modules

Pin modules to keep them visible while panning.

### How to Pin
1. Select a module (click on it)
2. Press `F3`
3. Module is now pinned

### How to Unpin
1. Select the pinned module
2. Press `F3` again
3. Module unpins

### When to Use Pinning

**Useful for**:
- Master output (always visible)
- Transport controls (always accessible)
- Performance controls (keep in view)
- Reference modules (scales, notes)

**Note**: Pinning only works when not group-selecting modules.

## Multi-Monitor Setup

Using multiple monitors? Here's how to optimize:

### Extending Canvas
- BespokeSynth window can span multiple monitors
- Maximize window across displays
- Use extra space for complex patches

### Organization Strategies
- **Monitor 1**: Main performance controls
- **Monitor 2**: Sequencers and patterns
- **Monitor 3**: Effects and mixing

### Considerations
- More screen space = larger patches possible
- May impact performance on some systems
- Useful for live performance setups

## Performance Considerations

Navigation can impact performance in large patches.

### Optimization Tips

1. **Disable minimap**: If not needed, saves CPU
2. **Reduce zoom changes**: Frequent zooming can be CPU-intensive
3. **Organize efficiently**: Well-organized patches are easier to navigate
4. **Use bookmarks**: Faster than panning long distances
5. **Delete unused modules**: Reduces visual clutter and CPU load

## Keyboard Shortcuts Summary

| Action | Shortcut |
|--------|----------|
| Pan canvas | `Spacebar + Drag` or `Right-click Drag` |
| Zoom | `Spacebar + Scroll` or `Scroll in empty space` |
| Reset view | `~` then `home` + `Enter` |
| Store bookmark | `Shift + [1-9]` |
| Recall bookmark | `[1-9]` |
| Pin/unpin module | `F3` |
| Open console | `~` |

## Tips and Tricks

### Quick Navigation
- **Double-tap spacebar**: Some users find this helps with muscle memory
- **Use bookmarks liberally**: Set them up early in your patch
- **Zoom out to plan**: See the big picture before adding modules

### Workflow Efficiency
- **Start organized**: Begin with good layout, maintain it
- **Use consistent patterns**: Develop your own layout style
- **Group before growing**: Organize small sections before expanding
- **Navigate while playing**: Learn to navigate without stopping audio

### Common Mistakes to Avoid
- ❌ Placing all modules in one area (hard to navigate)
- ❌ Not using bookmarks (waste time panning)
- ❌ Ignoring cable organization (visual mess)
- ❌ Forgetting to zoom out (lose big picture)
- ❌ Not learning keyboard shortcuts (slower workflow)

## Practice Exercises

### Exercise 1: Navigation Basics
1. Create 5 modules in different canvas areas
2. Practice panning to each one
3. Practice zooming in and out
4. Reset view with console

### Exercise 2: Bookmarks
1. Create modules in 4 different areas
2. Set bookmarks 1-4 for each area
3. Practice recalling bookmarks
4. Try keyboard shortcuts

### Exercise 3: Organization
1. Create a simple patch (10+ modules)
2. Organize by signal flow
3. Align modules neatly
4. Minimize cable crossings
5. Set bookmarks for different sections

## Next Steps

Now that you can navigate efficiently, learn about:
- **[Modules](modules.md)** - Creating and managing modules
- **[Patching](patching.md)** - Connecting modules together
- **[UI Controls](ui-controls.md)** - Working with module controls

