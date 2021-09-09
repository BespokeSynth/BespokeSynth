#a collection of tools for building named chords

progression = [('D2', 'M', 1, 1),
               ('F2', 'M', 1, 1),
               ('G2', 'min7', 0, 1),
               ('F#2', 'M7', 0, 1),]
               
qualities = {}
qualities[''] = [0]
qualities['M'] = [0, 4, 7]
qualities['m'] = [0, 3, 7]
qualities['dim'] = [0, 3, 6]
qualities['aug'] = [0, 4, 8]
qualities['M7'] = [0, 4, 7, 11]
qualities['min7'] = [0, 3, 7, 10]
qualities['D7'] = [0, 4, 7, 10]
qualities['mM7'] = [0, 3, 7, 11]
qualities['dim7'] = [0, 3, 6, 10]
 
def get_chord(root, quality, inversion):
   ret = []
   for i in range(len(qualities[quality])):
      pitch = bespoke.name_to_pitch(root) + qualities[quality][i]
      if i < inversion:
         pitch += 12
      ret.append(pitch)
   return ret

def add_chord(canvas, root, quality, inversion, position, length):
   for pitch in get_chord(root, quality, inversion):
      canvas.add_note(position, pitch, 127, length)
      
def play_chord(root, quality, inversion, length):
   for pitch in get_chord(root, quality, inversion):
      this.play_note(pitch, 127, length)

