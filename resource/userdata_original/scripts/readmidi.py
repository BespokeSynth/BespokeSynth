# Path to your midi file
midifile='/home/asmw/src/BespokeSynth/multitrack.mid'

# If you want to write the notes to a canvas, set its name here
canvas = ''

# Which tracks to extract/play
tracks = [1,2]

# whether to play on script load
play_on_load = False

from mido import MidiFile, tick2second, bpm2tempo
import time

tempo = bpm2tempo(120)

try:
    tempo = bpm2tempo(bespoke.get_tempo())
except:
    pass

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

def parse_midi(midifile, tracks = [1]):
    mid = MidiFile(midifile)
    bespoke_notes = {}

    for track in tracks:
        if track > len(mid.tracks):
            continue

        bespoke_notes[track] = []
        t = mid.tracks[track - 1]
        timecount = 0.0
        on_notes = {}

        for msg in t:
            print(f'{timecount}: {msg}')
            timecount += tick2second(msg.time, mid.ticks_per_beat, tempo)
            if msg.type == 'note_on':
                on_notes[msg.note] = timecount
            elif msg.type == 'note_off':
                if msg.note in on_notes:
                    bespoke_notes[track].append(BespokeNote(msg.note, msg.velocity, on_notes[msg.note], timecount))
                    del(on_notes[msg.note])
    return bespoke_notes

nc = None
if canvas != '':
    try:
        import notecanvas
        nc = notecanvas.get(canvas)
        nc.clear()
    except:
        pass

def play(notes, canvas = None):
    output = 0
    for track in notes:
        for bn in notes[track]:
            if canvas is None:
                try:
                    me.schedule_note(bn.delay(), bn.pitch, bn.velocity, bn.length(), 0, output)
                except:
                    print(f'{output}: {bn}')
            else:
                canvas.add_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
        output += 1

notes = parse_midi(midifile, tracks)

if play_on_load:
    play(notes)

def on_pulse():
    play(notes)