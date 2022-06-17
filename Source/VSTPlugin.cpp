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
//  VSTPlugin.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/18/16.
//
//

#include "VSTPlugin.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"
#include "ModulationChain.h"
#include "PatchCableSource.h"
//#include "NSWindowOverlay.h"

namespace
{
   const int kGlobalModulationIdx = 16;
   juce::String GetFileNameWithoutExtension(const juce::String& fullPath)
   {
      auto lastSlash = fullPath.lastIndexOfChar('/') + 1;
      if (lastSlash == 0)
         lastSlash = fullPath.lastIndexOfChar('\\') + 1;
      auto lastDot = fullPath.lastIndexOfChar('.');

      if (lastDot > lastSlash)
         return fullPath.substring(lastSlash, lastDot);

      return fullPath.substring(lastSlash);
   }
}

using namespace juce;

namespace VSTLookup
{
   void GetAvailableVSTs(std::vector<PluginDescription>& vsts)
   {
      static bool sFirstTime = true;
      if (sFirstTime)
      {
         auto file = juce::File(ofToDataPath("vst/found_vsts.xml"));
         if (file.existsAsFile())
         {
            auto xml = juce::parseXML(file);
            TheSynth->GetKnownPluginList().recreateFromXml(*xml);
         }
      }
      
      TheSynth->GetKnownPluginList().sort(KnownPluginList::SortMethod::sortAlphabetically, true);
      auto types = TheSynth->GetKnownPluginList().getTypes();
      for (int i = 0; i < types.size(); ++i)
         vsts.push_back(types[i]);

      //for (int i = 0; i < 2000; ++i)
      //   vsts.insert(vsts.begin(), std::string("c:/a+") + ofToString(gRandom()));

      //SortByLastUsed(vsts);

      //add a bunch of duplicates to the list, to simulate a user with many VSTs
      /*auto vstCopy = vsts;
      for (int i = 0; i < 40; ++i)
         vsts.insert(vsts.end(), vstCopy.begin(), vstCopy.end());*/

      sFirstTime = false;
   }

   void FillVSTList(DropdownList* list)
   {
      assert(list);
      std::vector<PluginDescription> vsts;
      GetAvailableVSTs(vsts);
      for (int i = 0; i < vsts.size(); ++i)
         list->AddLabel(vsts[i].name.toStdString(), vsts[i].uniqueId);
   }

   std::string GetVSTPath(std::string vstName)
   {
      if (juce::String(vstName).contains("/") || juce::String(vstName).contains("\\")) //already a path
         return vstName;

      vstName = GetFileNameWithoutExtension(vstName).toStdString();
      auto types = TheSynth->GetKnownPluginList().getTypes();
      for (int i = 0; i < types.size(); ++i)
      {
         juce::File vst(types[i].fileOrIdentifier);
         if (vst.getFileNameWithoutExtension().toStdString() == vstName)
            return types[i].fileOrIdentifier.toStdString();
      }

      return "";
   }

   juce::PluginDescription GetPluginDesc(int id)
   {
      juce::PluginDescription desc;
      auto types = TheSynth->GetKnownPluginList().getTypes();
      for (int i = 0; i < types.size(); ++i)
      {
         if (id == types[i].uniqueId)
         {
            desc = types[i];
            break;
         }
      }
      return desc;
   }
   
   juce::PluginDescription GetPluginDesc(std::string vstName)
   {
      juce::PluginDescription desc;
      auto types = TheSynth->GetKnownPluginList().getTypes();
      for (int i = 0; i < types.size(); ++i)
      {
         if (vstName == types[i].name)
         {
            desc = types[i];
            break;
         }
      }
      return desc;
   }   

   juce::PluginDescription GetPluginDesc(juce::String pluginId)
   {
      juce::PluginDescription desc;
      auto types = TheSynth->GetKnownPluginList().getTypes();
      for (int i = 0; i < types.size(); ++i)
      {
         if (types[i].createIdentifierString() == pluginId)
         {
            desc = types[i];
            break;
         }
      }
      return desc;
   }


   std::vector<juce::PluginDescription> GetRecentPlugins(int num)
   {
      std::vector<juce::PluginDescription> recentPlugins;
      std::map<double, std::string> lastUsedTimes;

      if (juce::File(ofToDataPath("vst/used_vsts.json")).existsAsFile())
      {
         ofxJSONElement root;
         root.open(ofToDataPath("vst/used_vsts.json"));
         ofxJSONElement jsonList = root["vsts"];

         for (auto it = jsonList.begin(); it != jsonList.end(); ++it)
         {
            try
            {
                std::string name = it.key().asString();
                double time = jsonList[name].asDouble();
                lastUsedTimes.insert(std::make_pair(time, name));
            }
            catch (Json::LogicError& e)
            {
               TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
            }
         }
      }
      std::map<double, std::string>::const_iterator it;
      it = lastUsedTimes.begin();
      while ( it != lastUsedTimes.end())
      {
          //DBG(it->second);
          recentPlugins.push_back(GetPluginDesc(juce::String(it->second)));
          ++it;
      }

      return recentPlugins;
   }

