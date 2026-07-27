#pragma once

#include "IAudioProcessor.h"
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "ClickButton.h"
#include "Slider.h"
#include "Transport.h"
#include <vector>

enum FxType
{
   kFx_Bitcrush = 0,
   kFx_Filter,
   kFx_Pitch,
   kFx_Stutter,
   kFx_Reverb,
   kFx_Gate,
   kFx_Delay,
   kFx_Count
};

enum FxPreset
{
   kPreset_Custom = 0,
   kPreset_Clear,
   kPreset_GlitchStutter,
   kPreset_BitcrushDrop,
   kPreset_FilterRiser,
   kPreset_ChaosRandom,
   kPreset_DenseChaos,
   kPreset_RhythmicEcho,
   kPreset_AmbientWash,
   kPreset_SteppedRiser
};

class EffectMatrix : public IAudioProcessor, public IDrawableModule, public IDropdownListener, public IButtonListener, public IFloatSliderListener
{
public:
   EffectMatrix();
   virtual ~EffectMatrix();

   static IDrawableModule* Create() { return new EffectMatrix(); }
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   // IAudioProcessor / IAudioSource
   void Process(double time) override;

   // IDrawableModule
   void CreateUIControls() override;
   void Poll() override;
   void DrawModule() override;
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   void Resize(float w, float h) override;

   // Listeners
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   // Save/Load
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

private:
   void ApplyPreset(FxPreset preset);
   void ClearGrid();
   void RandomizeGrid(int rowToRandomize = -1);

   static const int NUM_STEPS = 16;
   static const int NUM_FX = 7;

   // Grid State: 6 rows, 16 cols
   float mGrid[NUM_FX][NUM_STEPS];

   // UI Controls
   DropdownList* mQuantizeLengthSelector{ nullptr };
   DropdownList* mPresetSelector{ nullptr };
   ClickButton* mClearButton{ nullptr };
   ClickButton* mRandomButton{ nullptr };
   ClickButton* mRowRandomButtons[7]{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
   FloatSlider* mRandomDensitySlider{ nullptr };
   FloatSlider* mRandomIntensitySlider{ nullptr };
   FloatSlider* mMixSlider{ nullptr };

   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_16n };
   FxPreset mCurrentPreset{ kPreset_Custom };
   float mWetDry{ 1.0f };
   float mRandomDensity{ 0.5f };
   float mRandomIntensity{ 0.8f };

   // Playback tracking
   double mPlayStartTime{ 0.0 };
   int mCurrentStep{ -1 };
   float mPlayheadProgress{ 0.0f };

   // Grid interaction
   bool mIsDraggingCell{ false };
   int mDragRow{ 0 };
   int mDragCol{ 0 };
   float mDragStartY{ 0 };
   float mDragStartValue{ 0 };

   // Layout
   float mArenaX{ 0 };
   float mArenaY{ 0 };
   float mArenaW{ 460 };
   float mArenaH{ 200 };

   // DSP States
   // 1. Bitcrush
   float mBitcrushHeld[2]{ 0, 0 };
   int mBitcrushCounter{ 0 };

   // 2. Filter (State Variable Filter)
   float mFilterLow[2]{ 0, 0 };
   float mFilterBand[2]{ 0, 0 };

   // 3. Pitch Shift (Granular delay)
   static const int PITCH_BUF_SIZE = 8192;
   std::vector<float> mPitchBuffer0;
   std::vector<float> mPitchBuffer1;
   int mPitchWritePos{ 0 };
   float mPitchReadPos[2]{ 0, 0 };

   // 4. Stutter
   static const int STUTTER_BUF_SIZE = 44100;
   std::vector<float> mStutterBuffer0;
   std::vector<float> mStutterBuffer1;
   int mStutterWritePos{ 0 };
   int mStutterReadPos{ 0 };

   // 5. Reverb
   static const int REV_DEL_1 = 1411;
   static const int REV_DEL_2 = 1787;
   static const int REV_DEL_3 = 2111;
   std::vector<float> mRevBuf1L, mRevBuf1R;
   std::vector<float> mRevBuf2L, mRevBuf2R;
   std::vector<float> mRevBuf3L, mRevBuf3R;
   int mRevIdx1{ 0 }, mRevIdx2{ 0 }, mRevIdx3{ 0 };

   // 7. Delay
   static const int DELAY_BUF_SIZE = 88200;
   std::vector<float> mDelayBuffer0;
   std::vector<float> mDelayBuffer1;
   int mDelayWritePos{ 0 };

   ChannelBuffer mOutputBuffer;
};
