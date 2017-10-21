//
//  LaunchpadInterpreter.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/24/12.
//
//

#ifndef __modularSynth__LaunchpadInterpreter__
#define __modularSynth__LaunchpadInterpreter__

#include "OpenFrameworksPort.h"

class MidiController;
struct MidiNote;
struct MidiControl;

struct LightUpdate
{
   LightUpdate(int x, int y, char r, char g, float intensity=1) : mX(x), mY(y), mR(r), mG(g), mIntensity(intensity) {}
   int mX;
   int mY;
   char mR;
   char mG;
   float mIntensity;
};

class ILaunchpadListener
{
public:
   virtual ~ILaunchpadListener() {}
   virtual void OnButtonPress(int x, int y, bool bOn) = 0;
};

class LaunchpadInterpreter
{
public:
   LaunchpadInterpreter(ILaunchpadListener* listener);
   void SetController(MidiController* controller, int controllerPage);
   void OnMidiNote(MidiNote& note);
   void OnMidiControl(MidiControl& control);
   void UpdateLights(vector<LightUpdate> lightUpdates, bool force = false);
   void Draw(ofVec2f vPos);
   void ResetLaunchpad();
   bool HasLaunchpad() { return mController != nullptr; }
   
   static int LaunchpadColor(int r, int g);
   
private:
   void ClearStoredLights();
   bool IsMonome() const;
   
   ILaunchpadListener* mListener;
   int mLights[64+8+8]; //grid + side + top
   MidiController* mController;
   int mControllerPage;
};

#endif /* defined(__modularSynth__LaunchpadInterpreter__) */
