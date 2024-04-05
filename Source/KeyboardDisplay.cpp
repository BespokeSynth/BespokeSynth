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
//  KeyboardDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/16.
//  Tweaked by ArkyonVeil on April/2024
//

#include "KeyboardDisplay.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "ModuleContainer.h"
#include "FileStream.h"
#include "Keyboard2MidiLayout.h"
#include "ModularSynth.h"
#include "QuickSpawnMenu.h"

namespace
{
   const float kKeyboardYOffset = 0;
}

KeyboardDisplay::KeyboardDisplay()
{
   for (int i = 0; i < 128; ++i)
   {
      mLastOnTime[i] = 0;
      mLastOffTime[i] = 0;
   }
   KeyboardDisplay::OnHoverEnter();
}
void KeyboardDisplay::OnHoverEnter()
{
   mTypingInput = true;
   QuickSpawnMenu::enableKeyboardInput = false;
}
void KeyboardDisplay::OnHoverExit()
{
   if (mMidPress <= 0)
   {
      mTypingInput = false;
      QuickSpawnMenu::enableKeyboardInput = true;
   }
}
void KeyboardDisplay::OnSelect()
{
}
void KeyboardDisplay::OnDeselect()
{
}
void KeyboardDisplay::Exit()
{
   QuickSpawnMenu::enableKeyboardInput = true;
   IDrawableModule::Exit();
}

void KeyboardDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}
void KeyboardDisplay::SetEnabled(bool enabled)
{
   //Todo, figure out a way to flush out pressed notes during this step.
   mEnabled = enabled;
}

void KeyboardDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawKeyboard(0, kKeyboardYOffset, mWidth, mHeight - kKeyboardYOffset);

   DrawHoverSelect();
}

void KeyboardDisplay::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);

   if (pitch >= 0 && pitch < 128)
   {
      if (velocity > 0)
      {
         mLastOnTime[pitch] = time;
         mLastOffTime[pitch] = 0;
      }
      else
      {
         mLastOffTime[pitch] = time;
      }
   }
}

void KeyboardDisplay::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (IsHoveringOverResizeHandle() || !IsEnabled())
      return;

   double time = NextBufferTime(false);
   for (int i = 0; i < NumKeys(); ++i)
   {
      for (int pass = 0; pass < 2; ++pass)
      {
         for (int i = 0; i < NumKeys(); ++i)
         {
            bool isBlackKey;
            if (GetKeyboardKeyRect(i + RootKey(), mWidth, mHeight - kKeyboardYOffset, isBlackKey).contains(x, y - kKeyboardYOffset))
            {
               if ((pass == 0 && isBlackKey) || (pass == 1 && !isBlackKey))
               {
                  int pitch = i + RootKey();
                  if (mPlayingMousePitch == -1 || !mLatch)
                  {
                     PlayNote(time, pitch, 127);
                     mPlayingMousePitch = pitch;
                  }
                  else
                  {
                     bool newNote = (mPlayingMousePitch != pitch);
                     if (newNote)
                        PlayNote(time, pitch, 127);
                     PlayNote(time, mPlayingMousePitch, 0);
                     mPlayingMousePitch = newNote ? pitch : -1;
                  }
                  return;
               }
            }
         }
      }
   }
}

void KeyboardDisplay::MouseReleased()
{
   IDrawableModule::MouseReleased();
   if (mPlayingMousePitch != -1 && !mLatch)
   {
      double time = NextBufferTime(false);
      PlayNote(time, mPlayingMousePitch, 0);
      mPlayingMousePitch = -1;
   }
}

int KeyboardDisplay::RootKey() const
{
   return TheScale->GetPitchesPerOctave() * mRootOctave;
}

int KeyboardDisplay::NumKeys() const
{
   return TheScale->GetPitchesPerOctave() * mNumOctaves + 1;
}

