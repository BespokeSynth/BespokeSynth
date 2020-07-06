//
//  Polyrhythms.h
//  modularSynth
//
//  Created by Ryan Challinor on 3/12/13.
//
//

#ifndef __modularSynth__Polyrhythms__
#define __modularSynth__Polyrhythms__

#include <iostream>
#include "Transport.h"
#include "UIGrid.h"
#include "Checkbox.h"
#include "UIGrid.h"
#include "Slider.h"
#include "DropdownList.h"
#include "IDrawableModule.h"
#include "INoteSource.h"

class Polyrhythms;

class RhythmLine
{
public:
   RhythmLine(Polyrhythms* owner, int index);
   void Draw();
   void OnClicked(int x, int y, bool right);
   void MouseReleased();
   void MouseMoved(float x, float y);
   void CreateUIControls();
   void OnResize();
   void UpdateGrid();
   
   int mIndex;
   UIGrid* mGrid;
   int mLength;
   DropdownList* mLengthSelector;
   int mNote;
   DropdownList* mNoteSelector;
   Polyrhythms* mOwner;
};

class Polyrhythms : public IDrawableModule, public INoteSource, public IAudioPoller, public IFloatSliderListener, public IDropdownListener
{
public:
   Polyrhythms();
   ~Polyrhythms();
   static IDrawableModule* Create() { return new Polyrhythms(); }
   
   string GetTitleLabel() override { return "polyrhythms"; }
   void CreateUIControls() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;

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
   void GetModuleDimensions(float& width, float& height) override { width=mWidth; height=mHeight; }
   void OnClicked(int x, int y, bool right) override;
   
   int mNumLines;
   float mWidth;
   float mHeight;
   std::vector<RhythmLine*> mRhythmLines;
};

#endif /* defined(__modularSynth__Polyrhythms__) */

