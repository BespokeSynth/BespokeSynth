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
//  EffectChain.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#include "EffectChain.h"
#include "IAudioEffect.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"

const double gSwapLength = 150.0;

EffectChain::EffectChain()
: IAudioProcessor(gBufferSize)
, mDryBuffer(gBufferSize)
, mVolume(1)
, mVolumeSlider(nullptr)
, mNumFXWide(3)
, mSpawnIndex(-1)
, mEffectSpawnList(nullptr)
, mInitialized(false)
, mSwapTime(-1)
, mShowSpawnList(true)
, mWantToDeleteEffectAtIndex(-1)
, mPush2DisplayEffect(nullptr)
{
}

EffectChain::~EffectChain()
{
   for (int i=0; i<mEffects.size(); ++i)
      delete mEffects[i];
}

void EffectChain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mVolumeSlider = new FloatSlider(this,"volume", 10, 100, 100, 15, &mVolume, 0, 2);
   mEffectSpawnList = new DropdownList(this,"effect", 10, 100, &mSpawnIndex);
   mSpawnEffectButton = new ClickButton(this,"spawn", -1, -1);
   mPush2ExitEffectButton = new ClickButton(this, "exit effect", HIDDEN_UICONTROL, HIDDEN_UICONTROL);
   mPush2ExitEffectButton->SetShowing(false);
}

void EffectChain::Init()
{
   IDrawableModule::Init();
   
   mEffectTypesToSpawn = TheSynth->GetEffectFactory()->GetSpawnableEffects();
   mEffectSpawnList->SetUnknownItemString("add effect:");
   for (int i=0; i<mEffectTypesToSpawn.size(); ++i)
      mEffectSpawnList->AddLabel(mEffectTypesToSpawn[i].c_str(), i);
   
   mInitialized = true;
}

void EffectChain::AddEffect(std::string type, bool onTheFly /*=false*/)
{
   assert(mEffects.size() < MAX_EFFECTS_IN_CHAIN - 1);
 
   IAudioEffect* effect = TheSynth->GetEffectFactory()->MakeEffect(type);
   if (effect == nullptr)
      throw UnknownEffectTypeException();
   assert(effect->GetType() == type);  //make sure things are named the same in code
   std::vector<std::string> otherEffectNames;
   for (auto* e : mEffects)
      otherEffectNames.push_back(e->Name());
   std::string name = GetUniqueName(type, otherEffectNames);
   effect->SetName(name.c_str());
   effect->SetTypeName(type);
   effect->SetParent(this);
   effect->CreateUIControls();
   if (onTheFly)
   {
      ofxJSONElement empty;
      effect->LoadLayout(empty);
      effect->SetUpFromSaveData();
   }
   
   if (mInitialized) //if we've already been initialized, call init on this
      effect->Init();
   
   mEffectMutex.lock();
   mEffects.push_back(effect);
   mEffectMutex.unlock();
   AddChild(effect);
   
   float* dryWet = &(mDryWetLevels[mEffects.size()-1]);
   *dryWet = 1;
   
   EffectControls controls;
   controls.mMoveLeftButton = new ClickButton(this, "<", 0, 0);
   controls.mMoveRightButton = new ClickButton(this, ">", 0, 0);
   controls.mDeleteButton = new ClickButton(this, "x", 0, 0);
   controls.mDryWetSlider = new FloatSlider(this, ("mix" + ofToString(mEffects.size() - 1)).c_str(), 0, 0, 60, 13, dryWet, 0, 1, 2);
   controls.mPush2DisplayEffectButton = new ClickButton(this, ("edit "+name).c_str(), 0, 0);
   controls.mPush2DisplayEffectButton->SetShowing(false);
   mEffectControls.push_back(controls);
}

