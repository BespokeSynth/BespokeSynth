/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  Sampler.h
//  modularSynth
//
//  Created by Ryan Challinor on 2/5/14.
//
//

#pragma once

#include "IAudioProcessor.h"
#include "PolyphonyMgr.h"
#include "SampleVoice.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "PitchDetector.h"
#include "TextEntry.h"
#include "ClickButton.h"
#include "Sample.h"

class ofxJSONElement;

class Sampler : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public ITextEntryListener, public IButtonListener
{
public:
   Sampler();
   ~Sampler();
   static IDrawableModule* Create() { return new Sampler(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void Poll() override;

   //IAudioProcessor
   InputMode GetInputMode() override { return kInputMode_Mono; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   void FilesDropped(std::vector<std::string> files, int x, int y) override;
   void SampleDropped(int x, int y, Sample* sample) override;
   bool CanDropSample() const override { return true; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void TextEntryComplete(TextEntry* entry) override {}
   void ButtonClicked(ClickButton* button, double time) override;

   bool HasDebugDraw() const override { return true; }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 3; }

   bool IsEnabled() const override { return mEnabled; }

private:
   void StopRecording();
   float DetectSamplePitch();
   void UpdateForNewSample();

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   float mWidth{ 100 };
   float mHeight{ 100 };

   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   SampleVoiceParams mVoiceParams;
   FloatSlider* mVolSlider{ nullptr };
   ADSRDisplay* mADSRDisplay{ nullptr };
   float mThresh{ .2 };
   FloatSlider* mThreshSlider{ nullptr };
   IntSlider* mStartSlider{ nullptr };
   IntSlider* mStopSlider{ nullptr };
   bool mSustainLoop{ false };
   Checkbox* mSustainLoopCheckbox{ nullptr };
   IntSlider* mSustainLoopStartSlider{ nullptr };
   IntSlider* mSustainLoopEndSlider{ nullptr };

   Sample mSample{};
   int mRecordPos{ 0 };
   bool mRecording{ false };
   Checkbox* mRecordCheckbox{ nullptr };
   ClickButton* mDetectPitchButton{ nullptr };
   bool mPassthrough{ false };
   Checkbox* mPassthroughCheckbox{ nullptr };
   TextEntry* mSamplePitchEntry{ nullptr };
   int mMostRecentVoiceIdx{ -1 };

   ChannelBuffer mWriteBuffer;

   PitchDetector mPitchDetector;

   struct DebugLine
   {
      std::string text;
      ofColor color;
   };

   std::array<DebugLine, 20> mDebugLines;
   int mDebugLinesPos{ 0 };
};
