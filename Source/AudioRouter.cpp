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
: mRouteIndex(0)
, mRouteSelector(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

void AudioRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this,"route",5,3,&mRouteIndex);
}

AudioRouter::~AudioRouter()
{
   delete[] mInputBuffer;
}

float* AudioRouter::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void AudioRouter::AddReceiver(IAudioReceiver* receiver, const char* name)
{
   if (receiver)
   {
      mReceivers.push_back(receiver);
      mRouteSelector->AddLabel(name, mReceivers.size() - 1);
      mModuleSaveData.UpdatePropertyMax("initialtarget",mReceivers.size()-1);
   }
}

void AudioRouter::Process(double time)
{
   Profiler profiler("AudioRouter");

   if (GetTarget() == NULL)
      return;

   int bufferSize = gBufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);

   Add(out, mInputBuffer, bufferSize);

   GetVizBuffer()->WriteChunk(mInputBuffer,bufferSize);

   Clear(mInputBuffer, mInputBufferSize);
}

void AudioRouter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;

   mRouteSelector->Draw();
}

void AudioRouter::PostRepatch(PatchCableSource* cableSource)
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

void AudioRouter::GetModuleDimensions(int& width, int& height)
{
   int w,h;
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
   
   moduleInfo["targets"].resize(mReceivers.size());
   for (int i=0; i<mReceivers.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mReceivers[i]);
      moduleInfo["targets"][i] = module->Name();
   }
}

