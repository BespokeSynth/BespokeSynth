//
//  VocoderCarrierInput.cpp
//  modularSynth
//
//  Created by Ryan Challinor on 4/18/13.
//
//

#include "VocoderCarrierInput.h"
#include "Vocoder.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"

VocoderCarrierInput::VocoderCarrierInput()
: IAudioProcessor(gBufferSize)
, mVocoder(nullptr)
{
}

VocoderCarrierInput::~VocoderCarrierInput()
{
}

void VocoderCarrierInput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   GetPatchCableSource()->AddTypeFilter("vocoder");
   GetPatchCableSource()->AddTypeFilter("bandvocoder");
}

void VocoderCarrierInput::Process(double time)
{
   PROFILER(VocoderCarrierInput);

   if (GetTarget() == nullptr)
      return;

   if (mVocoder == nullptr)
      mVocoder = dynamic_cast<VocoderBase*>(GetTarget());

   if (mVocoder == nullptr)
      return;
   
   SyncBuffers();

   mVocoder->SetCarrierBuffer(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize());

   GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(0), GetBuffer()->BufferSize(), 0);

   GetBuffer()->Reset();
}

void VocoderCarrierInput::DrawModule()
{
}

void VocoderCarrierInput::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("vocoder",moduleInfo,"",FillDropdown<VocoderBase*>);
   
   SetUpFromSaveData();
}

void VocoderCarrierInput::SetUpFromSaveData()
{
   IDrawableModule* vocoder = TheSynth->FindModule(mModuleSaveData.GetString("vocoder"),false);
   mVocoder = dynamic_cast<VocoderBase*>(vocoder);
   GetPatchCableSource()->SetTarget(dynamic_cast<IClickable*>(vocoder));
}
