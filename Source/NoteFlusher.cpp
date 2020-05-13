//
//  NoteFlusher.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/23/14.
//
//

#include "NoteFlusher.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

NoteFlusher::NoteFlusher()
: mFlushButton(nullptr)
{
}

void NoteFlusher::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mFlushButton = new ClickButton(this,"flush",5,2);
}

void NoteFlusher::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mFlushButton->Draw();
}

void NoteFlusher::ButtonClicked(ClickButton* button)
{
   if (button == mFlushButton)
   {
      mNoteOutput.Flush(gTime);
      for (int i=0; i<127; ++i)
         mNoteOutput.PlayNote(gTime, i, 0);
   }
}

void NoteFlusher::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void NoteFlusher::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}


