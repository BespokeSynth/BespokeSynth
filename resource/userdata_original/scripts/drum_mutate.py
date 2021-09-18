#requires a drumsequencer named "drumsequencer"

import drumsequencer

seq = drumsequencer.get("drumsequencer")

def mutate():
   for step in range(16):
      for pitch in range(8):
         if random.random() < 0.1 and (pitch != 0 or step % 8 != 0):
            if random.random() < .15:
               seq.set(step, pitch, random.choice([80,127]))
            else:
               seq.set(step, pitch, 0)

def on_pulse():
   mutate()
   
on_pulse()