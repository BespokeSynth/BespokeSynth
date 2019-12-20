/*
  ==============================================================================

    NoteHocket.cpp
    Created: 19 Dec 2019 10:40:58pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "NoteHocket.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"

NoteHocket::NoteHocket()
: mCurrentReceiver(0)
, mRouteSelector(nullptr)
{
}

void NoteHocket::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this,"route",5,3,&mCurrentReceiver);
}

void NoteHocket::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mRouteSelector->Draw();
}

void NoteHocket::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (velocity > 0)
      SelectNewReceiver();
   PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void NoteHocket::SendCC(int control, int value, int voiceIdx)
{
   SendCCOutput(control, value, voiceIdx);
}

void NoteHocket::AddReceiver(INoteReceiver* receiver, const char* name)
{
   if (receiver)
   {
      mReceivers.push_back(receiver);
      mRouteSelector->AddLabel(name, mReceivers.size() - 1);
   }
}

void NoteHocket::SelectNewReceiver()
{
   int old = mCurrentReceiver;
   mCurrentReceiver = ofRandom(0, mReceivers.size());
   if (mCurrentReceiver != old)
      RadioButtonUpdated(mRouteSelector, old);
}

void NoteHocket::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mRouteSelector)
   {
      if (mReceivers.empty())
         return;
      
      for (auto cable : GetPatchCableSource()->GetPatchCables())
      {
         if (mReceivers[oldVal] == dynamic_cast<INoteReceiver*>(cable->GetTarget()))
         {
            mNoteOutput.FlushTarget(mReceivers[oldVal]);
            cable->Destroy();
            break;
         }
      }
      
      GetPatchCableSource()->AddPatchCable(dynamic_cast<IClickable*>(mReceivers[mCurrentReceiver]));
   }
}

void NoteHocket::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   INoteSource::PostRepatch(cableSource, fromUserClick);
   for (auto cable : GetPatchCableSource()->GetPatchCables())
   {
      INoteReceiver* target = dynamic_cast<INoteReceiver*>(cable->GetTarget());
      if (target && !VectorContains(target, mReceivers))
         AddReceiver(target, dynamic_cast<IClickable*>(target)->Name());
      
      for (size_t i=0; i<mReceivers.size(); ++i)
      {
         if (mReceivers[i] == target)
            mCurrentReceiver = i;
      }
   }
}

void NoteHocket::GetModuleDimensions(int& width, int& height)
{
   int w,h;
   mRouteSelector->GetDimensions(w, h);
   width = 10+w;
   height = 8+h;
}

void NoteHocket::LoadLayout(const ofxJSONElement& moduleInfo)
{
   const Json::Value& targets = moduleInfo["targets"];
   for (int i=0; i<targets.size(); ++i)
   {
      string target = targets[i].asString();
      INoteReceiver* receiver = TheSynth->FindNoteReceiver(target.c_str());
      AddReceiver(receiver, target.c_str());
   }
   
   SetUpFromSaveData();
}

void NoteHocket::SetUpFromSaveData()
{
}

void NoteHocket::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["targets"].resize(mReceivers.size());
   for (int i=0; i<mReceivers.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mReceivers[i]);
      moduleInfo["targets"][i] = module->Name();
   }
}
