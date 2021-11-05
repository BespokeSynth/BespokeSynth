# Path to your midi file
midifile='/home/asmw/src/BespokeSynth/test.mid'

# If you want to write the notes to a canvas, set its name here
canvas = ''

from mido import MidiFile
import time

mid = MidiFile(midifile)

bespoke_notes = []

class BespokeNote(object):
    def __init__(self, pitch, velocity, on_time, off_time):
        print(on_time, off_time)
        self.pitch = pitch
        self.velocity = velocity
        self.on_time = on_time
        self.off_time = off_time

    def delay(self):
        return self.on_time

    def length(self):
        return self.off_time - self.on_time
    
    def __str__(self):
        return f'{self.delay()}/{self.pitch}/{self.velocity}/{self.length()}'

timecount = 0.0

for msg in mid:
    if msg.type == 'note_on':
        last_start = timecount
    elif msg.type == 'note_off':
        bespoke_notes.append(BespokeNote(msg.note, msg.velocity, last_start, last_start + msg.time))
    timecount += msg.time

if canvas == '':
    for bn in bespoke_notes:
        me.schedule_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
else:
    import notecanvas
    n = notecanvas.get(canvas)
    n.clear()
    for bn in bespoke_notes:
        n.add_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
       