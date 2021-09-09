#attach a pulser set to 16n

def on_pulse():
   step = bespoke.get_step(16)
   if step % 16 in [0,4,8,11]:
      this.play_note(0,127,1.0/16)
   elif step % 16 in [4,11,14]:
      this.play_note(1,127,1.0/16)
   else:
      this.play_note(2,127,1.0/16)