void EffectChain::Process(double time)
{
   IAudioReceiver* target = GetTarget();

   if (target == nullptr)
      return;

   ComputeSliders(0);
   SyncBuffers();
   mDryBuffer.SetNumActiveChannels(GetBuffer()->NumActiveChannels());
   
   int bufferSize = GetBuffer()->BufferSize();
   
   if (mEnabled)
   {
      mEffectMutex.lock();
      
      for (int i=0; i<mEffects.size(); ++i)
      {
         mDryBuffer.CopyFrom(GetBuffer());
         
         mEffects[i]->ProcessAudio(time,GetBuffer());
       
         float* dryWetBuffer = gWorkBuffer;
         float* invDryWetBuffer = gWorkBuffer + bufferSize;
         for (int j = 0; j < bufferSize; ++j)
         {
            ComputeSliders(j);
            dryWetBuffer[j] = mDryWetLevels[i];
            invDryWetBuffer[j] = 1.0f - mDryWetLevels[i];
         }

         for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
         {
            Mult(mDryBuffer.GetChannel(ch), invDryWetBuffer, bufferSize);
            Mult(GetBuffer()->GetChannel(ch), dryWetBuffer, bufferSize);
            Add(GetBuffer()->GetChannel(ch), mDryBuffer.GetChannel(ch), bufferSize);
         }
      }
      
      mEffectMutex.unlock();
   }
   
   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      float* buffer = GetBuffer()->GetChannel(ch);
      float volSq = mVolume * mVolume;
      for (int i=0; i<bufferSize; ++i)
         buffer[i] *= volSq;
      Add(target->GetBuffer()->GetChannel(ch), buffer, bufferSize);
      GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
   }
   
   GetBuffer()->Reset();
}

void EffectChain::Poll()
{
   if (mWantToDeleteEffectAtIndex != -1)
   {
      DeleteEffect(mWantToDeleteEffectAtIndex);
      mWantToDeleteEffectAtIndex = -1;
   }
}

void EffectChain::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mEffectSpawnList->SetShowing(mShowSpawnList);
   mSpawnEffectButton->SetShowing(mShowSpawnList && mSpawnIndex != -1);
   if (mSpawnIndex != -1)
      mSpawnEffectButton->SetLabel((std::string("spawn ") + mEffectSpawnList->GetLabel(mSpawnIndex)).c_str());

   for (int i=0; i<mEffects.size(); ++i)
   {
      ofVec2f pos = GetEffectPos(i);

      if (gTime < mSwapTime)  //in swap animation
      {
         double progress = 1 - (mSwapTime - gTime)/gSwapLength;
         if (i == mSwapFromIdx)
         {
            pos.set(mSwapToPos.x * (1-progress) + pos.x * progress, 
                    mSwapToPos.y * (1-progress) + pos.y * progress);
         }
         if (i == mSwapToIdx)
         {
            pos.set(mSwapFromPos.x * (1-progress) + pos.x * progress, 
                    mSwapFromPos.y * (1-progress) + pos.y * progress);
         }
      }
      
      mEffects[i]->SetPosition(pos.x, pos.y);
      float w, h;
      mEffects[i]->GetDimensions(w,h);
      w = MAX(w,MIN_EFFECT_WIDTH);
      
      mEffectControls[i].mMoveLeftButton->SetShowing(i > 0);
      mEffectControls[i].mMoveLeftButton->SetPosition(pos.x + w / 2 - 46, pos.y - 30);
      mEffectControls[i].mMoveLeftButton->Draw();

      mEffectControls[i].mMoveRightButton->SetShowing(i < (int)mEffects.size() - 1 && !(GetKeyModifiers() & kModifier_Shift));
      mEffectControls[i].mMoveRightButton->SetPosition(pos.x + w / 2 + 35, pos.y - 30);
      mEffectControls[i].mMoveRightButton->Draw();

      mEffectControls[i].mDeleteButton->SetShowing(i == (int)mEffects.size() - 1 || (GetKeyModifiers() & kModifier_Shift));
      mEffectControls[i].mDeleteButton->SetPosition(pos.x + w / 2 + 35, pos.y - 30);
      mEffectControls[i].mDeleteButton->Draw();

      mEffectControls[i].mDryWetSlider->SetPosition(pos.x + w / 2 - 30, pos.y - 29);
      mEffectControls[i].mDryWetSlider->Draw();
   }
   
   for (int i=0; i<mEffects.size(); ++i)
   {
      mEffects[i]->Draw();
      
      float x,y,w,h;
      mEffects[i]->GetPosition(x,y,true);
      mEffects[i]->GetDimensions(w,h);
      w = MAX(w,MIN_EFFECT_WIDTH);
      
      if (mDryWetLevels[i] == 0)
      {
         ofPushStyle();
         ofFill();
         ofSetColor(0, 0, 0, 100);
         ofRect(x, y - IDrawableModule::TitleBarHeight(), w, h + IDrawableModule::TitleBarHeight());
         ofPopStyle();
      }
      
      if (i < mEffects.size() - 1)
      {
         ofPushMatrix();
         ofTranslate(x,y);
         mEffects[i]->DrawConnection(mEffects[i+1]);
         ofPopMatrix();
      }
   }

   float w,h;
   GetDimensions(w,h);
   mVolumeSlider->SetPosition(4, h-17);
   mVolumeSlider->Draw();
   mEffectSpawnList->SetPosition(106, h-17);
   mEffectSpawnList->Draw();
   mSpawnEffectButton->SetPosition(mEffectSpawnList->GetRect(true).getMaxX()+2, h-17);
   mSpawnEffectButton->Draw();
}

