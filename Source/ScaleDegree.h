//
//  ScaleDegree.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/5/16.
//
//

#ifndef __Bespoke__ScaleDegree__
#define __Bespoke__ScaleDegree__

#include "NoteEffectBase.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "DropdownList.h"

class ScaleDegree : public NoteEffectBase, public IDrawableModule, public IDropdownListener
{
public:
   ScaleDegree();
   static IDrawableModule* Create() { return new ScaleDegree(); }
   
   string GetTitleLabel() override { return "scale degree"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   struct NoteInfo
   {
      int mPitch;
      int mVelocity;
      int mVoiceIdx;
   };
   
   int TransformPitch(int pitch);
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = 110; height = 22; }
   bool Enabled() const override { return mEnabled; }
   
   int mScaleDegree;
   DropdownList* mScaleDegreeSelector;
   std::list<NoteInfo> mInputNotes;
   ofMutex mNotesMutex;
};

#endif /* defined(__Bespoke__ScaleDegree__) */
