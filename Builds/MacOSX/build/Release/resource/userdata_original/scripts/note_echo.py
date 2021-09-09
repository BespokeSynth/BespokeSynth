#send a note input into here

def on_note(pitch,velocity):
   for i in range(1,5):
      me.schedule_note_msg(i/8.0, pitch, int(velocity/i))