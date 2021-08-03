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
//  AudioRouter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/7/13.
//
//

#include "AudioRouter.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "PatchCableSource.h"

AudioRouter::AudioRouter()
: IAudioProcessor(gBufferSize)
, mRouteIndex(0)
, mRouteSelector(nullptr)
{
}

void AudioRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this,"route",5,3,&mRouteIndex);
}

AudioRouter::~AudioRouter()
{
}

void AudioRouter::AddReceiver(IAudioReceiver* receiver, const char* name)
{
   if (receiver)
   {
      mReceivers.push_back(receiver);
      mRouteSelector->AddLabel(name, (int)mReceivers.size() - 1);
      mModuleSaveData.UpdatePropertyMax("initialtarget",mReceivers.size()-1);
   }
}

void AudioRouter::Process(double time)
{
   PROFILER(AudioRouter);

   IAudioReceiver* target = GetTarget();
   if (target == nullptr)
      return;

   SyncBuffers();

   for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
   {
      Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
      GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
   }

   GetBuffer()->Reset();
}

void AudioRouter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mRouteSelector->Draw();
}

void AudioRouter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   IAudioReceiver* target = GetPatchCableSource()->GetAudioReceiver();
   if (target && !VectorContains(target, mReceivers))
      AddReceiver(target, dynamic_cast<IClickable*>(target)->Name());
   for (int i=0; i<mReceivers.size(); ++i)
   {
      if (mReceivers[i] == target)
         SetActiveIndex(i);
   }
}

void AudioRouter::GetModuleDimensions(float& width, float& height)
{
   float w,h;
   mRouteSelector->GetDimensions(w, h);
   width = 10+w;
   height = 8+h;
}

void AudioRouter::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mRouteSelector)
   {
      GetPatchCableSource()->SetTarget(dynamic_cast<IClickable*>(mReceivers[mRouteIndex]));
      TheSynth->ArrangeAudioSourceDependencies();
   }
}

void AudioRouter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   const Json::Value& targets = moduleInfo["targets"];
   for (int i=0; i<targets.size(); ++i)
   {
      string target = targets[i].asString();
      IAudioReceiver* receiver = TheSynth->FindAudioReceiver(target.c_str());
      AddReceiver(receiver, target.c_str());
   }
   
   mModuleSaveData.LoadInt("initialtarget", moduleInfo, 0, 0, targets.size()-1);
   
   SetUpFromSaveData();
}

void AudioRouter::SetUpFromSaveData()
{
   int initialTarget = mModuleSaveData.GetInt("initialtarget");
   SetActiveIndex(initialTarget);
   if (mReceivers.size() > initialTarget)
      GetPatchCableSource()->SetTarget(dynamic_cast<IClickable*>(mReceivers[initialTarget]));
}

void AudioRouter::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["targets"].resize((unsigned int)mReceivers.size());
   for (int i=0; i<mReceivers.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mReceivers[i]);
      moduleInfo["targets"][i] = module->Name();
   }
}

