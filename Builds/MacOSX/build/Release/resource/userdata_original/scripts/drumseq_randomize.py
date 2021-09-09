#requires a drumsequencer named "drumsequencer"

import drumsequencer

d = drumsequencer.get("drumsequencer")

def randomVel():
   return random.choice([0,0,random.randint(1,127)])   

for row in range(8):
   for step in range(16):
      d.set(step, row, randomVel()) 
   for step in range(16):
      vel = d.get(step, row)
      if random.random() < .25:
         vel = randomVel()
      d.set(step+16, row, vel)

for step in range(32):
   if step % 4 == 0:
      d.set(step, random.choice([0,4]), 127)