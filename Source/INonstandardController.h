//
//  INonstandardController.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/14.
//
//

#ifndef Bespoke_INonstandardController_h
#define Bespoke_INonstandardController_h

class ofxJSONElement;

class INonstandardController
{
public:
   virtual ~INonstandardController() {}
   virtual void SendValue(int page, int control, float value, bool forceNoteOn = false, int channel = -1) = 0;
   virtual void LoadInfo(const ofxJSONElement& moduleInfo) {}
   virtual bool IsInputConnected() { return true; }
   virtual bool Reconnect() { return true; }
};

#endif
