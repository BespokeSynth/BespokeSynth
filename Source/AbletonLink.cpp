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
//  AbletonLink.cpp
//
//  Created by Ryan Challinor on 12/9/21.
//
//

#include "AbletonLink.h"
#include "OpenFrameworksPort.h"
#include "Transport.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

#include "juce_audio_devices/juce_audio_devices.h"

#include "ableton/Link.hpp"
#include "ableton/link/Timeline.hpp"
#include "ableton/link/HostTimeFilter.hpp"

AbletonLink::AbletonLink()
{
   mNumPeers = 0;

   mTempo = TheTransport->GetTempo();

   mLink = std::make_unique<ableton::Link>(mTempo);

   mLink->enable(true);

   mLink->setNumPeersCallback([this](std::size_t p)
   {
      mNumPeers = p;
   });

   mLink->setTempoCallback([this](const double bpm)
   {
      TheTransport->SetTempo(bpm);
      mTempo = bpm;
   });
}

void AbletonLink::Init()
{
   IDrawableModule::Init();

   TheTransport->AddAudioPoller(this);
}

AbletonLink::~AbletonLink()
{
   TheTransport->RemoveAudioPoller(this);
}

void AbletonLink::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   ENDUIBLOCK(mWidth, mHeight);

   mWidth = 150;
   mHeight = 100;
}

void AbletonLink::Poll()
{
}

void AbletonLink::OnTransportAdvanced(float amount)
{
   if (mLink.get() != nullptr)
   {
      if (mTempo != TheTransport->GetTempo())
      {
         mTempo = TheTransport->GetTempo();
         auto sessionState = mLink->captureAppSessionState();
         sessionState.setTempo(mTempo, mLink->clock().micros());
         mLink->commitAppSessionState(sessionState);
      }

      auto hostTimeMs = mLink->clock().micros();
      auto& deviceManager = TheSynth->GetAudioDeviceManager();
      if (deviceManager.getCurrentAudioDevice() != nullptr)
      {
         auto sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
         auto outputLatencySamples = deviceManager.getCurrentAudioDevice()->getOutputLatencyInSamples();
         auto bufferSize = deviceManager.getCurrentAudioDevice()->getCurrentBufferSizeSamples();

         auto outputLatencyMs = std::chrono::microseconds(llround(outputLatencySamples / sampleRate));

         outputLatencyMs += std::chrono::microseconds(llround(1.0e6 * bufferSize));

         auto adjustedTimeMs = hostTimeMs + outputLatencyMs;

         auto sessionState = mLink->captureAppSessionState();

         double quantum = TheTransport->GetTimeSigTop();
         mLastReceivedBeat = sessionState.beatAtTime(adjustedTimeMs, quantum);

         TheTransport->SetMeasureTime(mLastReceivedBeat / quantum);

         // Timeline modifications are complete, commit the results
         //mLink->commitAppSessionState(sessionState);
      }
   }
}

void AbletonLink::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   DrawTextNormal("peers: " + ofToString(mNumPeers) + "\ntempo: " + ofToString(mTempo) + "\nbeat: " + ofToString(mLastReceivedBeat), 3, 15);
}

void AbletonLink::CheckboxUpdated(Checkbox *checkbox)
{
}

