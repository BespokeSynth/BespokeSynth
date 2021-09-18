#attach a pulser set to 4n

root = bespoke.get_root() + 48
scale = bespoke.get_scale()
chord = [scale[0] + root, scale[2] + root, scale[4] + root]

step = 0

def on_pulse():
   global step
   for p in chord:
      this.play_note(p, walk([60,30,10],step), 1.0/4)
   while True:
      newPitch = scale[random.randrange(len(scale))] + root + random.choice([-12,0,12])
      if not newPitch in chord:
         chord[random.randrange(len(chord))] = newPitch
         break
   step = step + 1
   
def walk(list, step):
   return list[step % len(list)]