void KeyboardDisplay::SetPitchColor(int pitch)
{
   if (mShowScale)
   {
      if (TheScale->IsRoot(pitch))
         ofSetColor(0, 200, 0);
      else if (TheScale->IsInPentatonic(pitch))
         ofSetColor(255, 128, 0);
      else if (TheScale->IsInScale(pitch))
         ofSetColor(180, 80, 0);
      else
         ofSetColor(50, 50, 50);
   }
   else
   {
      int enableOffset = 0;
      if (!IsEnabled())
         enableOffset = 80;

      pitch %= 12;
      ofColor color;
      if (pitch == 0 || pitch == 2 || pitch == 4 || pitch == 5 || pitch == 7 || pitch == 9 || pitch == 11)
         color.setHsb(240, 145, 200 - enableOffset);
      else
         color.setHsb(240, 145, 120 - enableOffset);
      ofSetColor(color);
   }
}

void KeyboardDisplay::Resize(float w, float h)
{
   mWidth = w;
   mHeight = h;

   RefreshOctaveCount();
}

void KeyboardDisplay::RefreshOctaveCount()
{
   if (mForceNumOctaves == 0)
   {
      float ratio = mWidth / mHeight;

      constexpr float baseRatioForOneElement = 250.0f / 180.0f;

      int elements = static_cast<int>(ratio / baseRatioForOneElement);

      elements = std::clamp(elements, 1, 10);

      if (mRootOctave + elements > 9) //Ensure that we can't go into octaves where it begins to break...
         mRootOctave = 10 - mNumOctaves;
      mNumOctaves = elements;
   }
   else
   {
      mNumOctaves = mForceNumOctaves;
      mRootOctave = 10 - mNumOctaves;
   }
}

void KeyboardDisplay::DrawKeyboard(int x, int y, int w, int h)
{
   ofPushStyle();
   ofPushMatrix();
   ofTranslate(x, y);

   for (int pass = 0; pass < 2; ++pass)
   {
      for (int i = 0; i < NumKeys(); ++i)
      {
         bool isBlackKey;
         ofRectangle key = GetKeyboardKeyRect(i + RootKey(), w, h, isBlackKey);

         if ((pass == 0 && !isBlackKey) || (pass == 1 && isBlackKey))
         {
            SetPitchColor(i);
            ofFill();
            ofRect(key);
            ofSetColor(0, 0, 0);
            ofNoFill();
            ofRect(key);
         }
      }
   }

   float keySpace = (float)w / ((float)NumKeys() - mNumOctaves * 5);

   int oct = mRootOctave;
   if (keySpace > 16 && h > 34 && !mHideLabels)
      for (int i = 0; i < NumKeys(); i += 7)
      {
         ofSetColor(108, 37, 62, 255);
         DrawTextNormal("C" + std::to_string(oct), keySpace * 0.5f - 6.5f + i * keySpace, h - 8, 14);
         oct++;
      }


   ofPushStyle();
   ofFill();
   ofSetLineWidth(2);
   for (int pitch = RootKey(); pitch < RootKey() + NumKeys(); ++pitch)
   {
      if (gTime >= mLastOnTime[pitch] && (gTime <= mLastOffTime[pitch] || mLastOffTime[pitch] < mLastOnTime[pitch]))
      {
         bool isBlackKey;
         ofRectangle key = GetKeyboardKeyRect(pitch, w, h, isBlackKey);
         key.height /= 3;
         key.y += key.height * 2;
         ofSetColor(255, 255, 255, ofLerp(255, 150, ofClamp((gTime - mLastOnTime[pitch]) / 150.0f, 0, 1)));
         ofRect(key);
      }
   }
   ofPopStyle();

   ofPopMatrix();
   ofPopStyle();
}

