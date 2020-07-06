//
//  SamplerGrid.h
//  Bespoke
//
//  Created by Ryan Challinor on 6/12/14.
//
//

#ifndef __Bespoke__SamplerGrid__
#define __Bespoke__SamplerGrid__

#include <iostream>
#include "IAudioProcessor.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "MidiDevice.h"
#include "Ramp.h"
#include "GridController.h"
#include "UIGrid.h"

class ofxJSONElement;

#define MAX_SAMPLER_GRID_LENGTH 5*44100
#define SAMPLE_RAMP_MS 3

class SamplerGrid : public IAudioProcessor, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IGridControllerListener, public UIGridListener, public INoteReceiver
{
public:
   SamplerGrid();
   ~SamplerGrid();
   static IDrawableModule* Create() { return new SamplerGrid(); }
   
   string GetTitleLabel() override { return "sampler grid"; }
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }
   
   //IGridControllerListener
   void OnControllerPageSelected() override;
   void OnGridButton(int x, int y, float velocity, IGridController* grid) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   //UIGridListener
   void GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue) override;
   
   void FilesDropped(vector<string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   void MouseReleased() override;
   
   void InitGrid();
   void UpdateLights();
   
   int GridToIdx(int x, int y) { return x+y*mCols; }
   
   Checkbox* mClearCheckbox;
   bool mClear;
   
   struct GridSample
   {
      float mSampleData[MAX_SAMPLER_GRID_LENGTH];
      int mPlayhead;
      bool mHasSample;
      bool mRecordingArmed;
      int mSampleLength;
      Ramp mRamp;
      int mSampleStart;
      int mSampleEnd;
   };
   
   void SetEditSample(GridSample* sample);
   
   GridSample* mGridSamples;
   int mRecordingSample;
   
   bool mPassthrough;
   Checkbox* mPassthroughCheckbox;
   
   float* mWriteBuffer;
   
   float mVolume;
   FloatSlider* mVolumeSlider;
   bool mEditMode;
   Checkbox* mEditCheckbox;
   bool mDuplicate;
   Checkbox* mDuplicateCheckbox;
   
   GridController* mGridController;
   int mCols;
   int mRows;
   bool mLastColumnIsGroup;
   
   int mEditSampleX;
   int mEditSampleY;
   float mEditSampleWidth;
   float mEditSampleHeight;
   GridSample* mEditSample;
   IntSlider* mEditStartSlider;
   IntSlider* mEditEndSlider;
   int mDummyInt;
   
   UIGrid* mGrid;
};

#endif /* defined(__Bespoke__SamplerGrid__) */