   void SortByLastUsed(std::vector<std::string>& vsts)
   {
      std::map<std::string, double> lastUsedTimes;

      if (juce::File(ofToDataPath("vst/used_vsts.json")).existsAsFile())
      {
         ofxJSONElement root;
         root.open(ofToDataPath("vst/used_vsts.json"));
         ofxJSONElement jsonList = root["vsts"];

         for (auto it = jsonList.begin(); it != jsonList.end(); ++it)
         {
            try
            {
               std::string key = it.key().asString();
               lastUsedTimes[key] = jsonList[key].asDouble();
            }
            catch (Json::LogicError& e)
            {
               TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
            }
         }
      }

      std::sort(vsts.begin(), vsts.end(), [lastUsedTimes](std::string a, std::string b)
                {
                   auto itA = lastUsedTimes.find(a);
                   auto itB = lastUsedTimes.find(b);
                   double timeA = 0;
                   double timeB = 0;
                   if (itA != lastUsedTimes.end())
                      timeA = (*itA).second;
                   if (itB != lastUsedTimes.end())
                      timeB = (*itB).second;

                   if (timeA == timeB)
                      return a < b;

                   return timeA > timeB;
                });
   }
}

VSTPlugin::VSTPlugin()
: IAudioProcessor(gBufferSize)
{
   juce::File(ofToDataPath("vst")).createDirectory();
   juce::File(ofToDataPath("vst/presets")).createDirectory();

   mChannelModulations.resize(kGlobalModulationIdx + 1);

   mPluginName = "no plugin loaded";
}

void VSTPlugin::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this, "vol", 3, 3, 80, 15, &mVol, 0, 4);
   mOpenEditorButton = new ClickButton(this, "open", mVolSlider, kAnchor_Right_Padded);
   mPresetFileSelector = new DropdownList(this, "preset", 3, 21, &mPresetFileIndex, 110);
   mSavePresetFileButton = new ClickButton(this, "save as", -1, -1);
   mShowParameterDropdown = new DropdownList(this, "show parameter", 3, 38, &mShowParameterIndex, 160);
   mPanicButton = new ClickButton(this, "panic", 166, 38);

   mPresetFileSelector->DrawLabel(true);
   mSavePresetFileButton->PositionTo(mPresetFileSelector, kAnchor_Right);

   mMidiOutCable = new AdditionalNoteCable();
   mMidiOutCable->SetPatchCableSource(new PatchCableSource(this, kConnectionType_Note));
   mMidiOutCable->GetPatchCableSource()->SetOverrideCableDir(ofVec2f(1, 0));
   AddPatchCableSource(mMidiOutCable->GetPatchCableSource());
   mMidiOutCable->GetPatchCableSource()->SetManualPosition(206 - 10, 10);


   if (mPlugin)
   {
      CreateParameterSliders();
   }

   //const auto* editor = mPlugin->createEditor();
}

VSTPlugin::~VSTPlugin()
{
}

void VSTPlugin::Exit()
{
   IDrawableModule::Exit();
   if (mWindow)
   {
      mWindow.reset();
   }
   if (mPlugin)
   {
      mPlugin.reset();
   }
}

std::string VSTPlugin::GetTitleLabel() const
{
   return GetPluginFormatName() + ": " + GetPluginName();
}

std::string VSTPlugin::GetPluginName() const
{
   return mPluginName;
}

std::string VSTPlugin::GetPluginFormatName() const
{
   if (mPlugin)
   {
      const auto& desc = dynamic_cast<juce::AudioPluginInstance*>(mPlugin.get())->getPluginDescription();
      return ofToString(desc.pluginFormatName.toLowerCase());
   }
   return "no plugin loaded";
}

std::string VSTPlugin::GetPluginId() const
{
   if (mPlugin)
   {
      const auto& desc = dynamic_cast<juce::AudioPluginInstance*>(mPlugin.get())->getPluginDescription();
      return GetPluginName() + "_" + ofToString(desc.uniqueId);
   }
   return "no plugin loaded";
}

void VSTPlugin::GetVSTFileDesc(std::string vstName, juce::PluginDescription& desc)
{
   std::string path = VSTLookup::GetVSTPath(vstName);

   auto types = TheSynth->GetKnownPluginList().getTypes();
   bool found = false;
   for (int i = 0; i < types.size(); ++i)
   {
      if (path == types[i].fileOrIdentifier)
      {
         found = true;
         desc = types[i];
         //return desc;
         break;
      }
   }

   if (!found) //couldn't find the VST at this path. maybe its installation got moved, or the bespoke state was saved on a different computer. try to find a VST of the same name.
   {
      juce::String desiredVstName = juce::String(path).replaceCharacter('\\', '/').fromLastOccurrenceOf("/", false, false).upToFirstOccurrenceOf(".", false, false);
      for (int i = 0; i < types.size(); ++i)
      {
         juce::String thisVstName = juce::String(types[i].fileOrIdentifier).replaceCharacter('\\', '/').fromLastOccurrenceOf("/", false, false).upToFirstOccurrenceOf(".", false, false);
         if (thisVstName == desiredVstName)
         {
            found = true;
            desc = types[i];
            //return desc;
            break;
         }
      }
   }
}

