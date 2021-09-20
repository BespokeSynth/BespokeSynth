#requires a drumplayer called "drumplayer"

def on_pulse():
   for i in range(8):
      if random.random() < .5:
         this.set("drumplayer~random "+str(i), 1)
      
on_pulse()