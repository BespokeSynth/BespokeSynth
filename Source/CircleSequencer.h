//
//  CircleSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/3/15.
//
//

#ifndef __Bespoke__CircleSequencer__
#define __Bespoke__CircleSequencer__

#include <iostream>
#include "Transport.h"
#include "UIGrid.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"

class CircleSequencer;

#define CIRCLE_SEQUENCER_MAX_STEPS 10

class CircleSequencerRing
{
public:
   CircleSequencerRing(CircleSequencer* owner, int index);
   void Draw();
   void OnClicked(int x, int y, bool right);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void CreateUIControls();
   void OnTransportAdvanced(float amount);
private:
   float GetRadius() { return 90-mIndex*15; }
   int mLength;
   DropdownList* mLengthSelector;
   int mNote;
   DropdownList* mNoteSelector;
   CircleSequencer* mOwner;
   int mIndex;
   float mSteps[CIRCLE_SEQUENCER_MAX_STEPS];
   float mOffset;
   FloatSlider* mOffsetSlider;
   int mCurrentlyClickedStepIdx;
   float mLastMouseRadius;
};

class CircleSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener
{
public:
   CircleSequencer();
   ~CircleSequencer();
   static IDrawableModule* Create() { return new CircleSequencer(); }
   
   string GetTitleLabel() override { return "circle sequencer"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool on) override { mEnabled = on; }
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   //IClickable
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=400; height=200; }
   void OnClicked(int x, int y, bool right) override;
   
   std::vector<CircleSequencerRing*> mCircleSequencerRings;
};

#endif /* defined(__Bespoke__CircleSequencer__) */