ofRectangle KeyboardDisplay::GetKeyboardKeyRect(int pitch, int w, int h, bool& isBlackKey) const
{
   float extraKeyWidth = w / (mNumOctaves * 7 + 1);
   float octaveWidth = (w - extraKeyWidth) / mNumOctaves;

   pitch -= RootKey();

   float offset = pitch / TheScale->GetPitchesPerOctave() * (octaveWidth);
   pitch %= 12;

   if ((pitch <= 4 && pitch % 2 == 0) || (pitch >= 5 && pitch % 2 == 1)) //white key
   {
      int whiteKey = (pitch + 1) / 2;
      isBlackKey = false;
      return ofRectangle(offset + whiteKey * octaveWidth / 7, 0, octaveWidth / 7, h);
   }
   else //black key
   {
      int blackKey = pitch / 2;
      isBlackKey = true;
      return ofRectangle(offset + blackKey * octaveWidth / 7 + octaveWidth / 16 + octaveWidth / 7 * .1f, 0, octaveWidth / 7 * .8f, h / 2);
   }
}

void KeyboardDisplay::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   //kPress++;
   //TheSynth->LogEvent("Press:"+std::to_string(debugKPress)+" Release:"+std::to_string(debugKReles),kLogEventType_Verbose);


   if (mTypingInput && mEnabled && !isRepeat)
   {
      double time = NextBufferTime(false);
      Keyboard2MidiResponse res = Keyboard2MidiLayout::GetPitchForComputerKey(key, mRootOctave);

      int pitch = res.pitch;
      if (mRootOctave != res.newOctave)
      {
         mRootOctave = res.newOctave;
         if (mRootOctave + mNumOctaves > 9) //Ensure that we can't go into octaves where it begins to break...
            mRootOctave = 10 - mNumOctaves;
         return;
      }
      if (pitch != -1)
      {
         //mMidPress++;
         mKeyPressRegister[key] = pitch;
         PlayNote(time, pitch, 127);
      }
   }

   /*
   else if (!mTypingInput && mMidPress > 0)
   {
      mMidPress = 0;
      //Since KeyRelease events can get eaten in this situation, flush the notes to prevent stucks.
      for (int i = 0; i < 127; i++)
      {
         double time = NextBufferTime(false);
         mNoteOutput.Flush(time);
         for (int i = 0; i < 127; ++i)
            PlayNote(time, i, 0);
      }
   }*/
}

void KeyboardDisplay::KeyReleased(int key)
{
   //kReles++;
   //TheSynth->LogEvent("Press:"+std::to_string(debugKPress)+" Release:"+std::to_string(debugKReles),kLogEventType_Verbose);

   //If no KeyRelease arrives, we can get a "stuck" note.
   if (true)
   {
      double time = NextBufferTime(false);
      int pitch;

      auto it = mKeyPressRegister.find(key);
      if (it != mKeyPressRegister.end())
         pitch = it->second; // Key found, use the found value
      else
         pitch = -1; // Key not found, use default value

      mKeyPressRegister.erase(key);
      if (pitch != -1)
      {
         PlayNote(time, pitch, 0);
         if (!hovered)
         {
            mTypingInput = false;
            QuickSpawnMenu::enableKeyboardInput = true;
         }
      }
   }
}

void KeyboardDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("force_num_octaves", moduleInfo, 0, 0, 10, K(isTextField));
   mModuleSaveData.LoadBool("latch", moduleInfo, false);
   mModuleSaveData.LoadBool("show_scale", moduleInfo, false);
   mModuleSaveData.LoadBool("hide_labels", moduleInfo, false);

   SetUpFromSaveData();
}

void KeyboardDisplay::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mForceNumOctaves = mModuleSaveData.GetInt("force_num_octaves");
   mLatch = mModuleSaveData.GetBool("latch");
   mShowScale = mModuleSaveData.GetBool("show_scale");
   mHideLabels = mModuleSaveData.GetBool("hide_labels");

   if (mForceNumOctaves)
      RefreshOctaveCount();
}

void KeyboardDisplay::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mWidth;
   out << mHeight;
   out << mNumOctaves;
   out << mRootOctave;
}

void KeyboardDisplay::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return; //this was saved before we added versioning, bail out

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   in >> mWidth;
   in >> mHeight;
   in >> mNumOctaves;
   in >> mRootOctave;
}