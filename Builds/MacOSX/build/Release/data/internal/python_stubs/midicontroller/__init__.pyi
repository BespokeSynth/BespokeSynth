from __future__ import annotations

def get(path) -> midicontroller:
   pass

class midicontroller:
   def add_connection(messageType, control, channel, controlPath):
      pass

   def send_note(pitch, velocity, forceNoteOn = false, channel = -1, page = 0):
      pass

   def send_cc(ctl, value, channel = -1, page = 0):
      pass

   def send_pitchbend(bend, channel = -1, page = 0):
      pass

   def send_data(a, b, c, page = 0):
      pass

