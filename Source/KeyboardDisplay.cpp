//
//  KeyboardDisplay.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/12/16.
//
//

#include "KeyboardDisplay.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "ModuleContainer.h"

namespace
{
   const float kKeyboardYOffset = 0;
}

KeyboardDisplay::KeyboardDisplay()
: mWidth(500)
, mHeight(110)
, mRootOctave(3)
, mNumOctaves(3)
, mPlayingMousePitch(-1)
, mTypingInput(false)
{
   SetIsNoteOrigin(true);
   
   for (int i=0; i<128; ++i)
      mLastPlayedTime[i] = 0;
}

void KeyboardDisplay::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
}

void KeyboardDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   DrawKeyboard(0,kKeyboardYOffset,mWidth,mHeight-kKeyboardYOffset);
}

void KeyboardDisplay::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
   
   if (velocity > 0)
   {
      assert(pitch < 128);
      mLastPlayedTime[pitch] = gTime;
   }
}

void KeyboardDisplay::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   for (int i=0; i<NumKeys(); ++i)
   {
      for (int pass=0; pass<2; ++pass)
      {
         for (int i=0;i<NumKeys();++i)
         {
            bool isBlackKey;
            if (GetKeyboardKeyRect(i+RootKey(), mWidth, mHeight - kKeyboardYOffset, isBlackKey).contains(x,y - kKeyboardYOffset))
            {
               if ((pass == 0 && isBlackKey) || (pass == 1 && !isBlackKey))
               {
                  int pitch = i+RootKey();
                  if (mPlayingMousePitch == -1 || !mLatch)
                  {
                     PlayNote(gTime, pitch, 127);
                     mPlayingMousePitch = pitch;
                  }
                  else
                  {
                     bool newNote = (mPlayingMousePitch != pitch);
                     if (newNote)
                        PlayNote(gTime, pitch, 127);
                     PlayNote(gTime, mPlayingMousePitch, 0);
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
      PlayNote(gTime, mPlayingMousePitch, 0);
      mPlayingMousePitch = -1;
   }
}

int KeyboardDisplay::RootKey() const
{
   return TheScale->GetTet() * mRootOctave;
}

int KeyboardDisplay::NumKeys() const
{
   return TheScale->GetTet() * mNumOctaves + 1;
}

namespace
{
   void SetPitchColor(int pitch)
   {
      if (TheScale->IsRoot(pitch))
         ofSetColor(0,200,0);
      else if (TheScale->IsInPentatonic(pitch))
         ofSetColor(255,128,0);
      else if (TheScale->IsInScale(pitch))
         ofSetColor(180,80,0);
      else
         ofSetColor(50,50,50);
   }
}

void KeyboardDisplay::DrawKeyboard(int x, int y, int w, int h)
{
   ofPushStyle();
   ofPushMatrix();
   ofTranslate(x, y);
   
   for (int pass=0; pass<2; ++pass)
   {
      for (int i=0;i<NumKeys();++i)
      {
         bool isBlackKey;
         ofRectangle key = GetKeyboardKeyRect(i + RootKey(), w, h, isBlackKey);
         
         if ((pass == 0 && !isBlackKey) || (pass == 1 && isBlackKey))
         {
            SetPitchColor(i);
            ofFill();
            ofRect(key);
            ofSetColor(0,0,0);
            ofNoFill();
            ofRect(key);
         }
      }
   }
   
   ofPushStyle();
   ofFill();
   ofSetLineWidth(2);
   list<int> heldNotes = mNoteOutput.GetHeldNotesList();
   for (int pitch : heldNotes)
   {
      bool isBlackKey;
      if (pitch >= RootKey() && pitch < RootKey() + NumKeys())
      {
         ofRectangle key = GetKeyboardKeyRect(pitch, w, h, isBlackKey);
         key.height /= 3;
         key.y += key.height*2;
         
         ofSetColor(255,255,255,ofLerp(255, 150, ofClamp((gTime - mLastPlayedTime[pitch]) / 150.0f, 0, 1)));
         
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
   
   float offset = pitch/TheScale->GetTet() * (octaveWidth);
   pitch %= 12;
   
   if ((pitch<=4&&pitch%2==0) || (pitch>=5&&pitch%2==1)) //white key
   {
      int whiteKey = (pitch+1)/2;
      isBlackKey = false;
      return ofRectangle(offset+whiteKey*octaveWidth/7,0,octaveWidth/7,h);
   }
   else //black key
   {
      int blackKey = pitch/2;
      isBlackKey = true;
      return ofRectangle(offset+blackKey*octaveWidth/7+octaveWidth/16+octaveWidth/7*.1f,0,octaveWidth/7*.8f,h/2);
   }
}

int KeyboardDisplay::GetPitchForTypingKey(int key) const
{
   int index = -1;
   
   if (key == 'a')
      index = 0;
   if (key == 'w')
      index = 1;
   if (key == 's')
      index = 2;
   if (key == 'e')
      index = 3;
   if (key == 'd')
      index = 4;
   if (key == 'f')
      index = 5;
   if (key == 't')
      index = 6;
   if (key == 'g')
      index = 7;
   if (key == 'y')
      index = 8;
   if (key == 'h')
      index = 9;
   if (key == 'u')
      index = 10;
   if (key == 'j')
      index = 11;
   if (key == 'k')
      index = 12;
   if (key == 'o')
      index = 13;
   if (key == 'l')
      index = 14;
   if (key == 'p')
      index = 15;
   if (key == ';')
      index = 16;
   
   if (index != -1)
      return mRootOctave*12+index;
   return -1;
}

void KeyboardDisplay::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   
   if (mTypingInput && mEnabled && !isRepeat)
   {
      int pitch = GetPitchForTypingKey(key);
      if (pitch != -1)
         PlayNote(gTime, pitch, 127);
   }
}

void KeyboardDisplay::KeyReleased(int key)
{
   if (mTypingInput && mEnabled)
   {
      int pitch = GetPitchForTypingKey(key);
      if (pitch != -1)
         PlayNote(gTime, pitch, 0);
   }
}

void KeyboardDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("root_octave", moduleInfo, 3, 0, 10, K(isTextField));
   mModuleSaveData.LoadInt("num_octaves", moduleInfo, 3, 0, 10, K(isTextField));
   mModuleSaveData.LoadBool("typing_control", moduleInfo, false);
   mModuleSaveData.LoadBool("latch", moduleInfo, false);
   
   SetUpFromSaveData();
}

void KeyboardDisplay::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mRootOctave = mModuleSaveData.GetInt("root_octave");
   mNumOctaves = mModuleSaveData.GetInt("num_octaves");
   mTypingInput = mModuleSaveData.GetBool("typing_control");
   mLatch = mModuleSaveData.GetBool("latch");
}

namespace
{
   const int kSaveStateRev = 1;
}

void KeyboardDisplay::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   out << mWidth;
   out << mHeight;
}

void KeyboardDisplay::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   if (!ModuleContainer::DoesModuleHaveMoreSaveData(in))
      return;  //this was saved before we added versioning, bail out
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   in >> mWidth;
   in >> mHeight;
}


