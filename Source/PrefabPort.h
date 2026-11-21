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

    PrefabPort.h
    Created: 18 Nov 2026
    Author:  Ryan Challinor

    Inlets/outlets for prefabs. These are simple passthrough modules that live
    inside a prefab and act as the patch points for the prefab's boundary. When
    the prefab is in presentation mode they are drawn compactly along the
    prefab's left (inlets) and right (outlets) edges.

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "IAudioProcessor.h"
#include "NoteEffectBase.h"
#include "IPulseReceiver.h"
#include "IModulator.h"
#include "Slider.h"

class ModuleFactory;

enum class PrefabPortType
{
   Audio,
   Note,
   Pulse,
   Modulation
};

class PrefabPort : public IDrawableModule
{
public:
   PrefabPort(PrefabPortType type, bool isInlet);

   void CreateUIControls() override;
   PrefabPortType GetPortType() const { return mPortType; }
   bool IsInlet() const { return mIsInlet; }

   //compact mode is used when the owning prefab is in presentation mode
   void SetCompact(bool compact);
   bool IsCompact() const { return mCompact; }

   bool HasTitleBar() const override { return !mCompact; }
   bool IsEnabled() const override { return true; }
   void SetEnabled(bool enabled) override {}

   static void RegisterAll(ModuleFactory* factory);
   static std::string GetTypeName(PrefabPortType type, bool isInlet);
   static std::string GetDefaultName(PrefabPortType type, bool isInlet);

protected:
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool ShouldShowTriangleMenu() const override { return !mCompact; }
   bool ShouldShowReceiveIndicators() const override { return !mCompact || mIsInlet; }
   bool ShouldForceExactDimensions() const override { return mCompact; }

   virtual void OnCompactChanged() {}
   virtual bool DrawsLabel() const { return true; }

private:
   PrefabPortType mPortType;
   bool mIsInlet;
   bool mCompact{ false };
};

class PrefabAudioPort : public PrefabPort, public IAudioProcessor
{
public:
   explicit PrefabAudioPort(bool isInlet);
   static bool AcceptsAudio() { return true; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //IAudioSource
   void Process(double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
};

class PrefabNotePort : public PrefabPort, public NoteEffectBase
{
public:
   explicit PrefabNotePort(bool isInlet);
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   //INoteReceiver
   void SendPressure(int channel, int pressure) override { mNoteOutput.SendPressure(channel, pressure); }
   void SendMidi(const juce::MidiMessage& message) override { mNoteOutput.SendMidi(message); }

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
};

class PrefabPulsePort : public PrefabPort, public IPulseSource, public IPulseReceiver
{
public:
   explicit PrefabPulsePort(bool isInlet);
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return true; }

   void CreateUIControls() override;

   //IPulseReceiver
   void OnPulse(double time, float velocity, int flags) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
};

class PrefabModulationPort : public PrefabPort, public IModulator, public IFloatSliderListener
{
public:
   explicit PrefabModulationPort(bool isInlet);
   ~PrefabModulationPort();
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;

   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return true; }
   bool CanAdjustRange() const override { return false; }

   //the patch target for incoming modulation
   FloatSlider* GetValueSlider() const { return mValueSlider; }

   //IFloatSliderListener
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

protected:
   void DrawModule() override;
   void OnCompactChanged() override;
   bool DrawsLabel() const override { return false; }

private:
   void MatchTargetRange();

   float mValue{ 0 };
   FloatSlider* mValueSlider{ nullptr };
};
