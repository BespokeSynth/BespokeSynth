#requires a notecanvas named "notecanvas"

import notecanvas

notes = [('C4', .125),
         ('A3', .125),
         ('G3', .125),
         ('A3', .125),
         ('F3', .125), 
         ('G3', .125),
         ('C4', .25)]

canvas = notecanvas.get("notecanvas")
canvas.clear()

pos = 0
for i in range(len(notes)):
   canvas.add_note(pos,bespoke.name_to_pitch(notes[i][0]),127,notes[i][1])
   pos = pos + notes[i][1]
