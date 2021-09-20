#attach a pulser set to 16n, and some keyboard input

sequence = [60,60,60,60]
insertPos = 0

def on_pulse():
   this.play_note(sequence[bespoke.get_step(16)%len(sequence)],127,1.0/16)
   
def on_note(pitch, velocity):
   global sequence
   global insertPos
   if velocity > 0:
      sequence[insertPos % len(sequence)] = pitch
      insertPos += 1