#requires a sampleplayer named "sampleplayer"
#requires a drumplayer named "drumplayer"

import sampleplayer
import drumplayer

s = sampleplayer.get("sampleplayer")
d = drumplayer.get("drumplayer")

for i in range(8):
   s.set_cue_point(i, random.uniform(0,4),.1,1)
   d.import_sampleplayer_cue(s, i, i)