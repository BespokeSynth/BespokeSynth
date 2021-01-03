from __future__ import annotations

   def play_note(this, pitch, velocity, length=1.0/16.0, pan = 0, output_index = 0):
      pass

   def schedule_note(this, delay, pitch, velocity, length=1.0/16.0, pan = 0, output_index = 0):
      pass

   def schedule_note_msg(this, delay, pitch, velocity, pan = 0, output_index = 0):
      pass

   def schedule_call(this, delay, method):
      pass

   def note_msg(this, pitch, velocity, pan = 0, output_index = 0):
      pass

   def set(this, path, value):
      pass

   def schedule_set(this, delay, path, value):
      pass

   def get(this, path):
      pass

   def adjust(this, path, amount):
      pass

   def highlight_line(this, lineNum, scriptModuleIndex):
      pass

   def output(this, obj):
      pass

   def me(this, ):
      pass

   def stop(this, ):
      pass

   def get_caller(this, ):
      pass

   def set_num_note_outputs(this, num):
      pass

