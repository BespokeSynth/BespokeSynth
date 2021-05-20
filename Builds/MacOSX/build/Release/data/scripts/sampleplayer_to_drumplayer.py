import sampleplayer
import drumplayer

s = sampleplayer.get("sampleplayer")
d = drumplayer.get("drumplayer")

for i in range(8):
   s.set_cue_point(i, random.uniform(7,20),.1,1)
   d.import_sampleplayer_cue(s, i, i)