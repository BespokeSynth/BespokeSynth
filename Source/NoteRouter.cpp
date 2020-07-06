//
//  NoteRouter.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 3/24/13.
//
//

#include "NoteRouter.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "PatchCable.h"
#include "PatchCableSource.h"

NoteRouter::NoteRouter()
: mRouteMask(0)
, mRouteSelector(nullptr)
, mRadioButtonMode(false)
{
}

void NoteRouter::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mRouteSelector = new RadioButton(this,"route",5,3,&mRouteMask);
   mRouteSelector->SetMultiSelect(!mRadioButtonMode);
}

void NoteRouter::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   mRouteSelector->Draw();
}

void NoteRouter::DrawModuleUnclipped()
{
   for (int i=0; i<mReceivers.size(); ++i)
      DrawConnection(dynamic_cast<IClickable*>(mReceivers[i]));
}

void NoteRouter::AddReceiver(INoteReceiver* receiver, const char* name)
{
   if (receiver)
   {
      mReceivers.push_back(receiver);
      mRouteSelector->AddLabel(name, (int)mReceivers.size() - 1);
      mModuleSaveData.UpdatePropertyMax("selectedmask",(1 << mReceivers.size())-1);
   }
}

void NoteRouter::SetSelectedMask(int mask)
{
   int oldMask = mRouteMask;
   mRouteMask = mask;
   RadioButtonUpdated(mRouteSelector, oldMask);
}

void NoteRouter::RadioButtonUpdated(RadioButton* radio, int oldVal)
{
   if (radio == mRouteSelector)
   {
      if (mReceivers.empty())
         return;
      
      if (mRadioButtonMode)
      {
         for (auto cable : GetPatchCableSource()->GetPatchCables())
         {
            if (mReceivers[oldVal] == dynamic_cast<INoteReceiver*>(cable->GetTarget()))
            {
               mNoteOutput.FlushTarget(gTime, mReceivers[oldVal]);
               cable->Destroy();
               break;
            }
         }
         
         GetPatchCableSource()->AddPatchCable(dynamic_cast<IClickable*>(mReceivers[mRouteMask]));
      }
      else //normal bitmask mode
      {
         int changed = mRouteMask ^ oldVal;
         int added = changed & mRouteMask;
         int removed = changed & oldVal;
         
         for (int i=0; i<=int(log2(added)); ++i)
         {
            if (1 & (added >> i))
            {
               GetPatchCableSource()->AddPatchCable(dynamic_cast<IClickable*>(mReceivers[i]));
            }
         }
         for (int i=0; i<=int(log2(removed)); ++i)
         {
            if (1 & (removed >> i))
            {
               for (auto cable : GetPatchCableSource()->GetPatchCables())
               {
                  if (mReceivers[i] == dynamic_cast<INoteReceiver*>(cable->GetTarget()))
                  {
                     mNoteOutput.FlushTarget(gTime, mReceivers[i]);
                     cable->Destroy();
                     break;
                  }
               }
            }
         }
      }
   }
}

void NoteRouter::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   INoteSource::PostRepatch(cableSource, fromUserClick);
   for (auto cable : GetPatchCableSource()->GetPatchCables())
   {
      INoteReceiver* target = dynamic_cast<INoteReceiver*>(cable->GetTarget());
      if (target && !VectorContains(target, mReceivers))
         AddReceiver(target, dynamic_cast<IClickable*>(target)->Name());
   }
}

void NoteRouter::GetModuleDimensions(float& width, float& height)
{
   float w,h;
   mRouteSelector->GetDimensions(w, h);
   width = 10+w;
   height = 8+h;
}

void NoteRouter::LoadLayout(const ofxJSONElement& moduleInfo)
{
   const Json::Value& targets = moduleInfo["targets"];
   for (int i=0; i<targets.size(); ++i)
   {
      string target = targets[i].asString();
      INoteReceiver* receiver = TheSynth->FindNoteReceiver(target.c_str());
      AddReceiver(receiver, target.c_str());
   }
   
   mModuleSaveData.LoadInt("selectedmask",moduleInfo,1,0,(1 << targets.size())-1);
   mModuleSaveData.LoadBool("radiobuttonmode", moduleInfo, true);
   
   SetUpFromSaveData();
}

void NoteRouter::SetUpFromSaveData()
{
   mRadioButtonMode = mModuleSaveData.GetBool("radiobuttonmode");
   mRouteSelector->SetMultiSelect(!mRadioButtonMode);
   SetSelectedMask(mModuleSaveData.GetInt("selectedmask"));
}

void NoteRouter::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   
   moduleInfo["targets"].resize((unsigned int)mReceivers.size());
   for (int i=0; i<mReceivers.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mReceivers[i]);
      moduleInfo["targets"][i] = module->Name();
   }
}


