# OSC Control

Complete guide to controlling BespokeSynth with OSC (Open Sound Control).

## Overview

OSC (Open Sound Control) is a network protocol for communication between computers, sound synthesizers, and multimedia devices. BespokeSynth supports bidirectional OSC communication, allowing you to:

- Control parameters from other software
- Send data to other applications
- Build custom control interfaces
- Integrate with visual software
- Create networked performances

## Quick Start

### Receiving OSC Messages

1. **Create OSC Controller Module**
   ```
   Right-click → osccontroller
   ```

2. **Set Input Port**
   ```
   in port: 8000  (or any port number)
   ```

3. **Send OSC Message**
   From another app (TouchOSC, Max/MSP, etc.):
   ```
   /bespoke/control/synth~volume 0.5
   ```

4. **Parameter Updates**
   The synth volume parameter changes to 0.5!

### Sending OSC Messages

1. **Create OSC Output Module**
   ```
   Right-click → oscoutput
   ```

2. **Configure Output**
   ```
   address: 192.168.1.100  (target IP)
   port: 9000              (target port)
   ```

3. **Send from Script**
   ```python
   out = oscoutput.get("oscoutput")
   out.send_float("/my/parameter", 0.75)
   ```

## OSC Controller Module

### Setup

**Module Creation:**
```
Right-click → osccontroller
```

**Configuration:**
- **in port** - Port to listen on (default: 8000)
- **out address** - IP address to send to (optional)
- **out port** - Port to send to (optional)

### Receiving OSC

The osccontroller automatically receives OSC messages and converts them to MIDI-style control messages.

**Supported message types:**
- Float values (0.0 - 1.0)
- Integer values (0 - 127)
- String values
- Multi-parameter messages

### OSC Address Patterns

#### Control Parameters

**Format:**
```
/bespoke/control/<module>~<parameter> <value>
```

**Examples:**
```
/bespoke/control/synth~volume 0.5
/bespoke/control/filter~cutoff 1000
/bespoke/control/delay~feedback 0.3
```

**Scaled Control (0-1 range):**
```
/bespoke/control_scaled/<module>~<parameter> <value>
```

This maps 0.0-1.0 to the parameter's full range.

#### Send Notes

**Format:**
```
/bespoke/note <pitch> <velocity>
```

**With channel:**
```
/bespoke/note <channel> <pitch> <velocity>
```

**Examples:**
```
/bespoke/note 60 100        # Middle C, velocity 100
/bespoke/note 1 60 100      # Channel 1, Middle C
```

#### Module-Specific Messages

**Send note to specific module:**
```
/bespoke/module/note/<module_name> <pitch> <velocity>
```

**Example:**
```
/bespoke/module/note/synth 60 100
```

#### Console Commands

**Execute console commands:**
```
/bespoke/console <command>
```

**Example:**
```
/bespoke/console "load patch.bsk"
```

### Custom OSC Addresses

**Any OSC address works:**
```
/my/custom/address 0.5
```

The osccontroller automatically creates a mapping for new addresses.

**Access in scripts:**
```python
def on_osc(message):
    print(message)  # "/my/custom/address 0.5"
```

## OSC Output Module

### Setup

**Module Creation:**
```
Right-click → oscoutput
```

**Configuration:**
- **address** - Target IP address
- **port** - Target port number
- **label** - Optional label for note output

### Sending OSC from Scripts

**Get module reference:**
```python
out = oscoutput.get("oscoutput")
```

**Send float:**
```python
out.send_float("/my/parameter", 0.75)
```

**Send integer:**
```python
out.send_int("/my/counter", 42)
```

**Send string:**
```python
out.send_string("/my/message", "hello")
```

### Automatic Parameter Feedback

**Connect sliders to oscoutput:**
1. Create sliders on oscoutput module
2. Name them appropriately
3. Values automatically sent as OSC

**Example:**
```
Slider "volume" → sends /bespoke/volume <value>
```

### Note Output

**Set note output label:**
```
label: "notes"
```

**Notes automatically sent:**
```
/bespoke/notes <pitch> <velocity>
```

## Python Scripting Integration

### Receiving OSC in Scripts

**Enable OSC input:**
```python
me.connect_osc_input(8000)  # Listen on port 8000
```

