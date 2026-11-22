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
/*
  ==============================================================================

    PrefabPort.cpp
    Created: 18 Nov 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "PrefabPort.h"

#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Profiler.h"
#include "SynthGlobals.h"

PrefabPort::PrefabPort(PrefabPortType type, bool isInlet)
: mPortType(type)
, mIsInlet(isInlet)
{
}

void PrefabPort::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   if (PatchCableSource* cableSource = GetPatchCableSource())
      cableSource->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
}

void PrefabPort::SetCompact(bool compact)
{
   if (compact == mCompact)
      return;
   mCompact = compact;

   //in presentation mode an inlet's output only feeds the prefab's hidden internals, so hide it
   if (PatchCableSource* cableSource = GetPatchCableSource())
      cableSource->SetShowing(!(mIsInlet && mCompact));

   OnCompactChanged();
}

void PrefabPort::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!DrawsLabel())
      return;

   if (mCompact)
      DrawTextNormal(std::string(Name()), 4, 12);
}

void PrefabPort::GetModuleDimensions(float& width, float& height)
{
   if (mPortType == PrefabPortType::Modulation)
   {
      width = 86;
      height = 30;
   }
   else
   {
      width = GetStringWidth(Name()) + 8;
      height = 17;
   }
}

//static
std::string PrefabPort::GetTypeName(PrefabPortType type, bool isInlet)
{
   return "prefab" + GetDefaultName(type, isInlet);
}

//static
std::string PrefabPort::GetDefaultName(PrefabPortType type, bool isInlet)
{
   std::string base;
   switch (type)
   {
      case PrefabPortType::Audio: base = "audio"; break;
      case PrefabPortType::Note: base = "note"; break;
      case PrefabPortType::Pulse: base = "pulse"; break;
      case PrefabPortType::Modulation: base = "mod"; break;
   }
   return base + (isInlet ? "in" : "out");
}

/////////////////////////////////////////////////////////////////////////////
// audio

PrefabAudioPort::PrefabAudioPort(bool isInlet)
: PrefabPort(PrefabPortType::Audio, isInlet)
, IAudioProcessor(gBufferSize)
{
}

void PrefabAudioPort::CreateUIControls()
{
   PrefabPort::CreateUIControls();
}

void PrefabAudioPort::Process(double time)
{
   PROFILER(PrefabAudioPort);

   SyncBuffers();

   IAudioReceiver* target = GetTarget();
   if (target)
   {
      ChannelBuffer* out = target->GetBuffer();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         Add(out->GetChannel(ch), GetBuffer()->GetChannel(ch), out->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Reset();
}

void PrefabAudioPort::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void PrefabAudioPort::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

/////////////////////////////////////////////////////////////////////////////
// note

PrefabNotePort::PrefabNotePort(bool isInlet)
: PrefabPort(PrefabPortType::Note, isInlet)
{
}

void PrefabNotePort::CreateUIControls()
{
   PrefabPort::CreateUIControls();
}

void PrefabNotePort::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void PrefabNotePort::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

/////////////////////////////////////////////////////////////////////////////
// pulse

PrefabPulsePort::PrefabPulsePort(bool isInlet)
: PrefabPort(PrefabPortType::Pulse, isInlet)
{
}

void PrefabPulsePort::CreateUIControls()
{
   PrefabPort::CreateUIControls();
}

void PrefabPulsePort::OnPulse(double time, float velocity, int flags)
{
   DispatchPulse(GetPatchCableSource(), time, velocity, flags);
}

void PrefabPulsePort::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void PrefabPulsePort::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

/////////////////////////////////////////////////////////////////////////////
// modulation

PrefabModulationPort::PrefabModulationPort(bool isInlet)
: PrefabPort(PrefabPortType::Modulation, isInlet)
{
}

PrefabModulationPort::~PrefabModulationPort()
{
}

void PrefabModulationPort::CreateUIControls()
{
   PrefabPort::CreateUIControls();

   //the slider is the patch target for incoming modulation, and its value is what we output
   mValueSlider = new FloatSlider(this, "value", 3, 1, 80, 15, &mValue, 0, 1);
   mValueSlider->SetShowName(false);

   mTargetCableSource = new PatchCableSource(this, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   AddPatchCableSource(mTargetCableSource);
}

void PrefabModulationPort::OnCompactChanged()
{
   //when compact there's no title bar, so make room to draw our name above the slider
   if (mValueSlider)
      mValueSlider->SetPosition(3, IsCompact() ? 14 : 1);
}

void PrefabModulationPort::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (IsCompact())
      DrawTextNormal(std::string(Name()), 4, 11);

   mValueSlider->Draw();
}

void PrefabModulationPort::MatchTargetRange()
{
   FloatSlider* target = GetSliderTarget();
   if (target == nullptr)
      return;
   if (mValueSlider->GetMin() != target->GetMin() || mValueSlider->GetMax() != target->GetMax())
      mValueSlider->SetExtents(target->GetMin(), target->GetMax());
   if (mValueSlider->GetMode() != target->GetMode())
      mValueSlider->SetMode(target->GetMode());
}

void PrefabModulationPort::Poll()
{
   //keep the input range mirroring whatever we're driving, so upstream modulators see a meaningful range
   MatchTargetRange();
}

void PrefabModulationPort::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   OnModulatorRepatch();

   if (GetSliderTarget() && fromUserClick)
      mValue = GetSliderTarget()->GetValue();
   MatchTargetRange();
}

float PrefabModulationPort::Value(int samplesIn)
{
   ComputeSliders(samplesIn);
   return ofClamp(mValue, GetMin(), GetMax());
}

void PrefabModulationPort::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void PrefabModulationPort::SetUpFromSaveData()
{
}
