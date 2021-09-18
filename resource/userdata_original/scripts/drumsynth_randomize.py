#requires a drumsynth named "drumsynth"

for i in range(8):
   if random.random() < .5:
      me.set("drumsynth~vol"+str(i), random.uniform(0,1))
      me.set("drumsynth~noise"+str(i), random.uniform(0,1))
      me.set("drumsynth~adsrtone"+str(i)+"A",random.uniform(1,10))
      me.set("drumsynth~adsrtone"+str(i)+"D",random.uniform(10,150))
      me.set("drumsynth~adsrnoise"+str(i)+"A",random.uniform(1,10))
      me.set("drumsynth~adsrnoise"+str(i)+"D",random.uniform(10,250))
      me.set("drumsynth~adsrfreq"+str(i)+"A",random.uniform(1,10))
      me.set("drumsynth~adsrfreq"+str(i)+"D",random.uniform(3,120))
      me.set("drumsynth~adsrfilter"+str(i)+"A",random.uniform(1,10))
      me.set("drumsynth~adsrfilter"+str(i)+"D",random.uniform(10,250))
      me.set("drumsynth~freqmax"+str(i),random.uniform(100,1000))
      me.set("drumsynth~freqmin"+str(i),random.uniform(10,100))
      me.set("drumsynth~cutoffmax"+str(i),random.uniform(100,10000))
      me.set("drumsynth~cutoffmin"+str(i),random.uniform(10,200))
      me.set("drumsynth~q"+str(i),random.uniform(.1,8))
      me.set("drumsynth~type"+str(i),random.choice([0,1,2,3]))