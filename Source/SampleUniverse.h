#pragma once

#include "IAudioSource.h"
#include "IDrawableModule.h"
#include "Sample.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Transport.h"
#include <vector>

enum ShapePreset
{
   kShape_Random,
   kShape_Line,
   kShape_Circle,
   kShape_Spiral,
   kShape_Square,
   kShape_Triangle,
   kShape_Cross,
   kShape_Star,
   kShape_Torus,
   kShape_Cat,
   kShape_Frog,
   kShape_Computer
};

class SampleUniverse : public IAudioSource, public IDrawableModule, public ITextEntryListener, public IButtonListener, public IFloatSliderListener, public IDropdownListener
{
public:
   SampleUniverse();
   virtual ~SampleUniverse();

   static IDrawableModule* Create() { return new SampleUniverse(); }

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   // IAudioSource
   void Process(double time) override;

   // IDrawableModule
   void CreateUIControls() override;
   void Poll() override;
   void DrawModule() override;
   void OnClicked(float x, float y, bool right) override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   void Resize(float w, float h) override;

   bool CanDropSample() const override { return true; }
   void SampleDropped(int x, int y, Sample* sample) override;
   void FilesDropped(std::vector<std::string> files, int x, int y) override;

   // Listeners
   void TextEntryComplete(TextEntry* entry) override;
   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;

   // Save/Load
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }

private:
   void RespawnDots();
   void Reseed();
   void GenerateRandomPath();
   ofVec2f GetShapePosition(ShapePreset shape, float t, int i, int N);
   void TriggerSample(int index, double time);
   void ClearSamples();

   struct UniverseSample
   {
      Sample* sample{ nullptr };
      float x{ 0 };
      float y{ 0 };
      ofColor color;
      float hitTime{ 0 }; // decays from 1.0 to 0.0 for bump animation
   };

   struct SampleVoice
   {
      int sampleIndex{ -1 };
      double playPos{ 0.0 };
      bool playing{ false };
   };

   std::vector<UniverseSample> mSamples;
   std::vector<SampleVoice> mVoices;

   int mSeed{ 1 };
   int mHoveredIndex{ -1 };

   // UI Controls
   TextEntry* mSeedEntry{ nullptr };
   ClickButton* mPrevSeedButton{ nullptr };
   ClickButton* mNextSeedButton{ nullptr };
   ClickButton* mReseedButton{ nullptr };
   ClickButton* mClearButton{ nullptr };

   Checkbox* mRecordCheckbox{ nullptr };
   Checkbox* mPlayPathCheckbox{ nullptr };
   Checkbox* mLoopPathCheckbox{ nullptr };
   Checkbox* mReversePathCheckbox{ nullptr };
   DropdownList* mQuantizeLengthSelector;
   NoteInterval mQuantizeInterval{ NoteInterval::kInterval_4n };

   DropdownList* mShapeSelector;
   ShapePreset mCurrentShape{ kShape_Random };

   // State
   bool mRecording{ false };
   bool mPlayingPath{ false };
   bool mLoopPath{ false };
   bool mReversePath{ false };
   float mVolume{ 1.0f };

   double mPlayStartTime{ 0.0 };
   bool mIsDrawing{ false };

   std::vector<ofVec2f> mRecordedPath;
   float mPlayheadPos{ 0.0f };
   float mPlayheadProgress{ 0.0f }; // continuous 0-1 for smooth visual

   ChannelBuffer mOutputBuffer;

   float mArenaX{ 0 };
   float mArenaY{ 0 };
   float mArenaW{ 400 };
   float mArenaH{ 300 };
};
