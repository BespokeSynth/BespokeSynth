//
//  Monome.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/22/14.
//
//

#ifndef __modularSynth__Monome__
#define __modularSynth__Monome__

#include "../JuceLibraryCode/JuceHeader.h"
#include "OpenFrameworksPort.h"
#include "MidiDevice.h"
#include "INonstandardController.h"
#include "Oscillator.h"

#define HOST "127.0.0.1"
#define SERIAL_OSC_PORT 12002
#define MONOME_RECEIVE_PORT 13338

#define NUM_MONOME_BUTTONS 64

class Monome : public INonstandardController,
               private OSCReceiver,
               private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:
   Monome(MidiDeviceListener* listener);
   ~Monome();
   
   void Connect();
   void SetLight(int x, int y, bool on);
   void SetLightFlicker(int x, int y, float flickerMs);
   
   void oscMessageReceived(const OSCMessage& msg) override;
   
   void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1)override;
   
   bool IsInputConnected() override { return mHasMonome; }
   bool Reconnect() override { Connect(); return mHasMonome; }

private:
   void SetLightInternal(int x, int y, bool on);
   
   OSCSender mToSerialOsc;
   OSCSender mToMonome;
   bool mHasMonome;
   int mMaxColumns;
   
   MidiDeviceListener* mListener;
};

#endif /* defined(__modularSynth__Monome__) */
