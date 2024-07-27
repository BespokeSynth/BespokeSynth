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

static ableton::link::HostTimeFilter<ableton::link::platform::Clock> sHostTimeFilter;

AbletonLink::AbletonLink()
{
   sHostTimeFilter.reset();

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
                              if (mEnabled)
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
   FLOATSLIDER(mOffsetMsSlider, "offset ms", &mOffsetMs, -1000, 1000);
   BUTTON(mResetButton, "reset next downbeat");
   ENDUIBLOCK(mWidth, mHeight);

   mHeight = 80;
}

void AbletonLink::Poll()
{
}

void AbletonLink::OnTransportAdvanced(float amount)
{
   if (mEnabled && mLink.get() != nullptr)
   {
      if (mTempo != TheTransport->GetTempo())
      {
         mTempo = TheTransport->GetTempo();
         auto sessionState = mLink->captureAudioSessionState();
         sessionState.setTempo(mTempo, mLink->clock().micros());
         mLink->commitAudioSessionState(sessionState);
      }

      auto& deviceManager = TheSynth->GetAudioDeviceManager();
      if (deviceManager.getCurrentAudioDevice() != nullptr)
      {
         auto sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
         auto bufferSize = deviceManager.getCurrentAudioDevice()->getCurrentBufferSizeSamples();

         const auto hostTimeUs = sHostTimeFilter.sampleTimeToHostTime(mSampleTime);
         const auto outputLatencyUs = std::chrono::microseconds{ std::llround(1.0e6 * bufferSize / sampleRate) };
         auto offsetUs = std::chrono::microseconds{ llround(mOffsetMs * 1000) };

         auto adjustedTimeUs = hostTimeUs + outputLatencyUs + offsetUs;

         auto sessionState = mLink->captureAudioSessionState();

         double quantum = TheTransport->GetTimeSigTop();
         mLastReceivedBeat = sessionState.beatAtTime(adjustedTimeUs, quantum);

         double measureTime = mLastReceivedBeat / quantum;
         double localMeasureTime = TheTransport->GetMeasureTime(gTime);
         double difference = measureTime - localMeasureTime;
         if (abs(difference) > .01f)
         {
            //too far off, correct
            TheTransport->SetMeasureTime(measureTime);
            ofLog() << "correcting transport position for ableton link";
         }

         // Timeline modifications are complete, commit the results
         //mLink->commitAudioSessionState(sessionState);

         mSampleTime += gBufferSize;
      }
   }
}

void AbletonLink::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mOffsetMsSlider->Draw();
   mResetButton->Draw();

   DrawTextNormal("peers: " + ofToString(mNumPeers) + "\ntempo: " + ofToString(mTempo) + "\nbeat: " + ofToString(mLastReceivedBeat), 3, 48);
}

void AbletonLink::CheckboxUpdated(Checkbox* checkbox, double time)
{
}

void AbletonLink::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void AbletonLink::ButtonClicked(ClickButton* button, double time)
{
   if (button == mResetButton)
   {
      auto sessionState = mLink->captureAudioSessionState();
      double quantum = TheTransport->GetTimeSigTop();
      sessionState.requestBeatAtTime(0, mLink->clock().micros(), quantum);
      mLink->commitAudioSessionState(sessionState);
   }
}
