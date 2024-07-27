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
//  CurveLooper.h
//  Bespoke
//
//  Created by Ryan Challinor on 3/5/16.
//
//

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "EnvelopeEditor.h"

class PatchCableSource;

class CurveLooper : public IDrawableModule, public IDropdownListener, public IButtonListener, public IAudioPoller
{
public:
   CurveLooper();
   ~CurveLooper();
   static IDrawableModule* Create() { return new CurveLooper(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void OnTransportAdvanced(float amount) override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   bool IsEnabled() const override { return mEnabled; }

private:
   float GetPlaybackPosition();

   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   std::array<IUIControl*, 16> mUIControls{ nullptr };
   int mLength{ 1 };
   DropdownList* mLengthSelector{ nullptr };
   PatchCableSource* mControlCable{ nullptr };
   float mWidth{ 200 };
   float mHeight{ 120 };
   EnvelopeControl mEnvelopeControl{ ofVec2f(5, 25), ofVec2f(mWidth - 10, mHeight - 30) };
   ::ADSR mAdsr;
   ClickButton* mRandomizeButton{ nullptr };
};
