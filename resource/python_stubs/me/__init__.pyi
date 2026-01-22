from __future__ import annotations

   def play_note(pitch, velocity, length=1.0/16.0, pan = 0, output_index = 0):
      pass

   def schedule_note(delay, pitch, velocity, length=1.0/16.0, pan = 0, output_index = 0):
      pass

   def schedule_note_msg(delay, pitch, velocity, pan = 0, output_index = 0):
      pass

   def schedule_call(delay, method):
      pass

   def note_msg(pitch, velocity, pan = 0, output_index = 0):
      pass

   def set(path, value):
      pass

   def schedule_set(delay, path, value):
      pass

   def get(path):
      pass

   def get_text(path):
      pass

   def set_text(path, text):
      pass

   def adjust(path, amount):
      pass

   def highlight_line(lineNum, scriptModuleIndex):
      pass

   def output(obj):
      pass

   def me():
      pass

   def stop():
      pass

   def get_caller():
      pass

   def set_num_note_outputs(num):
      pass

   def connect_osc_input(port):
      pass

   def send_cc(control, value, output_index = 0):
      pass

