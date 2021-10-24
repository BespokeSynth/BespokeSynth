#adds digits in dozenal notation as variables
#this allows you to call me.play_note(d40, 127) to play the 0th note of the 4th octave

def dozenaldigit(decimal):
   if decimal == 11:
      return 'E'
   elif decimal == 10:
      return 'X'
   else:
      return str(decimal)

def dozenal(decimal):
   ret = dozenaldigit(decimal % 12)
   decimal //= 12
   while (decimal > 0):
      ret = dozenaldigit(decimal % 12) + ret
      decimal //= 12
   return ret

for i in range(128):
   globals()['d'+dozenal(i)] = i