**Handle OSC messages:**
```python
def on_osc(message):
    # message format: "/address value1 value2 ..."
    parts = message.split()
    address = parts[0]
    
    if address == "/trigger":
        me.play_note(60, 100, 1.0/4)
    
    elif address == "/param":
        value = float(parts[1])
        me.set("slider1", value)
```

### Sending OSC from Scripts

**Method 1: Use oscoutput module**
```python
out = oscoutput.get("oscoutput")
out.send_float("/my/param", 0.5)
```

**Method 2: Use osccontroller (bidirectional)**
```python
# OSC controller automatically sends to configured output
# Just receive and process messages
```

## Common Use Cases

### TouchOSC Integration

**Setup:**
1. Install TouchOSC on tablet/phone
2. Create osccontroller in BespokeSynth
3. Set port (e.g., 8000)
4. Configure TouchOSC to send to computer's IP:8000

**TouchOSC Layout:**
```
Fader 1 → /bespoke/control/synth~volume
Fader 2 → /bespoke/control/filter~cutoff
Button 1 → /bespoke/note 60 100
XY Pad → /bespoke/control/effect~x and /effect~y
```

### Max/MSP Integration

**Send from Max:**
```
[udpsend <ip> <port>]
|
[prepend /bespoke/control/synth~volume]
|
[0.5(
```

**Receive in Max:**
```
[udpreceive <port>]
|
[route /bespoke/notes]
|
[unpack f f]  (pitch, velocity)
```

### Processing Integration

**Send from Processing:**
```java
import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress bespoke;

void setup() {
  oscP5 = new OscP5(this, 12000);
  bespoke = new NetAddress("127.0.0.1", 8000);
}

void draw() {
  OscMessage msg = new OscMessage("/bespoke/control/synth~volume");
  msg.add(mouseX / (float)width);
  oscP5.send(msg, bespoke);
}
```

### Pure Data Integration

**Send from Pd:**
```
[pack f f]
|
[prepend send /bespoke/note]
|
[sendOSC]
```

**Receive in Pd:**
```
[dumpOSC 9000]
|
[route /bespoke/notes]
|
[unpack f f]
```

### Web Browser Control

**Using JavaScript:**
```javascript
// Requires OSC.js library
const osc = new OSC();
osc.open({ host: 'localhost', port: 8000 });

// Send message
osc.send(new OSC.Message('/bespoke/control/synth~volume', 0.5));
```

## Advanced Techniques

### Bidirectional Communication

**Setup two-way OSC:**
1. Create osccontroller
2. Set **in port** (e.g., 8000)
3. Set **out address** and **out port** (e.g., 192.168.1.100:9000)
4. BespokeSynth receives on 8000, sends to 9000

**Use case:**
- Receive control messages
- Send feedback/status updates
- Synchronize state with external app

### OSC Bundles

**Send multiple messages atomically:**
```
OSC Bundle [
  /bespoke/control/synth~volume 0.5
  /bespoke/control/filter~cutoff 1000
  /bespoke/control/delay~feedback 0.3
]
```

All parameters update simultaneously.

### Wildcard Patterns

**OSC supports wildcards:**
```
/bespoke/control/synth* 0.5  # Matches synth1, synth2, etc.
/bespoke/control/?ilter~cutoff 1000  # Matches filter, pilter, etc.
```

### Timestamped Messages

**OSC supports timestamps:**
```
Bundle with timestamp: 2024-01-15 10:30:00
  /bespoke/note 60 100
```

Messages execute at specified time.

### Pattern Matching

**Use patterns for flexible routing:**
```
/synth/[0-9]/volume  # Matches /synth/0/volume, /synth/1/volume, etc.
/effect/{reverb,delay}/mix  # Matches reverb or delay
```

## Network Configuration

### Local (Same Computer)

**Address:** `127.0.0.1` or `localhost`
**Port:** Any available port (8000-9000 common)

**Example:**
```
BespokeSynth: in port 8000
TouchOSC: send to 127.0.0.1:8000
```

### LAN (Local Network)

**Find computer's IP:**
- **macOS:** System Preferences → Network
- **Windows:** ipconfig in Command Prompt
- **Linux:** ifconfig or ip addr

