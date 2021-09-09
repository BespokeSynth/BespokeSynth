#requires a notecanvas named "notecanvas"

import notecanvas

n = notecanvas.get("notecanvas")
scale = bespoke.get_scale_range(3,10)[::2]

n.clear()
t = 0.0
subdivision = 16
canvasLength = n.get("measures")

if canvasLength > 0:
   while t < canvasLength:
      length = 1/subdivision * random.randint(1,3)
      if t+length > canvasLength:
         length = canvasLength-t
      skip = False
      if length == 1/subdivision:
         skip = random.random() < .25
      if not skip:
         n.add_note(t, random.choice(scale), 127, length)
      t += length
