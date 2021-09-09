#schedule a sequence with a tidal-like pattern notation

def schedule_seq(seq, pitchOffset=0, velocity=80, start=0, bars=1, sustain=1, pan=0, output=None, velocityRandomization=0, panRandomization=0):
   if output is None:
      output = this.get_caller()
   if not type(seq) is list:
      seq = [seq]
   if len(seq) == 0:
      return
   stepLength = float(bars) / len(seq)
   for i,entry in enumerate(seq):
      if entry != '':
         if type(entry) is tuple:
            for subentry in entry:
               schedule_seq(subentry, pitchOffset, velocity, start + stepLength * i, stepLength, sustain, pan, output)
         elif type(entry) is list:
            schedule_seq(entry, pitchOffset, velocity, start + stepLength * i, stepLength, sustain, pan, output)
         elif type(entry) is int or type(entry) is str:
            pitch = entry
            if type(entry) is str:
               pitch = bespoke.name_to_pitch(entry)
            output.schedule_note(start + stepLength * i, pitch + pitchOffset, int(velocity * random.uniform(1-velocityRandomization,1)), stepLength*sustain, pan=pan * random.uniform(1-panRandomization,1))