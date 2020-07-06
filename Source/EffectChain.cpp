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
, mDeleteLastEffectButton(nullptr)
, mShowSpawnList(true)
, mWantDeleteLastEffect(false)
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
   mDeleteLastEffectButton = new ClickButton(this,"x", 10, 100);
}

void EffectChain::Init()
{
   IDrawableModule::Init();
   
   mEffectTypesToSpawn = TheSynth->GetEffectFactory()->GetSpawnableEffects();
   mEffectSpawnList->SetUnknownItemString("add effect:");
   for (int i=0; i<mEffectTypesToSpawn.size(); ++i)
      mEffectSpawnList->AddLabel(mEffectTypesToSpawn[i].c_str(), i);
   mEffectSpawnList->SetNoHover(true);
   
   mInitialized = true;
}

void EffectChain::AddEffect(string type, bool onTheFly /*=false*/)
{
   assert(mEffects.size() < MAX_EFFECTS_IN_CHAIN - 1);
 
   IAudioEffect* effect = TheSynth->GetEffectFactory()->MakeEffect(type);
   if (effect == nullptr)
      throw UnknownEffectTypeException();
   assert(effect->GetType() == type);  //make sure things are named the same in code
   string name = GetUniqueName(type, vector<IDrawableModule*>(mEffects.begin(), mEffects.end()));
   effect->SetName(name.c_str());
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
   mDryWetSliders.push_back(new FloatSlider(this,("dw"+ofToString(mEffects.size()-1)).c_str(),0,0,60,13,dryWet,0,1,2));
   
   if (mEffects.size() > 1)
   {
      mMoveButtons.push_back(new ClickButton(this,">",0,0));
      mMoveButtons.push_back(new ClickButton(this,"<",0,0));
   }
}

void EffectChain::Process(double time)
{
   if (GetTarget() == nullptr)
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
      Add(GetTarget()->GetBuffer()->GetChannel(ch), buffer, bufferSize);
      GetVizBuffer()->WriteChunk(buffer, bufferSize, ch);
   }
   
   GetBuffer()->Reset();
}

void EffectChain::Poll()
{
   if (mWantDeleteLastEffect)
   {
      DeleteLastEffect();
      mWantDeleteLastEffect = false;
   }
}

void EffectChain::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mEffectSpawnList->SetShowing(mShowSpawnList);

   float xPos = 10;
   float yPos = 32;
   for (int i=0; i<mEffects.size(); ++i)
   {
      if (i > 0 && i%mNumFXWide == 0) //newline
      {
         xPos = 10;
         yPos += GetRowHeight(i/mNumFXWide - 1);
      }

      float w,h;
      float thisX = xPos;
      float thisY = yPos;
      if (gTime < mSwapTime)  //in swap animation
      {
         double progress = 1 - (mSwapTime - gTime)/gSwapLength;
         if (i == mSwapFromIdx)
         {
            thisX = int(mSwapToPos.x * (1-progress) + mSwapFromPos.x * progress);
            thisY = int(mSwapToPos.y * (1-progress) + mSwapFromPos.y * progress);
         }
         if (i == mSwapToIdx)
         {
            thisX = int(mSwapFromPos.x * (1-progress) + mSwapToPos.x * progress);
            thisY = int(mSwapFromPos.y * (1-progress) + mSwapToPos.y * progress);
         }
      }
      
      mEffects[i]->SetPosition(thisX,thisY);
      mEffects[i]->GetDimensions(w,h);
      w = MAX(w,MIN_EFFECT_WIDTH);
      
      int leftButtonIdx = i*2-1;
      if (leftButtonIdx >= 0 && leftButtonIdx < mMoveButtons.size())
      {
         mMoveButtons[leftButtonIdx]->SetPosition(thisX + w/2 - 46, thisY-30);
         mMoveButtons[leftButtonIdx]->Draw();
      }
      int rightButtonIdx = i*2;
      if (rightButtonIdx >= 0 && rightButtonIdx < mMoveButtons.size())
      {
         mMoveButtons[rightButtonIdx]->SetPosition(thisX + w/2 + 35, thisY-30);
         mMoveButtons[rightButtonIdx]->Draw();
      }
      
      if (i == mEffects.size()-1)
      {
         mDeleteLastEffectButton->SetPosition(thisX + w/2 + 35, thisY-30);
         mDeleteLastEffectButton->Draw();
      }
      
      mDryWetSliders[i]->SetPosition(thisX + w/2 - 30, thisY-29);
      mDryWetSliders[i]->Draw();
      
      xPos += w+20;
   }
   
   for (int i=0; i<mEffects.size(); ++i)
   {
      mEffects[i]->Draw();
      
      float x,y,w,h;
      mEffects[i]->GetPosition(x,y,true);
      mEffects[i]->GetDimensions(w,h);
      w = MAX(w,MIN_EFFECT_WIDTH);
      
      ofPushStyle();
      ofNoFill();
      ofSetLineWidth(2);
      ofSetColor(0,255,255,gModuleDrawAlpha*mEffects[i]->GetEffectAmount()*mDryWetLevels[i]);
      ofRect(x,y-IDrawableModule::TitleBarHeight(),w,h+IDrawableModule::TitleBarHeight());
      ofPopStyle();
      
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
   mVolumeSlider->SetPosition(4, h-15);
   mVolumeSlider->Draw();
   mEffectSpawnList->SetPosition(106, h-15);
   mEffectSpawnList->Draw();
}