int EffectChain::NumRows() const
{
   return ((int)mEffects.size() + mNumFXWide - 1) / mNumFXWide;  //round up
}

int EffectChain::GetRowHeight(int row) const
{
   float max = 0;
   for (int i=0; i<mEffects.size(); ++i)
   {
      if (i/mNumFXWide == row)
      {
         float w, h;
         mEffects[i]->GetDimensions(w, h);
         h += IDrawableModule::TitleBarHeight();
         h += 20;
         max = MAX(h, max);
      }
   }
   
   return max;
}

ofVec2f EffectChain::GetEffectPos(int index) const
{
   float xPos = 10;
   float yPos = 32;
   for (int i = 0; i < mEffects.size(); ++i)
   {
      if (i > 0 && i%mNumFXWide == 0) //newline
      {
         xPos = 10;
         yPos += GetRowHeight(i / mNumFXWide - 1);
      }

      if (i == index)
         return ofVec2f(xPos, yPos);

      float w, h;
      mEffects[i]->GetDimensions(w, h);
      w = MAX(w, MIN_EFFECT_WIDTH);

      xPos += w + 20;
   }

   return ofVec2f(xPos, yPos);
}

void EffectChain::GetPush2OverrideControls(std::vector<IUIControl*>& controls) const
{
   int effectIndex = -1;
   if (mPush2DisplayEffect != nullptr)
   {
      for (int i = 0; i < (int)mEffects.size(); ++i)
      {
         if (mEffects[i] == mPush2DisplayEffect)
            effectIndex = i;
      }
   }

   if (effectIndex == -1)
   {
      controls.push_back(mVolumeSlider);
      controls.push_back(mEffectSpawnList);
      for (int i = 0; i < (int)mEffects.size(); ++i)
         controls.push_back(mEffectControls[i].mPush2DisplayEffectButton);
      if (mSpawnIndex != -1)
         controls.push_back(mSpawnEffectButton);
   }
   else
   {
      controls.push_back(mEffectControls[effectIndex].mDryWetSlider);
      controls.push_back(mPush2ExitEffectButton);
      controls.push_back(mEffectControls[effectIndex].mMoveLeftButton);
      controls.push_back(mEffectControls[effectIndex].mMoveRightButton);
      controls.push_back(mEffectControls[effectIndex].mDeleteButton);
      for (auto* control : mPush2DisplayEffect->GetUIControls())
         controls.push_back(control);
   }
}

