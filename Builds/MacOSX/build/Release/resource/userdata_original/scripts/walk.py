#attach a pulser set to 8n

scale = bespoke.get_scale_range(3,7)
random.shuffle(scale)

def on_pulse():
   step = bespoke.get_step(8)
   this.play_note(walk(scale,step), walk([110,30,60,80],step), 1.0/8)
   
def walk(list, step):
   return list[step % len(list)]