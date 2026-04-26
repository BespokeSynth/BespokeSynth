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
/*
  ==============================================================================

    SessionOrganizer.h
    Created: 7 Apr 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "Slider.h"
#include "RadioButton.h"

class TrackOrganizer;

class SessionOrganizer : public IDrawableModule, public IFloatSliderListener, public IRadioButtonListener
{
public:
   SessionOrganizer();
   virtual ~SessionOrganizer();
   static IDrawableModule* Create() { return new SessionOrganizer(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;

   TrackOrganizer* GetTrack(int index) const;
   int GetNumTracks() const { return (int)mTrackCables.size(); }

   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return true; }

private:
   class TrackColumn
   {
   public:
      void CreateUIControls(SessionOrganizer* sessionOrganizer, int index);
      auto Draw(SessionOrganizer* sessionOrganizer, int index) -> void;
      ofVec2f GetPosition(int index) const;
      void Poll(SessionOrganizer* sessionOrganizer, int index);

      bool mEnabled;
      Checkbox* mEnabledCheckbox{ nullptr };
      RadioButton* mSnapshotSelector{ nullptr };
      FloatSlider* mGainSlider{ nullptr };
      FloatSlider* mSendSlider{ nullptr };

   private:
      int mDummySnapshot;
      float mDummyGain;
      float mDummySend;
   };

   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;

   std::array<PatchCableSource*, 8> mTrackCables{ nullptr };
   std::array<TrackColumn, 8> mTrackColumns;
};
