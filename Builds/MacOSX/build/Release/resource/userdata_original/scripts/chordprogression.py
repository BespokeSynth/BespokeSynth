#attach a pulser set to 16n

scale = bespoke.get_scale_range(4,50)
chord = [0,2,4]
progression = [0,2,0,1]
steps = [0,4,8,12]
if not 'idx' in globals():
   idx = 0

def on_pulse():
   global idx
   if bespoke.get_step(16) % 16 in steps:
      if bespoke.get_step(16) % 16 == 0:
         idx = 0
      first = True
      for tone in chord:
         this.play_note(scale[tone+progression[idx]]+random.choice([0]),127,1.0/4)
      idx = (idx + 1) % len(progression)

