//
//  LoopStorer.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/22/15.
//
//

#ifndef __Bespoke__LoopStorer__
#define __Bespoke__LoopStorer__

#include "EnvOscillator.h"
#include "IDrawableModule.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include "ClickButton.h"
#include "Transport.h"
#include "OpenFrameworksPort.h"

class Sample;
class Looper;
class ChannelBuffer;

class LoopStorer : public IDrawableModule, public IFloatSliderListener, public IIntSliderListener, public IDropdownListener, public IButtonListener, public ITimeListener
{
public:
   LoopStorer();
   ~LoopStorer();
   static IDrawableModule* Create() { return new LoopStorer(); }
   
   string GetTitleLabel() override { return "loop storer"; }
   void CreateUIControls() override;
   
   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   int GetRowY(int idx);
   Looper* GetLooper() { return mLooper; }
   int GetQueuedBufferIdx() { return mQueuedSwapBufferIdx; }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   void OnTimeEvent(double time) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   
   void SwapBuffer(int swapToIdx);
   
   class SampleData
   {
   public:
      SampleData();
      ~SampleData();
      
      void Init(LoopStorer* storer, int index);
      void Draw();
      
      ChannelBuffer* mBuffer;
      int mNumBars;
      Checkbox* mSelectCheckbox;
      LoopStorer* mLoopStorer;
      int mIndex;
      int mBufferLength;
      bool mIsCurrentBuffer;
   };
   
   Looper* mLooper;
   Checkbox* mRewriteToSelectionCheckbox;
   bool mRewriteToSelection;
   DropdownList* mQuantizationDropdown;
   NoteInterval mQuantization;
   int mQueuedSwapBufferIdx;
   ofMutex mSwapMutex;
   bool mIsSwapping;
   ClickButton* mClearButton;
   ofMutex mLoadMutex;
   
   vector<SampleData*> mSamples;
   int mCurrentBufferIdx;
   
   PatchCableSource* mLooperCable;
};

#endif /* defined(__Bespoke__LoopStorer__) */
