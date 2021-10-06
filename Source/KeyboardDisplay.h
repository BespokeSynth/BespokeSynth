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
//
//

#ifndef __Bespoke__KeyboardDisplay__
#define __Bespoke__KeyboardDisplay__

#include "IDrawableModule.h"
#include "NoteEffectBase.h"

class KeyboardDisplay : public NoteEffectBase, public IDrawableModule
{
public:
   KeyboardDisplay();
   static IDrawableModule* Create() { return new KeyboardDisplay(); }
   
   std::string GetTitleLabel() override { return "keyboard"; }
   void CreateUIControls() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   void MouseReleased() override;
   bool SignalEmit(const Signal& ev) override;
   void KeyPressed(int key, bool isRepeat) override;
   void KeyReleased(int key) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   bool Enabled() const override { return mEnabled; }
   void OnClicked(int x, int y, bool right) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override { mWidth = w; mHeight = h; }
   
   void DrawKeyboard(int x, int y, int w, int h);
   void SetPitchColor(int pitch);
   ofRectangle GetKeyboardKeyRect(int pitch, int w, int h, bool& isBlackKey) const;
   
   int RootKey() const;
   int NumKeys() const;
   int GetPitchForTypingKey(int key) const;
   
   float mWidth;
   float mHeight;
   int mRootOctave;
   int mNumOctaves;
   int mPlayingMousePitch;
   bool mTypingInput;
   bool mLatch;
   bool mShowScale;
   bool mRegisterKeys;
   std::array<float, 128> mLastOnTime{};
   std::array<float, 128> mLastOffTime{};
   std::vector<std::pair<Signal,Signal>> keymap;
};

#endif /* defined(__Bespoke__KeyboardDisplay__) */
