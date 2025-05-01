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
#include "QwertyToPitchMapping.h"
#include "ModularSynth.h"

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
}

void KeyboardDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void KeyboardDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawKeyboard(0, kKeyboardYOffset, mWidth, mHeight - kKeyboardYOffset);
}

void KeyboardDisplay::PlayNote(NoteMessage note)
{
   PlayNoteOutput(note);

   if (note.pitch >= 0 && note.pitch < 128)
   {
      if (note.velocity > 0)
      {
         mLastOnTime[note.pitch] = note.time;
         mLastOffTime[note.pitch] = 0;
      }
      else
      {
         mLastOffTime[note.pitch] = note.time;
      }
   }
}

void KeyboardDisplay::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (IsHoveringOverResizeHandle() || !IsEnabled())
      return;

   double time = NextBufferTime(false);
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

               float minVelocityY;
               float maxVelocityY;

               if (isBlackKey)
               {
                  minVelocityY = 0;
                  maxVelocityY = (mHeight / 2) * .9f;
               }
               else
               {
                  minVelocityY = mHeight / 2;
                  maxVelocityY = mHeight * .9f;
               }

               int noteVelocity = 127;
               if (mGetVelocityFromClickHeight)
                  noteVelocity = (int)ofMap(y, minVelocityY, maxVelocityY, 20, 127, K(clamp));

               if (mPlayingMousePitch == -1 || !mLatch)
               {
                  PlayNote(NoteMessage(time, pitch, noteVelocity));
                  mPlayingMousePitch = pitch;
               }
               else
               {
                  bool newNote = (mPlayingMousePitch != pitch);
                  if (newNote)
                     PlayNote(NoteMessage(time, pitch, noteVelocity));
                  PlayNote(NoteMessage(time, mPlayingMousePitch, 0));
                  mPlayingMousePitch = newNote ? pitch : -1;
               }
               return;
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
      PlayNote(NoteMessage(time, mPlayingMousePitch, 0));
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

      elements = std::clamp(elements, 1, 11);

      if (mRootOctave + elements > 10) //Ensure that we can't go into octaves where it begins to break...
         mRootOctave = 11 - mNumOctaves;
      mNumOctaves = elements;
   }
   else
   {
      mNumOctaves = mForceNumOctaves;
      mRootOctave = 11 - mNumOctaves;
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
         if (i + RootKey() > 127)
            break;
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
   {
      for (int i = 0; i < NumKeys(); i += 7)
      {
         ofSetColor(108, 37, 62, 255);
         DrawTextNormal(NoteName(oct * 12, false, true), keySpace * 0.5f - 6.5f + i * keySpace, h - 8, 12);
         oct++;
      }
   }

   ofPushStyle();
   ofFill();
   ofSetLineWidth(2);
   for (int pitch = RootKey(); pitch < MIN(RootKey() + NumKeys(), mLastOnTime.size()); ++pitch)
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

bool KeyboardDisplay::ShouldConsumeKey(int key)
{
   if (mEnabled && mAllowHoverTypingInput)
   {
      key = KeyToLower(key);
      QwertyToPitchResponse res = QwertyToPitchMapping::GetPitchForComputerKey(key);
      if (res.mOctaveShift != 0)
         return true;
      if (res.mPitch != -1)
         return true;
   }
   return false;
}

void KeyboardDisplay::OnKeyPressed(int key, bool isRepeat)
{
   if (mEnabled && !isRepeat && mAllowHoverTypingInput)
   {
      key = KeyToLower(key);
      QwertyToPitchResponse res = QwertyToPitchMapping::GetPitchForComputerKey(key);

      if (res.mOctaveShift != 0)
      {
         int newRootOctave = mRootOctave + res.mOctaveShift;
         if (newRootOctave >= 0 && newRootOctave + mNumOctaves < 12)
            mRootOctave = newRootOctave;
      }
      if (res.mPitch != -1)
      {
         int pitch = res.mPitch + mRootOctave * 12;
         mKeyPressRegister[key] = pitch;
         PlayNote(NoteMessage(NextBufferTime(false), pitch, 127));
      }
   }
}

void KeyboardDisplay::KeyReleased(int key)
{
   IDrawableModule::KeyReleased(key);

   key = KeyToLower(key);
   auto it = mKeyPressRegister.find(key);
   if (it != mKeyPressRegister.end())
   {
      int pitch = it->second;

      mKeyPressRegister.erase(key);
      if (pitch != -1)
         PlayNote(NoteMessage(NextBufferTime(false), pitch, 0));
   }
}

void KeyboardDisplay::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
   {
      if (!mEnabled)
      {
         mNoteOutput.Flush(NextBufferTime(false));
         mKeyPressRegister.clear();
         for (int i = 0; i < 128; ++i)
         {
            mLastOnTime[i] = 0;
            mLastOffTime[i] = 0;
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
   mModuleSaveData.LoadBool("get_velocity_from_click_height", moduleInfo, true);
   mModuleSaveData.LoadBool("allow_hover_typing_input", moduleInfo, true);

   SetUpFromSaveData();
}

void KeyboardDisplay::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mForceNumOctaves = mModuleSaveData.GetInt("force_num_octaves");
   mLatch = mModuleSaveData.GetBool("latch");
   mShowScale = mModuleSaveData.GetBool("show_scale");
   mHideLabels = mModuleSaveData.GetBool("hide_labels");
   mGetVelocityFromClickHeight = mModuleSaveData.GetBool("get_velocity_from_click_height");
   mAllowHoverTypingInput = mModuleSaveData.GetBool("allow_hover_typing_input");

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
   if (rev >= 2)
   {
      in >> mNumOctaves;
      in >> mRootOctave;
   }
}