void VSTPlugin::SetVST(juce::PluginDescription pluginDesc)
{
   //juce::PluginDescription vstDesc;

   //ofLog() << "loading VST: " << vstName << "ID: " << id;

   juce::String pluginId = pluginDesc.createIdentifierString();
   std::string strPluginId = pluginId.toStdString();

   //mModuleSaveData.SetString("vst", vstName);
   //mModuleSaveData.SetInt("vstId", id);
   mModuleSaveData.SetString("pluginId", strPluginId);
   //mark VST as used
   if(strPluginId != "--0-0")
   {
       ofxJSONElement root;
       root.open(ofToDataPath("vst/used_vsts.json"));

       auto time = juce::Time::getCurrentTime();
       root["vsts"][strPluginId] = (double)time.currentTimeMillis();

       root.save(ofToDataPath("vst/used_vsts.json"), true);
   }

   if (mPlugin != nullptr && dynamic_cast<juce::AudioPluginInstance*>(mPlugin.get())->getPluginDescription().matchesIdentifierString(pluginId))
      return; //this VST is already loaded! we're all set

   if (mPlugin != nullptr && mWindow != nullptr)
   {
      VSTWindow* window = mWindow.release();
      delete window;
      //delete mWindowOverlay;
      //mWindowOverlay = nullptr;
   }

   LoadVST(pluginDesc);
}

void VSTPlugin::LoadVST(juce::PluginDescription desc)
{
   mPluginReady = false;

   /*auto completionCallback = [this, &callbackDone] (std::unique_ptr<juce::AudioPluginInstance> instance, const String& error)
         {
            if (instance == nullptr)
            {
               ofLog() << error;
            }
            else
            {
               mVSTMutex.lock();
               //instance->enableAllBuses();
               instance->prepareToPlay(gSampleRate, gBufferSize);
               instance->setPlayHead(&mPlayhead);
               mNumInputs = CLAMP(instance->getTotalNumInputChannels(), 1, 4);
               mNumOutputs = CLAMP(instance->getTotalNumOutputChannels(), 1, 4);
               ofLog() << "vst inputs: " << mNumInputs << "  vst outputs: " << mNumOutputs;
               mPlugin = std::move(instance);
               mVSTMutex.unlock();
            }
            if (mPlugin != nullptr)
               CreateParameterSliders();
            callbackDone = true;
         };

         TheSynth->GetAudioPluginFormatManager().getFormat(i)->createPluginInstanceAsync(desc, gSampleRate, gBufferSize, completionCallback);*/

   mVSTMutex.lock();
   juce::String errorMessage;
   mPlugin = TheSynth->GetAudioPluginFormatManager().createPluginInstance(desc, gSampleRate, gBufferSize, errorMessage);
   if (mPlugin != nullptr)
   {
      mPlugin->enableAllBuses();
      mPlugin->disableNonMainBuses();

      /*
       * For now, since Bespoke is at best stereo in stereo out,
       * Disable all non-main input and output busses
       */
      auto layouts = mPlugin->getBusesLayout();

      for (int busIndex = 1; busIndex < layouts.outputBuses.size(); ++busIndex)
         layouts.outputBuses.getReference(busIndex) = AudioChannelSet::disabled();
      for (int busIndex = 1; busIndex < layouts.inputBuses.size(); ++busIndex)
         layouts.inputBuses.getReference(busIndex) = AudioChannelSet::disabled();

      ofLog() << "vst layout  - inputs: " << layouts.inputBuses.size() << " x outputs: " << layouts.outputBuses.size();
      mPlugin->setBusesLayout(layouts);

      mPlugin->prepareToPlay(gSampleRate, gBufferSize);
      mPlugin->setPlayHead(&mPlayhead);
      mNumInputs = mPlugin->getTotalNumInputChannels();
      mNumOutputs = mPlugin->getTotalNumOutputChannels();
      ofLog() << "vst channel - inputs: " << mNumInputs << " x outputs: " << mNumOutputs;

      mPluginName = mPlugin->getName().toStdString();

      CreateParameterSliders();

      mPluginReady = true;
   }
   else
   {
      TheSynth->LogEvent("error loading VST: " + errorMessage.toStdString(), kLogEventType_Error);

      if (mModuleSaveData.HasProperty("vst") && mModuleSaveData.GetString("vst").length() > 0)
         mPluginName = GetFileNameWithoutExtension(mModuleSaveData.GetString("vst")).toStdString() + " (not loaded)";
   }
   mVSTMutex.unlock();
}

bool VSTPlugin::ParameterNameExists(std::string name, int checkUntilIndex) const
{
   for (int i = 0; i < checkUntilIndex; ++i)
   {
      if (mParameterSliders[i].mName == name)
         return true;
   }

   return false;
}

