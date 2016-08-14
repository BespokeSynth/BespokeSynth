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
: mVocoder(NULL)
{
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
}

VocoderCarrierInput::~VocoderCarrierInput()
{
   delete[] mInputBuffer;
}

void VocoderCarrierInput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   GetPatchCableSource()->AddTypeFilter("vocoder");
   GetPatchCableSource()->AddTypeFilter("bandvocoder");
}

float* VocoderCarrierInput::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void VocoderCarrierInput::Process(double time)
{
   Profiler profiler("VocoderCarrierInput");

   if (GetTarget() == NULL)
      return;

   if (mVocoder == NULL)
      mVocoder = dynamic_cast<VocoderBase*>(GetTarget());

   if (mVocoder == NULL)
      return;

   mVocoder->SetCarrierBuffer(mInputBuffer, mInputBufferSize);

   GetVizBuffer()->WriteChunk(mInputBuffer, mInputBufferSize);

   Clear(mInputBuffer, mInputBufferSize);
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