int EffectChain::NumRows() const
{
   return ((int)mEffects.size() + mNumFXWide - 1) / mNumFXWide;  //round up
}

int EffectChain::GetRowHeight(int row)
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
   height = maxY + 20;
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

void EffectChain::DeleteLastEffect()
{
   assert(!mEffects.empty());
   
   if (mEffects.size() > 1)
   {
      RemoveUIControl(mMoveButtons[mMoveButtons.size()-1]);
      RemoveUIControl(mMoveButtons[mMoveButtons.size()-2]);
      mMoveButtons[mMoveButtons.size()-1]->Delete();
      mMoveButtons[mMoveButtons.size()-2]->Delete();
      mMoveButtons.resize(mMoveButtons.size() - 2);
   }
   RemoveUIControl(mDryWetSliders[mDryWetSliders.size()-1]);
   mDryWetSliders[mDryWetSliders.size()-1]->Delete();
   mDryWetSliders.resize(mDryWetSliders.size()-1);
   
   mEffectMutex.lock();
   IAudioEffect* toRemove = mEffects[mEffects.size()-1];
   RemoveFromVector(toRemove, mEffects);
   RemoveChild(toRemove);
   //delete toRemove;   TODO(Ryan) can't do this in case stuff is referring to its UI controls
   mEffectMutex.unlock();
}

void EffectChain::ButtonClicked(ClickButton* button)
{
   for (int i=0; i<mMoveButtons.size(); ++i)
   {
      if (mMoveButtons[i] == button)
      {
         int effectIndex = (i+1)/2;
         bool left = i%2==1;
         int newIndex = left ? effectIndex-1 : effectIndex+1;
         assert(newIndex>=0 && newIndex < mEffects.size());

         mSwapFromIdx = effectIndex;
         mSwapToIdx = newIndex;

         mEffects[mSwapFromIdx]->GetPosition(mSwapFromPos.x,mSwapFromPos.y,true);
         mEffects[mSwapToIdx]->GetPosition(mSwapToPos.x,mSwapToPos.y,true);
         mSwapTime = gTime + gSwapLength;

         mEffectMutex.lock();
         IAudioEffect* swap = mEffects[newIndex];
         mEffects[newIndex] = mEffects[effectIndex];
         mEffects[effectIndex] = swap;
         mEffectMutex.unlock();
         
         float level = mDryWetLevels[newIndex];
         mDryWetLevels[newIndex] = mDryWetLevels[effectIndex];
         mDryWetLevels[effectIndex] = level;
      }
   }
   if (button == mDeleteLastEffectButton)
   {
      mWantDeleteLastEffect = true;
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
      AddEffect(mEffectTypesToSpawn[mSpawnIndex], K(onTheFly));
      mSpawnIndex = -1;
   }
}

void EffectChain::LoadBasics(const ofxJSONElement& moduleInfo, string typeName)
{
   IDrawableModule::LoadBasics(moduleInfo, typeName);
   
   const ofxJSONElement& effects = moduleInfo["effects"];
   
   for (int i=0; i<effects.size(); ++i)
   {
      string type = effects[i]["type"].asString();
      AddEffect(type);
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
      string type = effects[i]["type"].asString();
      assert(mEffects[i]->GetType() == type);
      mEffects[i]->LoadLayout(effects[i]);
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
