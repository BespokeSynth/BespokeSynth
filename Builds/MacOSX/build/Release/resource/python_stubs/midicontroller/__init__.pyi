from __future__ import annotations

def get(path) -> midicontroller:
   pass

class midicontroller:
   def set_connection(this, messageType, control, channel, controlPath):
      pass

   def send_note(this, pitch, velocity, forceNoteOn = false, channel = -1, page = 0):
      pass

   def send_cc(this, ctl, value, channel = -1, page = 0):
      pass

   def send_program_change(this, program, channel = -1, page = 0):
      pass

   def send_pitchbend(this, bend, channel = -1, page = 0):
      pass

   def send_data(this, a, b, c, page = 0):
      pass

   def add_script_listener(this, script):
      pass

