# Ableton Link

Complete guide to synchronizing BespokeSynth with other applications using Ableton Link.

## Overview

Ableton Link is a technology that synchronizes musical beat, tempo, and phase across multiple applications running on one or more devices. With Link, you can:

- **Sync tempo** across multiple apps
- **Stay in phase** - everyone plays on the same beat
- **Start/stop independently** - each app can play/pause without affecting others
- **Change tempo together** - anyone can adjust tempo, all apps follow
- **Join/leave sessions** - apps discover each other automatically
- **Network jamming** - sync across multiple computers on same network

## Quick Start

### Basic Setup

1. **Create Ableton Link Module**
   ```
   Right-click → abletonlink
   ```

2. **Enable Link**
   - Module appears with Link status
   - Link automatically enabled
   - Searches for other Link-enabled apps

3. **Start Another Link App**
   - Open Ableton Live, or
   - Open another BespokeSynth instance, or
   - Open any Link-enabled app

4. **Automatic Sync**
   - Apps discover each other
   - Tempo syncs automatically
   - Beat phase aligns
   - You're jamming!

## Ableton Link Module

### Interface

**Status Display:**
- **peers** - Number of connected Link devices
- **tempo** - Current Link tempo (BPM)
- **beat** - Current beat position

**Controls:**
- **offset ms** - Timing offset adjustment (-1000 to +1000 ms)
- **reset next downbeat** - Force beat 0 on next downbeat
- **enabled** - Enable/disable Link sync

### How It Works

**Automatic Discovery:**
- Link uses local network to find other Link apps
- No manual configuration needed
- Apps appear/disappear automatically

**Tempo Sync:**
- Any app can change tempo
- All connected apps follow immediately
- Smooth tempo transitions

**Phase Sync:**
- All apps play on same beat
- Maintains phase relationship
- Corrects drift automatically

## Using Link

### With Ableton Live

**Setup:**
1. Enable Link in Ableton Live (top bar)
2. Create abletonlink module in BespokeSynth
3. Both apps show 1 peer connected
4. Tempo syncs automatically

**Workflow:**
- Set tempo in either app
- Both apps follow
- Start/stop independently
- Stay in sync

### With Multiple BespokeSynth Instances

**Setup:**
1. Launch multiple BespokeSynth instances
2. Create abletonlink module in each
3. All instances sync automatically

**Use cases:**
- Multi-computer performances
- Separate instances for different parts
- Collaborative jamming
- Redundancy (backup instance)

### With Mobile Apps

**Compatible apps:**
- Ableton Live
- Ableton Note
- Reason
- Traktor
- Serato
- Audiobus (iOS)
- Many more...

**Setup:**
1. Ensure all devices on same WiFi network
2. Enable Link in each app
3. Apps discover each other automatically

## Advanced Features

### Timing Offset

**Adjust sync timing:**
- **offset ms** slider (-1000 to +1000 ms)
- Compensates for network latency
- Aligns audio with other apps
- Useful for network performances

**When to use:**
- Audio from different apps not aligned
- Network latency causing drift
- Need to compensate for processing delay

**How to adjust:**
1. Play both apps simultaneously
2. Listen for timing difference
3. Adjust offset ms until aligned
4. Fine-tune by ear

### Reset Downbeat

**Force beat alignment:**
- Click **reset next downbeat** button
- On next measure, beat resets to 0
- All Link apps align to new downbeat
- Useful for starting fresh

**When to use:**
- Apps out of phase
- Starting new section
- Recovering from drift
- Beginning performance

### Tempo Changes

**Change tempo:**
- Adjust tempo in BespokeSynth transport
- All Link apps follow
- Smooth transition
- No phase disruption

**Or:**
- Change tempo in any Link app
- BespokeSynth follows
- Tempo updated automatically

### Start/Stop State

**Independent playback:**
- Each app can start/stop independently
- Tempo and phase remain synced
- One app stopping doesn't affect others
- Useful for live performance

**Synchronized start:**
- Some Link apps support synchronized start/stop
- BespokeSynth currently doesn't send start/stop state
- But maintains tempo/phase sync

## Network Configuration

### Local Network (LAN)

**Requirements:**
- All devices on same local network
- WiFi or Ethernet
- No special router configuration needed

**Best practices:**
- Use 5GHz WiFi for lower latency
- Ethernet preferred for stability
- Avoid congested networks
- Use quality router

### WiFi vs Ethernet

**WiFi:**
- ✅ Convenient
- ✅ Mobile devices
- ❌ Higher latency
- ❌ Potential dropouts

**Ethernet:**
- ✅ Lower latency
- ✅ More stable
- ✅ Recommended for critical sync
- ❌ Less convenient

### Multiple Networks

**Link works on:**
- Local WiFi network
- Wired Ethernet
- Mixed WiFi/Ethernet
- Ad-hoc networks

**Link does NOT work:**
- Across different networks
- Over internet (WAN)
- Through VPN (usually)
- Across subnets (usually)

