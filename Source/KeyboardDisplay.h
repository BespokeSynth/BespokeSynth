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
//  KeyboardDisplay.h
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/16.
//  Tweaked by ArkyonVeil on April/2024
//

#pragma once

#include "IDrawableModule.h"
#include "NoteEffectBase.h"
#include <unordered_map>

class KeyboardDisplay : public IDrawableModule, public NoteEffectBase, public IKeyboardFocusListener
{
public:
   KeyboardDisplay();
   static IDrawableModule* Create() { return new KeyboardDisplay(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(NoteMessage note) override;

   void MouseReleased() override;
   void KeyReleased(int key) override;

   //IKeyboardFocusListener
   void OnKeyPressed(int key, bool isRepeat) override;
   bool ShouldConsumeKey(int key) override;
   bool CanTakeFocus() override { return mAllowHoverTypingInput; }

   void CheckboxUpdated(Checkbox* checkbox, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 2; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   void RefreshOctaveCount();

   void DrawKeyboard(int x, int y, int w, int h);
   void SetPitchColor(int pitch);
   ofRectangle GetKeyboardKeyRect(int pitch, int w, int h, bool& isBlackKey) const;

   int RootKey() const;
   int NumKeys() const;

   float mWidth{ 500 };
   float mHeight{ 110 };
   int mRootOctave{ 3 };
   int mNumOctaves{ 3 };
   int mForceNumOctaves{ 0 };
   int mPlayingMousePitch{ -1 };
   bool mAllowHoverTypingInput{ true };
   bool mLatch{ false };
   bool mShowScale{ false };
   bool mGetVelocityFromClickHeight{ false };
   bool mHideLabels{ false };
   std::array<double, 128> mLastOnTime{};
   std::array<double, 128> mLastOffTime{};
   std::unordered_map<int, int> mKeyPressRegister{};
};
