# Path to your midi file
midifile='/home/asmw/src/BespokeSynth/multitrack.mid'

# If true, every selected track will be rendered into its own canvas
create_canvas = True

# Auto link the created canvases to the target of this script
auto_link = True

# Which tracks to extract/play, empty list means all
tracks = []

# whether to start playing all notes on script load
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

def parse_midi(midifile, tracks = []):
    mid = MidiFile(midifile)

    if not tracks:
        tracks = range(len(mid.tracks))

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

def schedule(notes):
    output = 0
    for track in notes:
        for bn in notes[track]:
            try:
                me.schedule_note(bn.delay(), bn.pitch, bn.velocity, bn.length(), 0, output)
            except:
                print(f'{output}: {bn}')
        output += 1

def render(notes, canvas):
    for bn in notes:
        canvas.add_note(bn.delay(), bn.pitch, bn.velocity, bn.length())
    canvas.fit()

def create_canvases(tracks):
    import module
    import notecanvas
    y = 260
    increment = 400
    for t in tracks:
        if len(tracks[t]) > 0:
            print(f"Creating canvas for track {t}")
            c = module.create('notecanvas', 10, y)
            name = f'MIDI Track {t}'
            c.set_name(name)
            # TODO: calling add_note on the returned module does not work, why?
            render(track_notes[t], c)
            if auto_link:
                target = module.get(me.get_target())
                if target:
                    c.set_target(target)
            y += increment
        else:
            print(f"Skipping empty track {t}")

track_notes = parse_midi(midifile, tracks)

if not tracks:
    tracks = track_notes.keys()

if create_canvas:
    print("Creating canvases")
    create_canvases(track_notes)

if play_on_load:
    schedule(track_notes)

def on_pulse():
    schedule(track_notes)