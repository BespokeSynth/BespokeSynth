#attach a pulser set to 8n

lastPitch = -1

def on_pulse():
   global lastPitch
   root = bespoke.get_root() + 48
   scale = bespoke.get_scale()
   while (True):
      pitch = root + random.choice(scale) + random.choice([0,12])
      if pitch != lastPitch:
         break
   velocity = get_velocity()
   if abs(pitch - lastPitch) == 2:
      this.schedule_note(1.0/16, (pitch + lastPitch) / 2, int(velocity * .8), 1.0/17)
   this.schedule_note(1.0/8, pitch, velocity, 1.0/9)
   #if pitch % 12 == root % 12:
   #   this.schedule_note(1.0/8, pitch + 7, 70, 1.0/4)
   lastPitch = pitch
   
def get_velocity():
   velocities = [127,80,100]
   ret = velocities[(bespoke.get_step(8) + 1) % len(velocities)]
   this.output(str(ret))
   return ret