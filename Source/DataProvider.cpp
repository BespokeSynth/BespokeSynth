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
//  DataProvider.cpp
//  Bespoke
//
//  Created by Noxy Nixie on 29-05-2024.
//
//

#include "DataProvider.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "ModulationChain.h"
#include "UIControlMacros.h"
#include <chrono>
#include <VersionInfo.h>

using namespace std::chrono_literals;

DataProvider::DataProvider()
{
}

DataProvider::~DataProvider()
{
}

void DataProvider::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   DROPDOWN(mTypeSelectorDropdown, "type", &mTypeSelectorIndex, 200);
   UIBLOCK_NEWLINE();
   CHECKBOX(mRealtimeCheckbox, "realtime", &mRealtime);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mIntervalSelector, "interval", reinterpret_cast<int*>(&mInterval), 60);
   UIBLOCK_SHIFTRIGHT();
   UICONTROL_CUSTOM(mButton, new ClickButton(UICONTROL_BASICS("set")));
   ENDUIBLOCK(mWidth, mHeight);

   int interval = 0;
   mTypeSelectorDropdown->AddLabel("Transport: Measure", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Measure time", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Measure position", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Interval fraction", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Interval duration", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Steps per measure", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Swing", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Tempo", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Time signature top", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Time signature bottom", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Milliseconds per measure", interval++);
   mTypeSelectorDropdown->AddLabel("Transport: Lookahead milliseconds", interval++);
   mTypeSelectorDropdown->AddSeparator(mTypeSelectorDropdown->GetNumValues());
   mTypeSelectorDropdown->AddLabel("Audio: Sample rate", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Buffer size", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Sample rate miliseconds", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Buffer size miliseconds", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Inverted sample rate miliseconds", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Two pi over sample rate", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Input channels", interval++);
   mTypeSelectorDropdown->AddLabel("Audio: Output channels", interval++);
   mTypeSelectorDropdown->AddSeparator(mTypeSelectorDropdown->GetNumValues());
   mTypeSelectorDropdown->AddLabel("UI: Frame count", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Frame rate", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Pixel ratio", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Scale", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Corner roundness", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Zoom", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Mouse X", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Mouse Y", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Raw mouse X", interval++);
   mTypeSelectorDropdown->AddLabel("UI: Raw mouse Y", interval++);
   mTypeSelectorDropdown->AddSeparator(mTypeSelectorDropdown->GetNumValues());
   mTypeSelectorDropdown->AddLabel("Time: Since epoch", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Millisecond", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Second", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Minute", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Hour", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Day of year", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Day of month", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Day of week", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Week", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Month", interval++);
   mTypeSelectorDropdown->AddLabel("Time: Year", interval++);
   mTypeSelectorDropdown->AddSeparator(mTypeSelectorDropdown->GetNumValues());
   mTypeSelectorDropdown->AddLabel("Bespoke: All modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Note modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Synth modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Audio modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Instrument modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Processor modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Modulator modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Pulse modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Other modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Unknown modules", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Cable sources", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Cables", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Is savestate loaded", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Version major", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Version minor", interval++);
   mTypeSelectorDropdown->AddLabel("Bespoke: Version patch", interval++);


   mIntervalSelector->AddLabel("64", kInterval_64);
   mIntervalSelector->AddLabel("32", kInterval_32);
   mIntervalSelector->AddLabel("16", kInterval_16);
   mIntervalSelector->AddLabel("8", kInterval_8);
   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("2nt", kInterval_2nt);
   mIntervalSelector->AddLabel("4nd", kInterval_4nd);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8nd", kInterval_8nd);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16nd", kInterval_16nd);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);

   mControlCable = new PatchCableSource(this, kConnectionType_ValueSetter);
   AddPatchCableSource(mControlCable);
}

void DataProvider::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   mRealtimeCheckbox->Draw();
   if (!mRealtime)
      mButton->Draw();
   mTypeSelectorDropdown->Draw();

   switch (mTypeSelectorIndex)
   {
      case 3:
      case 4:
         mIntervalSelector->Draw();
         break;
      default: break;
   }
}

void DataProvider::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (size_t i = 0; i < mTargets.size(); ++i)
   {
      if (i < mControlCable->GetPatchCables().size())
      {
         mTargets[i] = dynamic_cast<IUIControl*>(mControlCable->GetPatchCables()[i]->GetTarget());
         //if (mControlCable->GetPatchCables().size() == 1 && mTargets[i] != nullptr)
         //   mValueSlider->SetExtents(mTargets[i]->GetModulationRangeMin(), mTargets[i]->GetModulationRangeMax());
      }
      else
      {
         mTargets[i] = nullptr;
      }
   }
}

void DataProvider::OnTimeEvent(double time)
{
}

void DataProvider::OnPulse(double time, float velocity, int flags)
{
   if (velocity > 0 && mEnabled && !mRealtime)
      Go(time);
}

void DataProvider::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void DataProvider::OnTransportAdvanced(float amount)
{
   Go(gTime);
}

void DataProvider::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mRealtimeCheckbox)
   {
      if (mRealtime)
      {
         TheTransport->RemoveAudioPoller(this);
         TheTransport->AddAudioPoller(this);
      }
      else
         TheTransport->RemoveAudioPoller(this);
   }
}

void DataProvider::ButtonClicked(ClickButton* button, double time)
{
   if (button == mButton && mLastClickTime != time && !mRealtime)
   {
      mLastClickTime = time;
      Go(time);
   }
}

