from __future__ import annotations

def get(path) -> midicontroller:
   pass

class midicontroller:
   Note: ...
   Control: ...
   Program: ...
   PitchBend: ...
   Slider: ...
   SetValue: ...
   Toggle: ...
   Direct: ...
   SetValueOnRelease: ...
   Default: ...
   def set_connection(this, messageType, control, controlPath, controlType = Default, value = 0, channel = -1, page=0, midi_off=0, midi_on = 127, scale = false, blink = false, increment = 0, twoway = true, feedbackControl = -1, isPageless = false):
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