void VSTPlugin::CreateParameterSliders()
{
   assert(mPlugin);

   for (auto& slider : mParameterSliders)
   {
      if (slider.mSlider)
      {
         slider.mSlider->SetShowing(false);
         RemoveUIControl(slider.mSlider);
         slider.mSlider->Delete();
      }
   }
   mParameterSliders.clear();

   mShowParameterDropdown->Clear();

   const auto& parameters = mPlugin->getParameters();

   int numParameters = MIN(1000, parameters.size());
   mParameterSliders.resize(numParameters);
   for (int i = 0; i < numParameters; ++i)
   {
      mParameterSliders[i].mValue = parameters[i]->getValue();
      juce::String originalParamName = parameters[i]->getName(32);
      std::string name(originalParamName.getCharPointer());
      try
      {
         int append = 0;
         while (ParameterNameExists(name, i) || FindUIControl(name.c_str()))
         {
            ++append;
            name = originalParamName.toStdString() + ofToString(append);
         }
      }
      catch (UnknownUIControlException& e)
      {
      }
      mParameterSliders[i].mParameter = parameters[i];
      mParameterSliders[i].mName = name.c_str();
      mParameterSliders[i].mShowing = false;
      if (numParameters <= 30) //only show parameters in list if there are a small number. if there are many, make the user adjust them in the VST before they can be controlled
      {
         mShowParameterDropdown->AddLabel(name.c_str(), i);
         mParameterSliders[i].mInSelectorList = true;
      }
      else
      {
         mParameterSliders[i].mInSelectorList = false;
      }
   }
}

void VSTPlugin::Poll()
{
   if (mDisplayMode == kDisplayMode_Sliders)
   {
      for (int i = 0; i < mParameterSliders.size(); ++i)
      {
         float value = mParameterSliders[i].mParameter->getValue();
         if (mParameterSliders[i].mValue != value)
            mParameterSliders[i].mValue = value;
      }

      if (mChangeGestureParameterIndex != -1)
      {
         if (mChangeGestureParameterIndex < (int)mParameterSliders.size() &&
             !mParameterSliders[mChangeGestureParameterIndex].mInSelectorList &&
             mTemporarilyDisplayedParamIndex != mChangeGestureParameterIndex)
         {
            if (mTemporarilyDisplayedParamIndex != -1)
               mShowParameterDropdown->RemoveLabel(mTemporarilyDisplayedParamIndex);
            mTemporarilyDisplayedParamIndex = mChangeGestureParameterIndex;
            mShowParameterDropdown->AddLabel(mParameterSliders[mChangeGestureParameterIndex].mName, mChangeGestureParameterIndex);
         }

         mChangeGestureParameterIndex = -1;
      }
   }

   if (mWantOpenVstWindow)
   {
      if (mPlugin != nullptr)
      {
         if (mWindow == nullptr)
            mWindow = std::unique_ptr<VSTWindow>(VSTWindow::CreateVSTWindow(this, VSTWindow::Normal));
         mWindow->ShowWindow();

         //if (mWindow->GetNSViewComponent())
         //   mWindowOverlay = new NSWindowOverlay(mWindow->GetNSViewComponent()->getView());
      }
      mWantOpenVstWindow = false;
   }
}

void VSTPlugin::audioProcessorParameterChangeGestureBegin(juce::AudioProcessor* processor, int parameterIndex)
{
   //set this parameter so we can check it in Poll()
   mChangeGestureParameterIndex = parameterIndex;
}

void VSTPlugin::Process(double time)
{
   if (!mPluginReady)
   {
      //bypass
      GetBuffer()->SetNumActiveChannels(2);
      SyncBuffers();
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (GetTarget())
            Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }

      GetBuffer()->Clear();
   }

#if BESPOKE_LINUX //HACK: weird race condition, which this seems to fix for now
   if (mPlugin == nullptr)
      return;
