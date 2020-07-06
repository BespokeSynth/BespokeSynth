//
//  NoteChainNode.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/1/16.
//
//

#ifndef __Bespoke__NoteChainNode__
#define __Bespoke__NoteChainNode__

#include "IDrawableModule.h"
#include "INoteSource.h"
#include "ClickButton.h"
#include "TextEntry.h"
#include "Slider.h"
#include "Transport.h"
#include "DropdownList.h"

class NoteChainNode : public IDrawableModule, public INoteSource, public IButtonListener, public ITextEntryListener, public IFloatSliderListener, public IAudioPoller, public IDropdownListener, public ITimeListener
{
public:
   NoteChainNode();
   virtual ~NoteChainNode();
   static IDrawableModule* Create() { return new NoteChainNode(); }
   
   string GetTitleLabel() override { return "note chain"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void TriggerNote();
   
   void OnTimeEvent(double time) override;
   void OnTransportAdvanced(float amount) override;
   
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   void TextEntryComplete(TextEntry* entry) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& w, float& h) override { w=110; h=76; }
   
   ClickButton* mTriggerButton;
   TextEntry* mPitchEntry;
   FloatSlider* mVelocitySlider;
   FloatSlider* mDurationSlider;
   DropdownList* mNextSelector;
   int mPitch;
   float mVelocity;
   float mDuration;
   NoteInterval mNextInterval;
   float mNext;
   double mStartTime;
   bool mNoteOn;
   bool mWaitingToTrigger;
   bool mQueueTrigger;
   PatchCableSource* mNextNodeCable;
};

#endif /* defined(__Bespoke__NoteChainNode__) */
