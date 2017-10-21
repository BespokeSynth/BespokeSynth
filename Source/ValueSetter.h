//
//  ValueSetter.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/1/16.
//
//

#ifndef __Bespoke__ValueSetter__
#define __Bespoke__ValueSetter__

#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "TextEntry.h"

class PatchCableSource;
class IUIControl;

class ValueSetter : public IDrawableModule, public INoteReceiver, public ITextEntryListener
{
public:
   ValueSetter();
   virtual ~ValueSetter();
   static IDrawableModule* Create() { return new ValueSetter(); }
   
   string GetTitleLabel() override { return "value setter"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationChain* pitchBend = nullptr, ModulationChain* modWheel = nullptr, ModulationChain* pressure = nullptr) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource) override;
   
   void TextEntryComplete(TextEntry* entry) override {}
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(int& width, int& height) override { width = 110; height = 20; }
   bool Enabled() const override { return mEnabled; }
   
   PatchCableSource* mControlCable;
   IUIControl* mTarget;
   float mValue;
   TextEntry* mValueEntry;
};

#endif /* defined(__Bespoke__ValueSetter__) */
