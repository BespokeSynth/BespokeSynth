#requires a midicontroller named "midicontroller"

import midicontroller

controller = midicontroller.get("midicontroller")
controller.add_script_listener(me.me())

def on_midi(messageType, control, value, channel):
   debug = str(messageType) + " " + str(control) + " " + str(value) + " " + str(channel)
   me.output(debug)