void EffectChain::GetModuleDimensions(float& width, float& height)
{
   int maxX=100;
   if (mShowSpawnList)
      maxX += 100;
   int maxY=0;
   for (int i=0; i<mEffects.size(); ++i)
   {
      float x,y,w,h;
      mEffects[i]->GetPosition(x,y,true);
      mEffects[i]->GetDimensions(w,h);
      w = MAX(w,MIN_EFFECT_WIDTH);
      maxX = MAX(maxX,x+w);
      maxY = MAX(maxY,y+h);
   }
   width = maxX + 10;
   height = maxY + 24;
}

void EffectChain::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
   for (int i=0; i<mEffects.size(); ++i)
      mEffects[i]->KeyPressed(key, isRepeat);
}

void EffectChain::KeyReleased(int key)
{
   for (int i=0; i<mEffects.size(); ++i)
      mEffects[i]->KeyReleased(key);
}

void EffectChain::DeleteEffect(int index)
{
   assert(!mEffects.empty());
   
   {
      RemoveUIControl(mEffectControls[index].mMoveLeftButton);
      RemoveUIControl(mEffectControls[index].mMoveRightButton);
      RemoveUIControl(mEffectControls[index].mDeleteButton);
      RemoveUIControl(mEffectControls[index].mDryWetSlider);
      RemoveUIControl(mEffectControls[index].mPush2DisplayEffectButton);
      mEffectControls[index].mMoveLeftButton->Delete();
      mEffectControls[index].mMoveRightButton->Delete();
      mEffectControls[index].mDeleteButton->Delete();
      mEffectControls[index].mDryWetSlider->Delete();
      mEffectControls[index].mPush2DisplayEffectButton->Delete();
      //remove the element from mEffectControls
      int i = 0;
      for (auto iter = mEffectControls.begin(); iter != mEffectControls.end(); ++iter)
      {
         if (iter->mDeleteButton == mEffectControls[index].mDeleteButton)  //delete buttons match, we found the right one
         {
            mEffectControls.erase(iter);
            break;
         }
         ++i;
      }

      for (; i < mEffectControls.size() + 1; ++i)
         mDryWetLevels[i] = mDryWetLevels[i + 1];

      UpdateReshuffledDryWetSliders();
   }
   
   {
      mEffectMutex.lock();
      IAudioEffect* toRemove = mEffects[index];
      RemoveFromVector(toRemove, mEffects);
      RemoveChild(toRemove);
      //delete toRemove;   TODO(Ryan) can't do this in case stuff is referring to its UI controls
      mEffectMutex.unlock();
   }
}

void EffectChain::MoveEffect(int fromIndex, int direction)
{
   int newIndex = fromIndex + direction;
   if (newIndex >= 0 && newIndex < mEffects.size())
   {
      mSwapFromIdx = fromIndex;
      mSwapToIdx = newIndex;

      mEffects[mSwapFromIdx]->GetPosition(mSwapFromPos.x, mSwapFromPos.y, true);
      mEffects[mSwapToIdx]->GetPosition(mSwapToPos.x, mSwapToPos.y, true);
      mSwapTime = gTime + gSwapLength;

      mEffectMutex.lock();
      IAudioEffect* swap = mEffects[newIndex];
      mEffects[newIndex] = mEffects[fromIndex];
      mEffects[fromIndex] = swap;
      mEffectMutex.unlock();

      float level = mDryWetLevels[newIndex];
      mDryWetLevels[newIndex] = mDryWetLevels[fromIndex];
      mDryWetLevels[fromIndex] = level;

      FloatSlider* dryWetSlider = mEffectControls[newIndex].mDryWetSlider;
      mEffectControls[newIndex].mDryWetSlider = mEffectControls[fromIndex].mDryWetSlider;
      mEffectControls[fromIndex].mDryWetSlider = dryWetSlider;

      ClickButton* displayButton = mEffectControls[newIndex].mPush2DisplayEffectButton;
      mEffectControls[newIndex].mPush2DisplayEffectButton = mEffectControls[fromIndex].mPush2DisplayEffectButton;
      mEffectControls[fromIndex].mPush2DisplayEffectButton = displayButton;

      UpdateReshuffledDryWetSliders();
   }
}

