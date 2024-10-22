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
//  MultitrackRecorder.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/13/14.
//
//

#pragma once

#include "IDrawableModule.h"
#include "Slider.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "IAudioProcessor.h"
#include "ModuleContainer.h"

class MultitrackRecorderTrack;

class MultitrackRecorder : public IDrawableModule, public IButtonListener
{
public:
   MultitrackRecorder();
   virtual ~MultitrackRecorder();
   static IDrawableModule* Create() { return new MultitrackRecorder(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   ModuleContainer* GetContainer() override { return &mModuleContainer; }
   bool IsResizable() const override { return true; }
   void Resize(float width, float height) override { mWidth = ofClamp(width, 210, 9999); }

   void RemoveTrack(MultitrackRecorderTrack* track);

   void ButtonClicked(ClickButton* button, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 0; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   void AddTrack();
   int GetRecordingLength();

   float mWidth{ 700 };
   float mHeight{ 142 };

   ModuleContainer mModuleContainer;

   ClickButton* mAddTrackButton{ nullptr };
   Checkbox* mRecordCheckbox{ nullptr };
   bool mRecord{ false };
   ClickButton* mBounceButton{ nullptr };
   ClickButton* mClearButton{ nullptr };

   std::vector<MultitrackRecorderTrack*> mTracks;
   std::string mStatusString;
   double mStatusStringTime{ -9999 };
};

class MultitrackRecorderTrack : public IAudioProcessor, public IDrawableModule, public IFloatSliderListener, public IButtonListener
{
public:
   MultitrackRecorderTrack();
   virtual ~MultitrackRecorderTrack();
   static IDrawableModule* Create() { return new MultitrackRecorderTrack(); }
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   bool HasTitleBar() const override { return false; }

   void Poll() override;
   void Process(double time) override;

   void Setup(MultitrackRecorder* recorder, int minLength);
   void SetRecording(bool record);
   Sample* BounceRecording();
   void Clear();
   int GetRecordingLength() const { return mRecordingLength; }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;

   MultitrackRecorder* mRecorder{ nullptr };

   std::vector<ChannelBuffer*> mRecordChunks;
   bool mDoRecording{ false };
   int mRecordingLength{ 0 };
   ClickButton* mDeleteButton{ nullptr };
};
