def echo(pitch, velocity, repeats, length=1.0/16, loopLength=1, syncopateChance=0, pan=0, playChance=1, responseIndex=0, playFirstLoop=True, quantize=False):
   output = this.get_caller()
   for i in range(repeats):
      if i == 0 and playFirstLoop == False:
         continue
      if random.random() < playChance:
         quantizeShift = 0
         if quantize:
            quantizeSubdivision = 16.0
            quantizeShift = (round(bespoke.get_measure_time() * quantizeSubdivision) / quantizeSubdivision) - bespoke.get_measure_time()
         pos = i*loopLength + quantizeShift
         noteLength = length
         outputIndex = responseIndex
         if i == 0:
            outputIndex = 0
         canSyncopate = i > 0 and bespoke.get_step(32) % 4 == 0 or bespoke.get_step(32) % 4 == 3
         if canSyncopate and random.random() < syncopateChance:
            pos -= 1.0/16
            #if random.random() < .5:
            #   noteLength += 1.0/16
         output.schedule_note(pos, pitch, velocity, noteLength, pan=pan, output_index=outputIndex)