void EffectChain::UpdateReshuffledDryWetSliders()
{
   for (size_t i = 0; i < mEffectControls.size(); ++i)
   {
      mEffectControls[i].mDryWetSlider->SetName(("mix" + ofToString(i)).c_str());
      mEffectControls[i].mDryWetSlider->SetVar(&mDryWetLevels[i]);
   }
}

void EffectChain::ButtonClicked(ClickButton* button)
{
   if (button == mSpawnEffectButton)
   {
      if (mSpawnIndex >= 0 && mSpawnIndex < (int)mEffectTypesToSpawn.size())
      {
         AddEffect(mEffectTypesToSpawn[mSpawnIndex], K(onTheFly));
         mSpawnIndex = -1;
      }
   }
   if (button == mPush2ExitEffectButton)
      mPush2DisplayEffect = nullptr;
   for (int i=0; i<(int)mEffectControls.size(); ++i)
   {
      if (button == mEffectControls[i].mMoveLeftButton)
         MoveEffect(i, -1);
      if (button == mEffectControls[i].mMoveRightButton)
         MoveEffect(i, 1);
      if (button == mEffectControls[i].mDeleteButton)
      {
         mWantToDeleteEffectAtIndex = i;
         return;
      }
      if (button == mEffectControls[i].mPush2DisplayEffectButton)
      {
         mPush2DisplayEffect = mEffects[i];
         mPush2ExitEffectButton->SetLabel((std::string("exit ") + mEffects[i]->Name()).c_str());
      }
   }
}

void EffectChain::CheckboxUpdated(Checkbox* checkbox)
{
}

void EffectChain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void EffectChain::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mEffectSpawnList)
   {
      if (TheSynth->GetTopModalFocusItem() == mEffectSpawnList->GetModalDropdown())
      {
         AddEffect(mEffectTypesToSpawn[mSpawnIndex], K(onTheFly));
         mSpawnIndex = -1;
      }
   }
}

std::vector<IUIControl*> EffectChain::ControlsToIgnoreInSaveState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mSpawnEffectButton);
   return ignore;
}

void EffectChain::UpdateOldControlName(std::string& oldName)
{
   IDrawableModule::UpdateOldControlName(oldName);

   if (oldName.size() > 2 && oldName[0] == 'd' && oldName[1] == 'w')
      ofStringReplace(oldName, "dw", "mix");
}

void EffectChain::LoadBasics(const ofxJSONElement& moduleInfo, std::string typeName)
{
   IDrawableModule::LoadBasics(moduleInfo, typeName);
   
   const ofxJSONElement& effects = moduleInfo["effects"];
   
   for (int i=0; i<effects.size(); ++i)
   {
      try
      {
         std::string type = effects[i]["type"].asString();
         AddEffect(type);
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }
}

void EffectChain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("widecount",moduleInfo,5,1,50,true);
   mModuleSaveData.LoadBool("showspawnlist",moduleInfo,true);
   
   const ofxJSONElement& effects = moduleInfo["effects"];
   
   assert(mEffects.size() == effects.size());
   for (int i=0; i<mEffects.size(); ++i)
   {
      try
      {
         std::string type = effects[i]["type"].asString();
         assert(mEffects[i]->GetType() == type);
         mEffects[i]->LoadLayout(effects[i]);
      }
      catch (Json::LogicError& e)
      {
         TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
      }
   }

   SetUpFromSaveData();
}

void EffectChain::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   SetWideCount(mModuleSaveData.GetInt("widecount"));
   mShowSpawnList = mModuleSaveData.GetBool("showspawnlist");
   
   for (int i=0; i<mEffects.size(); ++i)
      mEffects[i]->SetUpFromSaveData();
}

void EffectChain::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["effects"].resize((unsigned int)mEffects.size());
   for (int i=0; i<mEffects.size(); ++i)
   {
      ofxJSONElement save;
      mEffects[i]->SaveLayout(save);
      moduleInfo["effects"][i] = save;
      moduleInfo["effects"][i]["type"] = mEffects[i]->GetType();
   }
}