#endif

   PROFILER(VSTPlugin);

   int inputChannels = MAX(2, mNumInputs);
   GetBuffer()->SetNumActiveChannels(inputChannels);
   SyncBuffers();

   /*
    * Multi-out VSTs which can't disable those outputs will expect *something* in the
    * buffer even though we don't read it.
    */
   int bufferChannels = MAX(inputChannels, mNumOutputs); // how much to allocate in the juce::AudioBuffer

   const int kSafetyMaxChannels = 16; //hitting a crazy issue (memory stomp?) where numchannels is getting blown out sometimes

   int bufferSize = GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   juce::AudioBuffer<float> buffer(bufferChannels, bufferSize);
   for (int i = 0; i < inputChannels && i < kSafetyMaxChannels; ++i)
      buffer.copyFrom(i, 0, GetBuffer()->GetChannel(MIN(i, GetBuffer()->NumActiveChannels() - 1)), GetBuffer()->BufferSize());

   IAudioReceiver* target = GetTarget();

   if (mEnabled && mPlugin != nullptr)
   {
      mVSTMutex.lock();

      ComputeSliders(0);

      {
         const juce::ScopedLock lock(mMidiInputLock);

         for (int i = 0; i < mChannelModulations.size(); ++i)
         {
            ChannelModulations& mod = mChannelModulations[i];
            int channel = i + 1;
            if (i == kGlobalModulationIdx)
               channel = 1;

            if (mUseVoiceAsChannel == false)
               channel = mChannel;

            float bend = mod.mModulation.pitchBend ? mod.mModulation.pitchBend->GetValue(0) : 0;
            if (bend != mod.mLastPitchBend)
            {
               mod.mLastPitchBend = bend;
               mMidiBuffer.addEvent(juce::MidiMessage::pitchWheel(channel, (int)ofMap(bend, -mPitchBendRange, mPitchBendRange, 0, 16383, K(clamp))), 0);
            }
            float modWheel = mod.mModulation.modWheel ? mod.mModulation.modWheel->GetValue(0) : 0;
            if (modWheel != mod.mLastModWheel)
            {
               mod.mLastModWheel = modWheel;
               mMidiBuffer.addEvent(juce::MidiMessage::controllerEvent(channel, mModwheelCC, ofClamp(modWheel * 127, 0, 127)), 0);
            }
            float pressure = mod.mModulation.pressure ? mod.mModulation.pressure->GetValue(0) : 0;
            if (pressure != mod.mLastPressure)
            {
               mod.mLastPressure = pressure;
               mMidiBuffer.addEvent(juce::MidiMessage::channelPressureChange(channel, ofClamp(pressure * 127, 0, 127)), 0);
            }
         }

         /*if (!mMidiBuffer.isEmpty())
         {
            ofLog() << mMidiBuffer.getFirstEventTime() << " " << mMidiBuffer.getLastEventTime();
         }*/

         mMidiBuffer.addEvents(mFutureMidiBuffer, 0, mFutureMidiBuffer.getLastEventTime() + 1, 0);
         mFutureMidiBuffer.clear();
         mFutureMidiBuffer.addEvents(mMidiBuffer, gBufferSize, mMidiBuffer.getLastEventTime() - gBufferSize + 1, -gBufferSize);
         mMidiBuffer.clear(gBufferSize, mMidiBuffer.getLastEventTime() + 1);

         if (mWantsPanic)
         {
            mWantsPanic = false;

            mMidiBuffer.clear();
            for (int channel = 1; channel <= 16; ++channel)
               mMidiBuffer.addEvent(juce::MidiMessage::allNotesOff(channel), 0);
            for (int channel = 1; channel <= 16; ++channel)
               mMidiBuffer.addEvent(juce::MidiMessage::allSoundOff(channel), 1);
         }

         mPlugin->processBlock(buffer, mMidiBuffer);

         if (!mMidiBuffer.isEmpty())
         {
            auto midiIt = mMidiBuffer.begin();
            while (midiIt != mMidiBuffer.end())
            {
               auto msg = (*midiIt).getMessage();
               auto tMidi = time + (*midiIt).samplePosition * gInvSampleRateMs;
               if (msg.isNoteOn())
               {
                  mMidiOutCable->PlayNoteOutput(tMidi, msg.getNoteNumber(), msg.getVelocity());
               }
               else if (msg.isNoteOff())
               {
                  mMidiOutCable->PlayNoteOutput(tMidi, msg.getNoteNumber(), 0);
               }
               else if (msg.isController())
               {
                  mMidiOutCable->SendCCOutput(msg.getControllerNumber(), msg.getControllerValue());
               }
               midiIt++;
            }
         }

         mMidiBuffer.clear();
      }
      mVSTMutex.unlock();

      GetBuffer()->Clear();
      /*
       * Until we support multi output we end up with this requirement that
       * the output is at most stereo. This stops mis-behaving plugins which
       * output the full buffer set from copying that onto the output.
       * (Ahem: Surge 1.9)
       */
      int nChannelsToCopy = MIN(2, buffer.getNumChannels());
      for (int ch = 0; ch < nChannelsToCopy && ch < kSafetyMaxChannels; ++ch)
      {
         int outputChannel = MIN(ch, GetBuffer()->NumActiveChannels() - 1);
         for (int sampleIndex = 0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
         {
            GetBuffer()->GetChannel(outputChannel)[sampleIndex] += buffer.getSample(ch, sampleIndex) * mVol;
         }
         if (target)
            Add(target->GetBuffer()->GetChannel(outputChannel), GetBuffer()->GetChannel(outputChannel), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(outputChannel), bufferSize, outputChannel);
      }
   }
   else
   {
      //bypass
      for (int ch = 0; ch < GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (target)
            Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Clear();
}

void VSTPlugin::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mPluginReady || mPlugin == nullptr)
      return;

   if (!mEnabled)
      return;

   if (pitch < 0 || pitch > 127)
      return;

   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;

   const juce::ScopedLock lock(mMidiInputLock);

   int sampleNumber = (time - gTime) * gSampleRateMs;
   //ofLog() << sampleNumber;

   if (velocity > 0)
   {
      mMidiBuffer.addEvent(juce::MidiMessage::noteOn(mUseVoiceAsChannel ? channel : mChannel, pitch, (uint8)velocity), sampleNumber);
      //ofLog() << "+ vst note on: " << (mUseVoiceAsChannel ? channel : mChannel) << " " << pitch << " " << (uint8)velocity;
   }
   else
   {
      mMidiBuffer.addEvent(juce::MidiMessage::noteOff(mUseVoiceAsChannel ? channel : mChannel, pitch), sampleNumber);
      //ofLog() << "- vst note off: " << (mUseVoiceAsChannel ? channel : mChannel) << " " << pitch;
   }

   int modIdx = voiceIdx;
   if (voiceIdx == -1)
      modIdx = kGlobalModulationIdx;

   mChannelModulations[modIdx].mModulation = modulation;
}

