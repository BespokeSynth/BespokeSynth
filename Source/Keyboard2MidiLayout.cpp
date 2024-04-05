#include "Keyboard2MidiLayout.h"


#include "UserPrefs.h"

/*Returns the pitch of a computer key char, according to the user selected layout.
 * May return -1 pitch for no matching key or the 'ignored' layout. 
 */ 
Keyboard2MidiResponse Keyboard2MidiLayout::GetPitchForComputerKey(int key, int octave)
{
   Keyboard2MidiLayoutType ly = static_cast<Keyboard2MidiLayoutType>(UserPrefs.keyboard_2_midi_layout.GetIndex());
   int idx = -1;
   
   switch (ly)
   {
      case Keyboard2MidiLayoutType::Ableton:
         switch (key)
         {
            case 'a':
               idx = 0;
               break;
            case 'w':
               idx = 1;
            break;
            case 's':
               idx = 2;
            break;
            case 'e':
               idx = 3;
            break;
            case 'd':
               idx = 4;
            break;
            case 'f':
               idx = 5;
            break;
            case 't':
               idx = 6;
            break;
            case 'g':
               idx = 7;
            break;
            case 'y':
               idx = 8;
            break;
            case 'h':
               idx = 9;
            break;
            case 'u':
               idx = 10;
            break;
            case 'j':
               idx = 11;
            break;
            case 'k':
               idx = 12;
            break;
            case 'o':
               idx = 13;
            break;
            case 'l':
               idx = 14;
            break;
            case 'p':
               idx = 15;
            break;
            case ';':
               idx = 16;
            break;
            case 'z':
               octave--;
            break;
            case 'x':
               octave++;
               break;
            default: idx = -1;
               break;
         }
         break;
      case Keyboard2MidiLayoutType::Fruity:
         switch (key)
         {
            //Oct #1
            case 'z':
               idx = 0;
            break;
            case 's':
               idx = 1;
            break;
            case 'x':
               idx = 2;
            break;
            case 'd':
               idx = 3;
            break;
            case 'c':
               idx = 4;
            break;
            case 'v':
               idx = 5;
            break;
            case 'g':
               idx = 6;
            break;
            case 'b':
               idx = 7;
            break;
            case 'h':
               idx = 8;
            break;
            case 'n':
               idx = 9;
            break;
            case 'j':
               idx = 10;
            break;
            case 'm':
               idx = 11;
            break;
            //Oct #2
            case 'q':
               idx = 12;
            break;
            case '2':
               idx = 13;
            break;
            case 'w':
               idx = 14;
            break;
            case '3':
               idx = 15;
            break;
            case 'e':
               idx = 16;
            break;
            case 'r':
               idx = 17;
            break;
            case '5':
               idx = 18;
            break;
            case 't':
               idx = 19;
            break;
            case '6':
               idx = 20;
            break;
            case 'y':
               idx = 21;
            break;
            case '7':
               idx = 22;
            break;
            case 'u':
               idx = 23;
            break;
            case 'i':
               idx = 24;
            break; //Trimmed so the layout fully supports 2 octaves +1
            /*
            case '9':
               idx = 25;
            break;
            case 'o':
               idx = 26;
            break;
            case '0':
               idx = 27;
            break;
            case 'p':
               idx = 28;
            break;*/
            //Oct Control
            case ',':
               octave--;
            break;
            case '.':
               octave++;
            break;
            default: idx = -1;
            break;
         }
         break;
      case Keyboard2MidiLayoutType::Ignore:
         return {idx,octave};
   }

   if (octave>9)
      octave = 9;
   if (octave<0)
      octave = 0;
   
   if (idx != -1)
      return {octave * 12 + idx,octave};
   
   return {idx,octave};
}

