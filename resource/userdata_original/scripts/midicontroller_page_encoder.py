# Maps a MIDI encoder to the midicontroller page parameter, clamping values above 31.
import midicontroller

CONTROLLER = "midicontroller"
ENCODER_CC = 85
MAX_VALUE = 127

mc = midicontroller.get(CONTROLLER)
mc.add_script_listener(me.me())

def on_midi(messageType, control, value, channel):
   if control != ENCODER_CC:
      return
   value = round(value * 127)
   if value <= 31:
      me.set(f"{CONTROLLER}~page", value)
