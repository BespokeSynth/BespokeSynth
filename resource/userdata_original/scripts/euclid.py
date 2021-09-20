#attach a pulser set to 16n

def on_pulse():
   step = bespoke.get_step(16)
   if (euclid(step,2,16)):
      me.play_note(0,127)
   if (euclid(step+4,3,16)):
      me.play_note(5,80)
   if (euclid(step,5,8)):
      me.play_note(2 if euclid(step,3,11) else 6,127)
      
def euclid(step, count, length):
   return math.floor(step*count/length) != math.floor((step-1)*count/length)