void VSTPlugin::SendCC(int control, int value, int voiceIdx /*=-1*/)
{
   if (!mPluginReady || mPlugin == nullptr)
      return;

   if (control < 0 || control > 127)
      return;

   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;

   const juce::ScopedLock lock(mMidiInputLock);

   mMidiBuffer.addEvent(juce::MidiMessage::controllerEvent((mUseVoiceAsChannel ? channel : mChannel), control, (uint8)value), 0);
}

void VSTPlugin::SendMidi(const juce::MidiMessage& message)
{
   if (!mPluginReady || mPlugin == nullptr)
      return;

   const juce::ScopedLock lock(mMidiInputLock);

   mMidiBuffer.addEvent(message, 0);
}

void VSTPlugin::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void VSTPlugin::PreDrawModule()
{
   /*if (mDisplayMode == kDisplayMode_PluginOverlay && mWindowOverlay)
   {
      mWindowOverlay->GetDimensions(mOverlayWidth, mOverlayHeight);
      if (mWindow)
      {
         mOverlayWidth = 500;
         mOverlayHeight = 500;
         float contentMult = gDrawScale;
         float width = mOverlayWidth * contentMult;
         float height = mOverlayHeight * contentMult;
         mWindow->setSize(width, height);
      }
      mWindowOverlay->UpdatePosition(this);
   }*/
}

void VSTPlugin::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mVolSlider->Draw();
   mPresetFileSelector->Draw();
   mSavePresetFileButton->Draw();
   mOpenEditorButton->Draw();
   mPanicButton->Draw();
   mShowParameterDropdown->Draw();

   ofPushStyle();
   ofSetColor(IDrawableModule::GetColor(kModuleType_Note));
   DrawTextRightJustify("midi out:", 206 - 18, 14);
   ofPopStyle();

   if (mDisplayMode == kDisplayMode_Sliders)
   {
      int sliderCount = 0;
      for (auto& slider : mParameterSliders)
      {
         if (slider.mSlider)
         {
            slider.mSlider->SetShowing(slider.mShowing);
            if (slider.mShowing)
            {
               const int kRows = 20;
               slider.mSlider->SetPosition(3 + (slider.mSlider->GetRect().width + 2) * (sliderCount / kRows), 60 + (17 * (sliderCount % kRows)));

               slider.mSlider->Draw();

               ++sliderCount;
            }
         }
      }
   }
}

void VSTPlugin::GetModuleDimensions(float& width, float& height)
{
   if (mDisplayMode == kDisplayMode_PluginOverlay)
   {
      /*if (mWindowOverlay)
      {
         width = mOverlayWidth;
         height = mOverlayHeight+20;
      }
      else
      {*/
      width = 206;
      height = 40;
      //}
   }
   else
   {
      width = 206;
      height = 58;
      for (auto slider : mParameterSliders)
      {
         if (slider.mSlider && slider.mShowing)
         {
            width = MAX(width, slider.mSlider->GetRect(true).x + slider.mSlider->GetRect(true).width + 3);
            height = MAX(height, slider.mSlider->GetRect(true).y + slider.mSlider->GetRect(true).height + 3);
         }
      }
   }
}

void VSTPlugin::OnVSTWindowClosed()
{
   mWindow.release();
}

std::vector<IUIControl*> VSTPlugin::ControlsToIgnoreInSaveState() const
{
   std::vector<IUIControl*> ignore;
   ignore.push_back(mPresetFileSelector);
   return ignore;
}

void VSTPlugin::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mPresetFileSelector)
   {
      if (mPresetFileIndex >= 0 && mPresetFileIndex < (int)mPresetFilePaths.size())
      {
         File resourceFile = File(mPresetFilePaths[mPresetFileIndex]);

         if (!resourceFile.existsAsFile())
         {
            DBG("File doesn't exist ...");
            return;
         }

         std::unique_ptr<FileInputStream> input(resourceFile.createInputStream());

         if (!input->openedOk())
         {
            DBG("Failed to open file");
            return;
         }

         int rev = input->readInt();

         int64 vstStateSize = input->readInt64();
         char* vstState = new char[vstStateSize];
         input->read(vstState, vstStateSize);
         mPlugin->setStateInformation(vstState, vstStateSize);

         int64 vstProgramStateSize = input->readInt64();
         if (vstProgramStateSize > 0)
         {
            char* vstProgramState = new char[vstProgramStateSize];
            input->read(vstProgramState, vstProgramStateSize);
            mPlugin->setCurrentProgramStateInformation(vstProgramState, vstProgramStateSize);
         }

         if (rev >= 2)
         {
            int numParamsShowing = input->readInt();
            for (auto& param : mParameterSliders)
               param.mShowing = false;
            for (int i = 0; i < numParamsShowing; ++i)
            {
               int index = input->readInt();
               if (index < mParameterSliders.size())
               {
                  mParameterSliders[index].mShowing = true;
                  if (mParameterSliders[index].mSlider == nullptr)
                     mParameterSliders[index].MakeSlider(this);
               }
            }
         }
      }
   }

   if (list == mShowParameterDropdown)
   {
      mParameterSliders[mShowParameterIndex].mShowing = true;
      if (mParameterSliders[mShowParameterIndex].mSlider == nullptr)
         mParameterSliders[mShowParameterIndex].MakeSlider(this);
      mParameterSliders[mShowParameterIndex].mInSelectorList = true;
      mShowParameterIndex = -1;
      mTemporarilyDisplayedParamIndex = -1;
   }
}

