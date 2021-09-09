#attach a pulser set to 1n
#requires the "schedule_sequence.py" script to be loaded

def on_pulse():
   pattern = [0,0,0,'',0,0,'',0,'',0,0,'']
   loop = (bespoke.get_measure()) % len(pattern)
   loop2 = (bespoke.get_measure() // 2) % len(pattern)
   loop3 = (bespoke.get_measure() // 3) % len(pattern)

   shifted = pattern[loop:] + pattern[:loop]
   shifted2 = pattern[loop2:] + pattern[:loop2]
   shifted3 = pattern[loop3:] + pattern[:loop3]
   
   schedule_seq(pattern, pan=-1, pitchOffset=4)
   schedule_seq(shifted, pan=1, pitchOffset=5)
   schedule_seq(shifted2, pan=-.5, pitchOffset=6)
   schedule_seq(shifted3, pan=.5, pitchOffset=7)
   schedule_seq([ 0, 0,'', 0])
   schedule_seq(['','', 1,''])
   
   #debug output
   this.output(loop)
   this.output(pattern)
   this.output(shifted)