void DataProvider::Go(double time)
{
   mControlCable->AddHistoryEvent(time, true);
   mControlCable->AddHistoryEvent(time + 15, false);

   std::vector<IDrawableModule*> modules = { nullptr };
   int counter = 0;

   switch (mTypeSelectorIndex)
   {
      //Transport
      default:
      case 0:
         mValue = TheTransport->GetMeasure(time);
         break;
      case 1:
         mValue = TheTransport->GetMeasureTime(time);
         break;
      case 2:
         mValue = TheTransport->GetMeasurePos(time);
         break;
      case 3:
         mValue = TheTransport->GetMeasureFraction(mInterval);
         break;
      case 4:
         mValue = TheTransport->GetDuration(mInterval);
         break;
      case 5:
         mValue = TheTransport->GetStepsPerMeasure(this);
         break;
      case 6:
         mValue = TheTransport->GetSwing();
         break;
      case 7:
         mValue = TheTransport->GetTempo();
         break;
      case 8:
         mValue = TheTransport->GetTimeSigTop();
         break;
      case 9:
         mValue = TheTransport->GetTimeSigBottom();
         break;
      case 10:
         mValue = TheTransport->MsPerBar();
         break;
      case 11:
         mValue = TheTransport->GetEventLookaheadMs();
         break;
      //Audio
      case 12:
         mValue = gSampleRate;
         break;
      case 13:
         mValue = gBufferSize;
         break;
      case 14:
         mValue = gSampleRateMs;
         break;
      case 15:
         mValue = gBufferSizeMs;
         break;
      case 16:
         mValue = gInvSampleRateMs;
         break;
      case 17:
         mValue = gTwoPiOverSampleRate;
         break;
      case 18:
         mValue = TheSynth->GetNumInputChannels();
         break;
      case 19:
         mValue = TheSynth->GetNumOutputChannels();
         break;
      //UI
      case 20:
         mValue = TheSynth->GetFrameCount();
         break;
      case 21:
         mValue = TheSynth->GetFrameRate();
         break;
      case 22:
         mValue = TheSynth->GetPixelRatio();
         break;
      case 23:
         mValue = TheSynth->GetUIScale();
         break;
      case 24:
         mValue = gCornerRoundness;
         break;
      case 25:
         mValue = gDrawScale;
         break;
      case 26:
         mValue = TheSynth->GetMouseX(TheSynth->GetRootContainer());
         break;
      case 27:
         mValue = TheSynth->GetMouseY(TheSynth->GetRootContainer());
         break;
      case 28:
         mValue = TheSynth->GetRawMouseX();
         break;
      case 29:
         mValue = TheSynth->GetRawMouseY();
         break;
      // Time and date
      case 30:
         //Note that we'd need at least `double` data type for sliders before this becomes more acurate/usable.
         mValue = std::chrono::system_clock::now().time_since_epoch().count();
         break;
      case 31:
         mValue = juce::Time::getCurrentTime().getMilliseconds();
         break;
      case 32:
         mValue = juce::Time::getCurrentTime().getSeconds();
         break;
      case 33:
         mValue = juce::Time::getCurrentTime().getMinutes();
         break;
      case 34:
         mValue = juce::Time::getCurrentTime().getHours();
         break;
      case 35:
         mValue = juce::Time::getCurrentTime().getDayOfYear();
         break;
      case 36:
         mValue = juce::Time::getCurrentTime().getDayOfMonth();
         break;
      case 37:
         mValue = juce::Time::getCurrentTime().getDayOfWeek();
         break;
      case 38:
         mValue = juce::Time::getCurrentTime().formatted("%U").getDoubleValue();
         break;
      case 39:
         mValue = juce::Time::getCurrentTime().getMonth();
         break;
      case 40:
         mValue = juce::Time::getCurrentTime().getYear();
         break;
      // Bespoke
      case 41:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         mValue = modules.size();
         break;
      case 42:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Note)
               counter++;
         mValue = counter;
         break;
      case 43:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Synth)
               counter++;
         mValue = counter;
         break;
      case 44:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Audio)
               counter++;
         mValue = counter;
         break;
      case 45:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Instrument)
               counter++;
         mValue = counter;
         break;
      case 46:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Processor)
               counter++;
         mValue = counter;
         break;
      case 47:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Modulator)
               counter++;
         mValue = counter;
         break;
      case 48:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Pulse)
               counter++;
         mValue = counter;
         break;
      case 49:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Other)
               counter++;
         mValue = counter;
         break;
      case 50:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr && mod->GetModuleCategory() == kModuleCategory_Unknown)
               counter++;
         mValue = counter;
         break;
      case 51:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr)
               counter += mod->GetPatchCableSources().size();
         mValue = counter;
         break;
      case 52:
         TheSynth->GetRootContainer()->GetAllModules(modules);
         for (const auto mod : modules)
            if (mod != nullptr)
               for (const auto sources : mod->GetPatchCableSources())
                  counter += sources->GetPatchCables().size();
         mValue = counter;
         break;
      case 53:
         mValue = !TheSynth->IsLoadingState();
         break;
      case 54:
         mValue = ofToDouble(ofSplitString(Bespoke::VERSION, ".")[0]);
         break;
      case 55:
         mValue = ofToDouble(ofSplitString(Bespoke::VERSION, ".")[1]);
         break;
      case 56:
         mValue = ofToDouble(ofSplitString(Bespoke::VERSION, ".")[2]);
         break;
   }

   for (const auto& mTarget : mTargets)
   {
      if (mTarget != nullptr)
         mTarget->SetValue(mValue, time, K(forceUpdate));
   }
}

void DataProvider::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void DataProvider::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void DataProvider::SetUpFromSaveData()
{
}