void VSTPlugin::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   for (int i = 0; i < mParameterSliders.size(); ++i)
   {
      if (mParameterSliders[i].mSlider == slider)
      {
         mParameterSliders[i].mParameter->setValue(mParameterSliders[i].mValue);
      }
   }
}

void VSTPlugin::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void VSTPlugin::CheckboxUpdated(Checkbox* checkbox)
{
}

void VSTPlugin::ButtonClicked(ClickButton* button)
{
   if (button == mOpenEditorButton)
      mWantOpenVstWindow = true;

   if (button == mPanicButton)
   {
      mWantsPanic = true;
   }

   if (button == mSavePresetFileButton && mPlugin != nullptr)
   {
      juce::File(ofToDataPath("vst/presets/" + GetPluginId())).createDirectory();
      FileChooser chooser("Save preset as...", File(ofToDataPath("vst/presets/" + GetPluginId() + "/preset.vstp")), "*.vstp", true, false, TheSynth->GetFileChooserParent());
      if (chooser.browseForFileToSave(true))
      {
         std::string path = chooser.getResult().getFullPathName().toStdString();

         File resourceFile(path);
         TemporaryFile tempFile(resourceFile);

         {
            FileOutputStream output(tempFile.getFile());

            if (!output.openedOk())
            {
               DBG("FileOutputStream didn't open correctly ...");
               return;
            }

            juce::MemoryBlock vstState;
            mPlugin->getStateInformation(vstState);
            juce::MemoryBlock vstProgramState;
            mPlugin->getCurrentProgramStateInformation(vstProgramState);

            output.writeInt(GetModuleSaveStateRev());
            output.writeInt64(vstState.getSize());
            output.write(vstState.getData(), vstState.getSize());
            output.writeInt64(vstProgramState.getSize());
            if (vstProgramState.getSize() > 0)
               output.write(vstProgramState.getData(), vstProgramState.getSize());

            std::vector<int> exposedParams;
            for (int i = 0; i < (int)mParameterSliders.size(); ++i)
            {
               if (mParameterSliders[i].mShowing)
                  exposedParams.push_back(i);
            }
            output.writeInt((int)exposedParams.size());
            for (int i : exposedParams)
               output.writeInt(i);

            output.flush(); // (called explicitly to force an fsync on posix)

            if (output.getStatus().failed())
            {
               DBG("An error occurred in the FileOutputStream");
               return;
            }
         }

         bool success = tempFile.overwriteTargetFileWithTemporary();
         if (!success)
         {
            DBG("An error occurred writing the file");
            return;
         }

         RefreshPresetFiles();

         for (size_t i = 0; i < mPresetFilePaths.size(); ++i)
         {
            if (mPresetFilePaths[i] == path)
            {
               mPresetFileIndex = (int)i;
               break;
            }
         }
      }
   }
}

void VSTPlugin::DropdownClicked(DropdownList* list)
{
   if (list == mPresetFileSelector)
      RefreshPresetFiles();
}

void VSTPlugin::RefreshPresetFiles()
{
   if (mPlugin == nullptr)
      return;

   juce::File(ofToDataPath("vst/presets/" + GetPluginId())).createDirectory();
   mPresetFilePaths.clear();
   mPresetFileSelector->Clear();
   for (const auto& entry : RangedDirectoryIterator{ File{ ofToDataPath("vst/presets/" + GetPluginId()) }, false, "*.vstp" })
   {
      const auto& file = entry.getFile();
      mPresetFileSelector->AddLabel(file.getFileName().toStdString(), (int)mPresetFilePaths.size());
      mPresetFilePaths.push_back(file.getFullPathName().toStdString());
   }
}


void VSTPlugin::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("vst", moduleInfo, "", VSTLookup::FillVSTList);
   mModuleSaveData.LoadInt("vstId", moduleInfo, 0, 0, 0);
   mModuleSaveData.LoadString("pluginId", moduleInfo, "", VSTLookup::FillVSTList);
   
   mModuleSaveData.LoadString("target", moduleInfo);

   mModuleSaveData.LoadInt("channel", moduleInfo, 1, 0, 16);
   mModuleSaveData.LoadBool("usevoiceaschannel", moduleInfo, false);
   mModuleSaveData.LoadFloat("pitchbendrange", moduleInfo, 2, 1, 96, K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)", moduleInfo, 1, 0, 127, K(isTextField));

   SetUpFromSaveData();
}

