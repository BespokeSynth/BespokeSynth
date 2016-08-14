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

KeyboardDisplay::KeyboardDisplay()
: mWidth(500)
, mHeight(100)
, mRootOctave(3)
, mNumOctaves(3)
, mPlayingPitch(-1)
{
}

void KeyboardDisplay::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   DrawKeyboard(0,0,mWidth,mHeight);
}

void KeyboardDisplay::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   PlayNoteOutput(time, pitch, velocity, voiceIdx, pitchBend, modWheel, pressure);
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
            if (GetKeyboardKeyRect(i+RootKey(), mWidth, mHeight, isBlackKey).inside(x,y))
            {
               if ((pass == 0 && isBlackKey) || (pass == 1 && !isBlackKey))
               {
                  mPlayingPitch = i+RootKey();
                  PlayNote(gTime, mPlayingPitch, 127);
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
   if (mPlayingPitch != -1)
   {
      PlayNote(gTime, mPlayingPitch, 0);
      mPlayingPitch = -1;
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
         ofSetColor(0,255,0);
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
   ofSetColor(255,255,255);
   ofSetLineWidth(2);
   list<int> heldNotes = mNoteOutput.GetHeldNotes();
   for (int pitch : heldNotes)
   {
      bool isBlackKey;
      if (pitch >= RootKey() && pitch < RootKey() + NumKeys())
      {
         ofRectangle key = GetKeyboardKeyRect(pitch, w, h, isBlackKey);
         key.height /= 3;
         key.y += key.height*2;
         
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

void KeyboardDisplay::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("root_octave", moduleInfo, 3, 0, 10, K(isTextField));
   mModuleSaveData.LoadInt("num_octaves", moduleInfo, 3, 0, 10, K(isTextField));
   
   SetUpFromSaveData();
}

void KeyboardDisplay::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mRootOctave = mModuleSaveData.GetInt("root_octave");
   mNumOctaves = mModuleSaveData.GetInt("num_octaves");
}

