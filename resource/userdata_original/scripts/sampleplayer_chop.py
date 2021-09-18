#requires a sampleplayer named "sampleplayer"

import sampleplayer

tempo = 99.5
secondsPerBeat = 60.0 / tempo
startOffsetSeconds = 3
startBeat = 0

sp = sampleplayer.get("sampleplayer")
for i in range(4):
   sp.set_cue_point(i, secondsPerBeat*(startBeat+i) + startOffsetSeconds, secondsPerBeat, 1)