void VSTPlugin::SetUpFromSaveData()
{
   juce::PluginDescription pluginDesc;
   
   if (mModuleSaveData.HasProperty("pluginId") && mModuleSaveData.GetString("pluginId") != "")
   {
   DBG("try to use description ident");
      auto pluginId = juce::String(mModuleSaveData.GetString("pluginId"));
      DBG(pluginId);
      pluginDesc = VSTLookup::GetPluginDesc(pluginId);
   }
  
   else if (mModuleSaveData.HasProperty("vstId") && mModuleSaveData.GetInt("vstId") != 0)
   {
     DBG("try to use uniqueId");
      int vstId = mModuleSaveData.GetInt("vstId");
      if (vstId != 0)
      {
         pluginDesc = VSTLookup::GetPluginDesc(vstId);
      }
   }
   
   else
   {
   DBG("try to use filename");
      std::string vstName = mModuleSaveData.GetString("vst");
      if (vstName != "")
      {
         GetVSTFileDesc(vstName, pluginDesc);
      }
   }

   SetVST(pluginDesc);

   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   mChannel = mModuleSaveData.GetInt("channel");
   mUseVoiceAsChannel = mModuleSaveData.GetBool("usevoiceaschannel");
   mPitchBendRange = mModuleSaveData.GetFloat("pitchbendrange");
   mModwheelCC = mModuleSaveData.GetInt("modwheelcc(1or74)");
}

void VSTPlugin::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   if (mPlugin)
   {
      out << true;
      juce::MemoryBlock vstState;
      mPlugin->getStateInformation(vstState);
      out << (int)vstState.getSize();
      out.WriteGeneric(vstState.getData(), (int)vstState.getSize());

      juce::MemoryBlock vstProgramState;
      mPlugin->getCurrentProgramStateInformation(vstProgramState);
      out << (int)vstProgramState.getSize();
      if (vstProgramState.getSize() > 0)
         out.WriteGeneric(vstProgramState.getData(), (int)vstProgramState.getSize());

      std::vector<int> exposedParams;
      for (int i = 0; i < (int)mParameterSliders.size(); ++i)
      {
         if (mParameterSliders[i].mShowing)
            exposedParams.push_back(i);
      }
      out << (int)exposedParams.size();
      for (int i : exposedParams)
         out << i;
   }
   else
   {
      out << false;
   }

   IDrawableModule::SaveState(out);
}

void VSTPlugin::LoadState(FileStreamIn& in, int rev)
{
   if (rev >= 3)
   {
      LoadVSTFromSaveData(in, rev);
   }
   else
   {
      //make all sliders, like we used to, so that the controls can load correctly
      for (auto& parameter : mParameterSliders)
      {
         if (parameter.mSlider == nullptr)
            parameter.MakeSlider(this);
      }
   }

   IDrawableModule::LoadState(in, rev);

   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev < 3)
      LoadVSTFromSaveData(in, rev);
}

void VSTPlugin::LoadVSTFromSaveData(FileStreamIn& in, int rev)
{
   bool hasPlugin;
   in >> hasPlugin;
   if (hasPlugin)
   {
      int vstStateSize;
      in >> vstStateSize;
      char* vstState = new char[vstStateSize];
      in.ReadGeneric(vstState, vstStateSize);

      int vstProgramStateSize = 0;
      if (rev >= 1)
         in >> vstProgramStateSize;
      char* vstProgramState = new char[vstProgramStateSize];
      if (rev >= 1 && vstProgramStateSize > 0)
         in.ReadGeneric(vstProgramState, vstProgramStateSize);

      if (mPlugin != nullptr)
      {
         ofLog() << "loading vst state for " << mPlugin->getName();

         mPlugin->setStateInformation(vstState, vstStateSize);
         if (rev >= 1 && vstProgramStateSize > 0)
            mPlugin->setCurrentProgramStateInformation(vstProgramState, vstProgramStateSize);
      }
      else
      {
         TheSynth->LogEvent("Couldn't instantiate plugin to load state for " + mModuleSaveData.GetString("vst"), kLogEventType_Error);
      }

      if (rev >= 2)
      {
         int numParamsShowing;
         in >> numParamsShowing;
         for (auto& param : mParameterSliders)
            param.mShowing = false;
         for (int i = 0; i < numParamsShowing; ++i)
         {
            int index;
            in >> index;
            {
               mParameterSliders[index].mShowing = true;
               if (mParameterSliders[index].mSlider == nullptr)
                  mParameterSliders[index].MakeSlider(this);
            }
         }
      }
   }
}

void VSTPlugin::ParameterSlider::MakeSlider(VSTPlugin* owner)
{
   mSlider = new FloatSlider(owner, mName.c_str(), -1, -1, 200, 15, &mValue, 0, 1);
}

void VSTPlugin::OnUIControlRequested(const char* name)
{
   for (auto& parameter : mParameterSliders)
   {
      if (parameter.mName == name && parameter.mSlider == nullptr)
         parameter.MakeSlider(this);
   }
}
