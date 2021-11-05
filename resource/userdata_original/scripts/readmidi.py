# Path to your midi file
midifile='/home/asmw/src/BespokeSynth/overlap.mid'

# If you want to write the notes to a canvas, set its name here
canvas = ''

from mido import MidiFile
import time

mid = MidiFile(midifile)

bespoke_notes = []

class BespokeNote(object):
    def __init__(self, pitch, velocity, on_time, off_time):
        self.pitch = pitch
        self.velocity = velocity
        self.on_time = on_time
        self.off_time = off_time
        print(self.__str__())

    def delay(self):
        return self.on_time

    def length(self):
        return self.off_time - self.on_time
    
    def __str__(self):
        return f'@{self.delay()}->{self.length()} [{self.pitch}/{self.velocity}]'

timecount = 0.0

on_notes = {}

for msg in mid:
    print(f'{timecount}: {msg}')
    timecount += msg.time
    if msg.type == 'note_on':
        on_notes[msg.note] = timecount
    elif msg.type == 'note_off':
        if msg.note in on_notes:
            bespoke_notes.append(BespokeNote(msg.note, msg.velocity, on_notes[msg.note], timecount))
            del(on_notes[msg.note])

if canvas == '':
    for bn in bespoke_notes:
        try:
            me.schedule_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
        except:
            print(bn)
else:
    import notecanvas
    n = notecanvas.get(canvas)
    n.clear()
    for bn in bespoke_notes:
        n.add_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
       