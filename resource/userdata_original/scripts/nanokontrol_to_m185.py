#requires a midicontroller named "midicontroller"
#requires a m185sequencer named "m185sequencer"
#the controls are mapped to the default controls of the nanoKONTROL2 device

import midicontroller

m = midicontroller.get("midicontroller")

for i in range(8):
   m.set_connection(m.Control, i + 0, "m185sequencer~pitch"+str(i))
   m.set_connection(m.Control, i +16, "m185sequencer~pulses"+str(i))
   m.set_connection(m.Control, i +32, "m185sequencer~gate"+str(i), m.SetValue, 0)
   m.set_connection(m.Control, i +48, "m185sequencer~gate"+str(i), m.SetValue, 2)
   m.set_connection(m.Control, i +64, "m185sequencer~gate"+str(i), m.SetValue, 3)