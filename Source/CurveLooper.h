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

#ifndef __Bespoke__CurveLooper__
#define __Bespoke__CurveLooper__

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Transport.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "EnvelopeEditor.h"

class PatchCableSource;

class CurveLooper : public IDrawableModule, public IDropdownListener, public IButtonListener, public IAudioPoller
{
public:
   CurveLooper();
   ~CurveLooper();
   static IDrawableModule* Create() { return new CurveLooper(); }

   void CreateUIControls() override;

   void OnTransportAdvanced(float amount) override;

   //IDrawableModule
   void Init() override;
   void Poll() override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   void CheckboxUpdated(Checkbox* checkbox) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void ButtonClicked(ClickButton* button) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

private:
   float GetPlaybackPosition();

   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return mEnabled; }
   void GetModuleDimensions(float& width, float& height) override;
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;

   std::array<IUIControl*, 16> mUIControls{ nullptr };
   int mLength;
   DropdownList* mLengthSelector;
   PatchCableSource* mControlCable;
   float mWidth;
   float mHeight;
   EnvelopeControl mEnvelopeControl;
   ::ADSR mAdsr;
   ClickButton* mRandomizeButton;
};

#endif /* defined(__Bespoke__CurveLooper__) */
