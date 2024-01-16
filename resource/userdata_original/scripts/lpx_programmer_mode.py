#this script puts the Launchpad X into "programmer mode"

import midicontroller

m = midicontroller.get("midicontroller")

m.send_sysex(bytes([0, 32, 41, 2, 12, 14, 1]))
m.resync_controller_state()