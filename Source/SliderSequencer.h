//
//  SliderSequencer.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/25/14.
//
//

#ifndef __Bespoke__SliderSequencer__
#define __Bespoke__SliderSequencer__

#include <iostream>

#include "Transport.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"

class SliderSequencer;

class SliderLine
{
public:
   SliderLine(SliderSequencer* owner, int x, int y, int index);
   void Draw();
   void CreateUIControls();
   
   float mPoint;
   FloatSlider* mSlider;
   float mVelocity;
   FloatSlider* mVelocitySlider;
   int mNote;
   DropdownList* mNoteSelector;
   double mPlayTime;
   bool mPlaying;
   Checkbox* mPlayingCheckbox;
   int mX;
   int mY;
   SliderSequencer* mOwner;
   int mIndex;
};

class SliderSequencer : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener, public IIntSliderListener
{
public:
   SliderSequencer();
   ~SliderSequencer();
   static IDrawableModule* Create() { return new SliderSequencer(); }
   
   string GetTitleLabel() override { return "slider sequencer"; }
   void CreateUIControls() override;
   
   //IAudioPoller
   void OnTransportAdvanced(float amount) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
private:
   float MeasurePos();
   
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width=320; height=165; }
   
   float mLastMeasurePos;
   std::vector<SliderLine*> mSliderLines;
   int mDivision;
   IntSlider* mDivisionSlider;
};


#endif /* defined(__Bespoke__SliderSequencer__) */