## Troubleshooting

### No Peers Found

**Check:**
- All devices on same network
- Link enabled in all apps
- Firewall not blocking Link
- Network allows multicast

**Solutions:**
- Verify network connection
- Disable firewall temporarily
- Restart Link apps
- Restart router

### Tempo Not Syncing

**Check:**
- Link module enabled
- Tempo not locked in other app
- No conflicting tempo automation

**Solutions:**
- Disable/re-enable Link
- Restart both apps
- Check for tempo automation

### Drift/Phase Issues

**Causes:**
- Network latency
- CPU overload
- Audio buffer issues

**Solutions:**
- Adjust offset ms
- Use wired connection
- Reduce CPU load
- Increase audio buffer size
- Click reset next downbeat

### High Latency

**Causes:**
- WiFi congestion
- Network traffic
- Distance from router

**Solutions:**
- Use 5GHz WiFi
- Switch to Ethernet
- Move closer to router
- Reduce network traffic

### Firewall Blocking

**macOS:**
1. System Preferences → Security & Privacy
2. Firewall → Firewall Options
3. Allow BespokeSynth
4. Allow incoming connections

**Windows:**
1. Windows Defender Firewall
2. Allow an app through firewall
3. Add BespokeSynth
4. Allow Private and Public networks

**Linux:**
```bash
sudo ufw allow from 192.168.1.0/24  # Your network
```

## Performance Tips

### Reduce Latency

1. **Use Ethernet** - Most stable, lowest latency
2. **5GHz WiFi** - Better than 2.4GHz
3. **Quality router** - Invest in good network hardware
4. **Reduce distance** - Stay close to router
5. **Dedicated network** - Separate from internet traffic

### Optimize Sync

1. **Adjust offset** - Compensate for latency
2. **Monitor CPU** - Keep CPU usage low
3. **Stable tempo** - Avoid rapid tempo changes
4. **Reset when needed** - Use reset downbeat button

### Network Setup

1. **Static IPs** - Prevent DHCP changes
2. **QoS settings** - Prioritize Link traffic
3. **Disable WiFi power saving** - Prevents dropouts
4. **Wired for critical devices** - Ethernet for main computer

## Use Cases

### Live Performance

**Setup:**
- BespokeSynth on main computer
- Ableton Live on laptop
- Both synced via Link
- Independent control of each

**Benefits:**
- Redundancy (backup system)
- Separate control surfaces
- Different parts on different systems
- Seamless tempo changes

### Collaborative Jamming

**Setup:**
- Multiple musicians
- Each with Link-enabled app
- All on same network
- Synced tempo and phase

**Benefits:**
- Everyone in time
- Anyone can change tempo
- Independent start/stop
- Spontaneous collaboration

### DJ Setup

**Setup:**
- BespokeSynth for live synthesis
- Traktor/Serato for DJing
- Both synced via Link
- Seamless integration

**Benefits:**
- Synth stays in time with tracks
- Smooth transitions
- Live remixing
- Creative possibilities

### Studio Production

**Setup:**
- BespokeSynth for sound design
- Ableton Live for arrangement
- Both synced via Link
- Record output to Live

**Benefits:**
- Tempo-synced recording
- Live parameter changes
- Integrated workflow
- Best of both worlds

## Link-Compatible Apps

### DAWs
- Ableton Live
- Reason
- Bitwig Studio
- FL Studio (beta)

### DJ Software
- Traktor
- Serato DJ
- djay Pro
- Mixxx

### Mobile Apps (iOS)
- Ableton Note
- Audiobus
- AUM
- Patterning
- Figure
- Many more...

### Other
- VCV Rack
- Max/MSP (with Link object)
- Pure Data (with Link external)
- SuperCollider (with Link quark)

## Technical Details

### Protocol

**Link uses:**
- UDP multicast for discovery
- Peer-to-peer synchronization
- No central server
- Automatic network discovery

**Port:**
- UDP port 20808
- Multicast address 224.76.78.75

### Precision

**Link provides:**
- Microsecond-level timing
- Automatic drift correction
- Network latency compensation
- Stable synchronization

### Quantum

**Beat quantum:**
- Link syncs to beat quantum (usually 4 beats)
- Ensures phase alignment
- Prevents mid-measure jumps
- Smooth transitions

## Resources

### Official

- [Ableton Link Website](https://www.ableton.com/link/)
- [Link Developer Documentation](https://ableton.github.io/link/)
- [Link GitHub](https://github.com/Ableton/link)

### Compatible Apps

- [Link-Enabled Apps List](https://www.ableton.com/link/products/)
- Regularly updated
- Hundreds of apps

### Community

- [BespokeSynth Discord](https://discord.gg/YdTMkvvpZZ)
- [Ableton Forum](https://forum.ableton.com/)

## Next Steps

- **[MIDI Mapping](midi-mapping.md)** - Control parameters
- **[OSC Control](osc-control.md)** - Network control
- **[Performance Tips](performance-tips.md)** - Optimize for live use
