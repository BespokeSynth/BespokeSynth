//
//  ClipArranger.h
//  Bespoke
//
//  Created by Ryan Challinor on 8/26/14.
//
//

#ifndef __Bespoke__ClipArranger__
#define __Bespoke__ClipArranger__

#include <iostream>
#include "IAudioReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "RollingBuffer.h"
#include "Ramp.h"
#include "Checkbox.h"
#include "NamedMutex.h"
#include "Sample.h"

class ClipArranger : public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   ClipArranger();
   virtual ~ClipArranger();
   static IDrawableModule* Create() { return new ClipArranger(); }
   
   void Poll() override;
   void Process(double time, float* left, float* right, int bufferSize);
   
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   
   float MouseXToBufferPos(float mouseX);
   int MouseXToSample(float mouseX);
   float SampleToX(int sample);
   bool IsMousePosWithinClip(int x, int y);
   void AddSample(Sample* sample, int x, int y);
   
   static const int MAX_CLIPS = 50;
   static const int BUFFER_MARGIN_X = 5;
   static const int BUFFER_MARGIN_Y = 5;
   
   class Clip
   {
   public:
      Clip()
      : mSample(nullptr)
      {
      }
      
      void Process(float* left, float* right, int bufferSize);
      
      Sample* mSample;
      int mStartSample;
      int mEndSample;
   };
   
   enum ClipMoveMode
   {
      kMoveMode_None,
      kMoveMode_Start,
      kMoveMode_End
   }mMoveMode;
   
   Clip* GetEmptyClip();
   
   Clip mClips[MAX_CLIPS];
   
   float mBufferWidth;
   float mBufferHeight;
   int mHighlightClip;
   bool mMouseDown;
   int mLastMouseX;
   int mLastMouseY;
   NamedMutex mMutex;
};

#endif /* defined(__Bespoke__ClipArranger__) */
