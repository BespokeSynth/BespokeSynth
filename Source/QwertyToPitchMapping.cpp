#include "QwertyToPitchMapping.h"
#include "UserPrefs.h"

/*Returns the pitch of a computer key char, according to the user selected layout.
 * May return -1 pitch for no matching key.
 */
QwertyToPitchResponse QwertyToPitchMapping::GetPitchForComputerKey(int key)
{
   QwertyToPitchMappingMode mode = static_cast<QwertyToPitchMappingMode>(UserPrefs.qwerty_to_pitch_mode.GetIndex());
   int pitch = -1;
   int octaveShift = 0;

   switch (mode)
   {
      case QwertyToPitchMappingMode::Ableton:
         switch (key)
         {
            case 'a':
               pitch = 0;
               break;
            case 'w':
               pitch = 1;
               break;
            case 's':
               pitch = 2;
               break;
            case 'e':
               pitch = 3;
               break;
            case 'd':
               pitch = 4;
               break;
            case 'f':
               pitch = 5;
               break;
            case 't':
               pitch = 6;
               break;
            case 'g':
               pitch = 7;
               break;
            case 'y':
               pitch = 8;
               break;
            case 'h':
               pitch = 9;
               break;
            case 'u':
               pitch = 10;
               break;
            case 'j':
               pitch = 11;
               break;
            case 'k':
               pitch = 12;
               break;
            case 'o':
               pitch = 13;
               break;
            case 'l':
               pitch = 14;
               break;
            case 'p':
               pitch = 15;
               break;
            case ';':
               pitch = 16;
               break;
            case 'z':
               octaveShift = -1;
               break;
            case 'x':
               octaveShift = 1;
               break;
            default:
               pitch = -1;
               break;
         }
         break;
      case QwertyToPitchMappingMode::Fruity:
         switch (key)
         {
            //Oct #1
            case 'z':
               pitch = 0;
               break;
            case 's':
               pitch = 1;
               break;
            case 'x':
               pitch = 2;
               break;
            case 'd':
               pitch = 3;
               break;
            case 'c':
               pitch = 4;
               break;
            case 'v':
               pitch = 5;
               break;
            case 'g':
               pitch = 6;
               break;
            case 'b':
               pitch = 7;
               break;
            case 'h':
               pitch = 8;
               break;
            case 'n':
               pitch = 9;
               break;
            case 'j':
               pitch = 10;
               break;
            case 'm':
               pitch = 11;
               break;
            //Oct #2
            case 'q':
               pitch = 12;
               break;
            case '2':
               pitch = 13;
               break;
            case 'w':
               pitch = 14;
               break;
            case '3':
               pitch = 15;
               break;
            case 'e':
               pitch = 16;
               break;
            case 'r':
               pitch = 17;
               break;
            case '5':
               pitch = 18;
               break;
            case 't':
               pitch = 19;
               break;
            case '6':
               pitch = 20;
               break;
            case 'y':
               pitch = 21;
               break;
            case '7':
               pitch = 22;
               break;
            case 'u':
               pitch = 23;
               break;
            case 'i':
               pitch = 24;
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
               octaveShift = -1;
               break;
            case '.':
               octaveShift = 1;
               break;
            default:
               pitch = -1;
               break;
         }
         break;
   }

   return { pitch, octaveShift };
}