**Example:**
```
Computer IP: 192.168.1.100
BespokeSynth: in port 8000
TouchOSC: send to 192.168.1.100:8000
```

### WiFi vs Ethernet

**WiFi:**
- Convenient
- Potential latency
- May drop packets

**Ethernet:**
- Lower latency
- More reliable
- Recommended for performance

### Firewall Configuration

**Allow OSC traffic:**
- **macOS:** System Preferences → Security → Firewall → Allow BespokeSynth
- **Windows:** Windows Defender → Allow app through firewall → Add BespokeSynth
- **Linux:** Configure iptables/ufw to allow UDP on OSC ports

## Troubleshooting

### Messages Not Received

**Check:**
- Correct IP address
- Correct port number
- Firewall allows traffic
- Both devices on same network
- OSC controller module exists and is enabled

**Test with OSC monitor:**
- Use OSC testing tools to verify messages
- Check message format is correct

### High Latency

**Solutions:**
- Use wired Ethernet instead of WiFi
- Reduce network traffic
- Use lower OSC message rate
- Optimize message size

### Messages Dropped

**Causes:**
- Network congestion
- Too many messages too fast
- UDP packet loss

**Solutions:**
- Reduce message rate
- Use OSC bundles for multiple parameters
- Switch to wired connection
- Implement message throttling

### Wrong Parameters Responding

**Check:**
- OSC address matches parameter path exactly
- Module name is correct
- Parameter name is correct
- Use `/bespoke/control/` prefix

## Best Practices

### Message Design

1. **Use clear addresses** - `/synth/volume` not `/s/v`
2. **Group related parameters** - `/synth/filter/cutoff`
3. **Use consistent naming** - Same convention throughout
4. **Document addresses** - Keep a list of all OSC addresses

### Performance

1. **Limit message rate** - Don't send faster than needed
2. **Bundle related messages** - Use OSC bundles
3. **Use appropriate data types** - Float for continuous, int for discrete
4. **Throttle high-frequency data** - Limit updates to 30-60 Hz

### Network

1. **Use static IPs** - Avoid DHCP changes
2. **Dedicated network** - Separate from internet traffic
3. **Quality router** - Low-latency, reliable
4. **Test before performing** - Verify all connections work

## Example Setups

### Tablet Control Surface

**Hardware:**
- iPad/Android tablet with TouchOSC
- Computer running BespokeSynth
- WiFi network

**Setup:**
1. Create osccontroller (port 8000)
2. Configure TouchOSC to send to computer IP:8000
3. Map faders/buttons to BespokeSynth parameters
4. Test all controls

### Visual Sync

**Hardware:**
- Computer running BespokeSynth
- Computer running visual software (Resolume, VDMX, etc.)
- Network connection

**Setup:**
1. BespokeSynth sends OSC to visual software
2. Visual software receives beat/tempo/parameter data
3. Visuals sync to music

**Example:**
```python
# Send beat pulse
def on_pulse():
    out = oscoutput.get("oscoutput")
    out.send_int("/beat", 1)
```

### Multi-Device Jam

**Hardware:**
- Multiple devices running BespokeSynth
- Network switch/router
- Ableton Link (optional)

**Setup:**
1. Each device has osccontroller and oscoutput
2. Devices send/receive OSC to each other
3. Synchronize parameters, notes, triggers
4. Collaborative performance

## Resources

### OSC Specification

- [OpenSoundControl.org](http://opensoundcontrol.org/)
- OSC 1.0 Specification
- OSC 1.1 Specification

### OSC Software

- **TouchOSC** - Mobile control surface
- **OSCulator** - OSC routing (macOS)
- **Max/MSP** - Visual programming
- **Pure Data** - Open-source visual programming
- **Processing** - Creative coding
- **SuperCollider** - Audio programming

### Testing Tools

- **OSC Monitor** - View OSC messages
- **OSC Data Monitor** - Debug OSC traffic
- **Protokol** - OSC/MIDI monitor (macOS)

## Next Steps

- **[MIDI Mapping](midi-mapping.md)** - Hardware controller integration
- **[Scripting](../scripting/README.md)** - Advanced OSC processing
- **[Performance Tips](performance-tips.md)** - Optimize for live use
