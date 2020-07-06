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
//#include "NSWindowOverlay.h"

namespace
{
   const int kGlobalModulationIdx = 16;
}

namespace VSTLookup
{
   static juce::AudioPluginFormatManager sFormatManager;
   static juce::KnownPluginList sPluginList;
   
   void GetAvailableVSTs(vector<string>& vsts, bool rescan)
   {
      static bool sFirstTime = true;
      if (sFirstTime)
         sFormatManager.addDefaultFormats();

      if (rescan)
      {
         sPluginList.clear();
         juce::File deadMansPedalFile(ofToDataPath("internal/deadmanspedal.txt"));
         juce::FileSearchPath searchPath;
         for (int i = 0; i < TheSynth->GetUserPrefs()["vstsearchdirs"].size(); ++i)
            searchPath.add(juce::File(TheSynth->GetUserPrefs()["vstsearchdirs"][i].asString()));
         for (int i = 0; i < sFormatManager.getNumFormats(); ++i)
         {
            juce::PluginDirectoryScanner scanner(sPluginList, *(sFormatManager.getFormat(i)), searchPath, true, deadMansPedalFile, true);
            juce::String nameOfPluginBeingScanned;
            while (scanner.scanNextFile(true, nameOfPluginBeingScanned))
            {
               ofLog() << "scanning " + nameOfPluginBeingScanned;
            }
         }
         sPluginList.createXml()->writeTo(juce::File(ofToDataPath("internal/found_vsts.xml")));
      }
      else
      {
         auto file = juce::File(ofToDataPath("internal/found_vsts.xml"));
         if (file.existsAsFile())
         {
            auto xml = juce::parseXML(file);
            sPluginList.recreateFromXml(*xml);
         }
      }
      auto types = sPluginList.getTypes();
      for (int i=0; i<types.size(); ++i)
      {
         vsts.push_back(types[i].fileOrIdentifier.toStdString());
      }
      
      std::map<string, double> lastUsedTimes;
      
      ofxJSONElement root;
      root.open(ofToDataPath("internal/used_vsts.json"));
      ofxJSONElement jsonList = root["vsts"];

      for (auto it = jsonList.begin(); it != jsonList.end(); ++it)
      {
         string key = it.key().asString();
         lastUsedTimes[key] = jsonList[key].asDouble();
      }
      
      std::sort(vsts.begin(), vsts.end(), [lastUsedTimes](string a, string b) {
         auto itA = lastUsedTimes.find(a);
         auto itB = lastUsedTimes.find(b);
         if (itA == lastUsedTimes.end() && itB == lastUsedTimes.end())
            return a < b;
         if (itA != lastUsedTimes.end() && itB == lastUsedTimes.end())
            return true;
         if (itA == lastUsedTimes.end() && itB != lastUsedTimes.end())
            return false;
         double timeA = (*itA).second;
         double timeB = (*itB).second;
         return timeA > timeB;
      });

      sFirstTime = false;
   }
   
   void FillVSTList(DropdownList* list)
   {
      assert(list);
      vector<string> vsts;
      GetAvailableVSTs(vsts, false);
      for (int i=0; i<vsts.size(); ++i)
         list->AddLabel(vsts[i].c_str(), i);
   }
   
   string GetVSTPath(string vstName)
   {
      if (juce::String(vstName).contains("/") || juce::String(vstName).contains("\\"))  //already a path
         return vstName;
      
      auto types = sPluginList.getTypes();
      for (int i=0; i<types.size(); ++i)
      {
         juce::File vst(types[i].fileOrIdentifier);
         if (vst.getFileName().toStdString() == vstName)
            return types[i].fileOrIdentifier.toStdString();
      }
      
      return "";
   }
}

VSTPlugin::VSTPlugin()
: IAudioProcessor(gBufferSize)
, mVol(1)
, mVolSlider(nullptr)
, mPlugin(nullptr)
, mChannel(1)
, mPitchBendRange(2)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
, mUseVoiceAsChannel(false)
, mProgramChangeSelector(nullptr)
, mProgramChange(0)
, mOpenEditorButton(nullptr)
//, mWindowOverlay(nullptr)
, mDisplayMode(kDisplayMode_Sliders)
{
   if (VSTLookup::sFormatManager.getNumFormats() == 0)
      VSTLookup::sFormatManager.addDefaultFormats();
   
   mChannelModulations.resize(kGlobalModulationIdx+1);
}

void VSTPlugin::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",3,3,80,15,&mVol,0,1);
   mProgramChangeSelector = new DropdownList(this,"program change",100,3,&mProgramChange);
   mOpenEditorButton = new ClickButton(this, "open", 150, 3);
   
   for (int i=0; i<128; ++i)
      mProgramChangeSelector->AddLabel(ofToString(i), i);
   
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
   if (mWindow)
   {
      VSTWindow* window = mWindow.release();
      delete window;
   }
}

string VSTPlugin::GetTitleLabel()
{
   if (mPlugin)
      return "vst: "+GetPluginName();
   return "vst";
}

string VSTPlugin::GetPluginName()
{
   if (mPlugin)
      return mPlugin->getName().toStdString();
   return "no plugin loaded";
}

void VSTPlugin::SetVST(string vstName)
{
   ofLog() << "loading VST: " << vstName;
   
   mModuleSaveData.SetString("vst", vstName);
   string path = VSTLookup::GetVSTPath(vstName);
   
   //mark VST as used
   {
      ofxJSONElement root;
      root.open(ofToDataPath("internal/used_vsts.json"));
      
      Time time = Time::getCurrentTime();
      root["vsts"][path] = (double)time.currentTimeMillis();

      root.save(ofToDataPath("internal/used_vsts.json"), true);
   }
   
   if (mPlugin != nullptr && dynamic_cast<juce::AudioPluginInstance*>(mPlugin.get())->getPluginDescription().fileOrIdentifier.toStdString() == path)
      return;  //this VST is already loaded! we're all set
   
   if (mPlugin != nullptr && mWindow != nullptr)
   {
      VSTWindow* window = mWindow.release();
      delete window;
      //delete mWindowOverlay;
      //mWindowOverlay = nullptr;
   }
   
   juce::String errorMessage;
   auto types = VSTLookup::sPluginList.getTypes();
   for (int i=0; i<types.size(); ++i)
   {
      if (path == types[i].fileOrIdentifier)
      {
         PluginDescription desc = types[i];
            
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
         
         sFormatManager.getFormat(i)->createPluginInstanceAsync(desc, gSampleRate, gBufferSize, completionCallback);*/
         
         mVSTMutex.lock();
         mPlugin = VSTLookup::sFormatManager.createPluginInstance(desc, gSampleRate, gBufferSize, errorMessage);
         if (mPlugin != nullptr)
         {
            mPlugin->prepareToPlay(gSampleRate, gBufferSize);
            mPlugin->setPlayHead(&mPlayhead);
            mNumInputs = CLAMP(mPlugin->getTotalNumInputChannels(), 1, 4);
            mNumOutputs = CLAMP(mPlugin->getTotalNumOutputChannels(), 1, 4);
            ofLog() << "vst inputs: " << mNumInputs << "  vst outputs: " << mNumOutputs;
         }
         mVSTMutex.unlock();
         
         if (mPlugin != nullptr)
            CreateParameterSliders();

         break;
      }
   }
}

void VSTPlugin::CreateParameterSliders()
{
   assert(mPlugin);
   
   for (auto& slider : mParameterSliders)
   {
      slider.mSlider->SetShowing(false);
      RemoveUIControl(slider.mSlider);
      slider.mSlider->Delete();
   }
   mParameterSliders.clear();
   
   const auto& parameters = mPlugin->getParameters();
   
   int numParameters = MIN(100, parameters.size());
   mParameterSliders.resize(numParameters);
   for (int i=0; i<numParameters; ++i)
   {
      mParameterSliders[i].mValue = parameters[i]->getValue();
      juce::String name = parameters[i]->getName(32);
      string label(name.getCharPointer());
      try
      {
         int append = 0;
         while (FindUIControl(label.c_str()))
         {
            ++append;
            label = name.toStdString() + ofToString(append);
         }
      }
      catch(UnknownUIControlException& e)
      {
         
      }
      mParameterSliders[i].mSlider = new FloatSlider(this, label.c_str(), 3, 35, 200, 15, &mParameterSliders[i].mValue, 0, 1);
      if (i > 0)
      {
         const int kRows = 20;
         if (i % kRows == 0)
            mParameterSliders[i].mSlider->PositionTo(mParameterSliders[i-kRows].mSlider, kAnchor_Right);
         else
            mParameterSliders[i].mSlider->PositionTo(mParameterSliders[i-1].mSlider, kAnchor_Below);
      }
      mParameterSliders[i].mParameter = parameters[i];
   }
}

void VSTPlugin::Poll()
{
   if (mDisplayMode == kDisplayMode_Sliders)
   {
      for (int i=0; i<mParameterSliders.size(); ++i)
      {
         mParameterSliders[i].mValue = mParameterSliders[i].mParameter->getValue();
      }
   }
}

void VSTPlugin::Process(double time)
{
   PROFILER(VSTPlugin);
   
   int inputChannels = MAX(2, mNumInputs);
   GetBuffer()->SetNumActiveChannels(inputChannels);
   
   ComputeSliders(0);
   SyncBuffers();
   
   const int kSafetyMaxChannels = 16; //hitting a crazy issue (memory stomp?) where numchannels is getting blown out sometimes
   
   int bufferSize = GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   juce::AudioBuffer<float> buffer(inputChannels, bufferSize);
   for (int i=0; i<inputChannels && i < kSafetyMaxChannels; ++i)
      buffer.copyFrom(i, 0, GetBuffer()->GetChannel(MIN(i,GetBuffer()->NumActiveChannels()-1)), GetBuffer()->BufferSize());
   
   if (mEnabled && mPlugin != nullptr)
   {
      mVSTMutex.lock();
      {
         const juce::ScopedLock lock(mMidiInputLock);
         
         for (int i=0; i<mChannelModulations.size(); ++i)
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
               mMidiBuffer.addEvent(juce::MidiMessage::pitchWheel(channel, (int)ofMap(bend,-mPitchBendRange,mPitchBendRange,0,16383,K(clamp))), 0);
            }
            float modWheel = mod.mModulation.modWheel ? mod.mModulation.modWheel->GetValue(0) : 0;
            if (modWheel != mod.mLastModWheel)
            {
               mod.mLastModWheel = modWheel;
               mMidiBuffer.addEvent(juce::MidiMessage::controllerEvent(channel, mModwheelCC, ofClamp(modWheel * 127,0,127)), 0);
            }
            float pressure = mod.mModulation.pressure ? mod.mModulation.pressure->GetValue(0) : 0;
            if (pressure != mod.mLastPressure)
            {
               mod.mLastPressure = pressure;
               mMidiBuffer.addEvent(juce::MidiMessage::channelPressureChange(channel, ofClamp(pressure*127,0,127)), 0);
            }
         }
         
         /*if (!mMidiBuffer.isEmpty())
         {
            ofLog() << mMidiBuffer.getFirstEventTime() << " " << mMidiBuffer.getLastEventTime();
         }*/
         
         mMidiBuffer.addEvents(mFutureMidiBuffer, 0, mFutureMidiBuffer.getLastEventTime() + 1, 0);
         mFutureMidiBuffer.clear();
         mFutureMidiBuffer.addEvents(mMidiBuffer, gBufferSize, mMidiBuffer.getLastEventTime()-gBufferSize + 1, -gBufferSize);
         mMidiBuffer.clear(gBufferSize, mMidiBuffer.getLastEventTime() + 1);
         
         mPlugin->processBlock(buffer, mMidiBuffer);
         
         mMidiBuffer.clear();
      }
      mVSTMutex.unlock();
   
      GetBuffer()->Clear();
      for (int ch=0; ch < buffer.getNumChannels() && ch < kSafetyMaxChannels; ++ch)
      {
         int outputChannel = MIN(ch,GetBuffer()->NumActiveChannels()-1);
         for (int sampleIndex=0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
         {
            GetBuffer()->GetChannel(outputChannel)[sampleIndex] += buffer.getSample(ch, sampleIndex) * mVol;
         }
         if (GetTarget())
            Add(GetTarget()->GetBuffer()->GetChannel(outputChannel), GetBuffer()->GetChannel(outputChannel), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(outputChannel), bufferSize, outputChannel);
      }
   }
   else
   {
      //bypass
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (GetTarget())
            Add(GetTarget()->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
      }
   }

   GetBuffer()->Clear();
}

void VSTPlugin::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (mPlugin == nullptr)
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
   if (mPlugin == nullptr)
      return;
   
   if (control < 0 || control > 127)
      return;
   
   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;
   
   const juce::ScopedLock lock(mMidiInputLock);
   
   mMidiBuffer.addEvent(juce::MidiMessage::controllerEvent((mUseVoiceAsChannel ? channel : mChannel), control, (uint8)value), 0);
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
   
   DrawTextNormal(GetPluginName(), 3, 32);
   
   mVolSlider->Draw();
   mProgramChangeSelector->Draw();
   mOpenEditorButton->Draw();
   
   if (mDisplayMode == kDisplayMode_Sliders)
   {
      for (auto& slider : mParameterSliders)
      {
         if (slider.mSlider)
            slider.mSlider->Draw();
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
      height = 40;
      for (auto slider : mParameterSliders)
      {
         if (slider.mSlider)
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

void VSTPlugin::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mProgramChangeSelector)
   {
      mMidiBuffer.addEvent(juce::MidiMessage::programChange(1, mProgramChange), 0);
   }
}

void VSTPlugin::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   for (int i=0; i<mParameterSliders.size(); ++i)
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
   {
      if (mPlugin != nullptr)
      {
         if (mWindow == nullptr)
            mWindow = VSTWindow::CreateWindow(this, VSTWindow::Normal);
         mWindow->toFront (true);
      }
      
      //if (mWindow->GetNSViewComponent())
      //   mWindowOverlay = new NSWindowOverlay(mWindow->GetNSViewComponent()->getView());
   }
}

void VSTPlugin::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("vst", moduleInfo, "", VSTLookup::FillVSTList);
   
   mModuleSaveData.LoadString("target", moduleInfo);
   
   mModuleSaveData.LoadInt("channel",moduleInfo,1,0,16);
   mModuleSaveData.LoadBool("usevoiceaschannel", moduleInfo, false);
   mModuleSaveData.LoadFloat("pitchbendrange",moduleInfo,2,1,96,K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)",moduleInfo,1,0,127,K(isTextField));
   
   SetUpFromSaveData();
}

void VSTPlugin::SetUpFromSaveData()
{
   string vstName = mModuleSaveData.GetString("vst");
   if (vstName != "")
      SetVST(vstName);
   
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   
   mChannel = mModuleSaveData.GetInt("channel");
   mUseVoiceAsChannel = mModuleSaveData.GetBool("usevoiceaschannel");
   mPitchBendRange = mModuleSaveData.GetFloat("pitchbendrange");
   mModwheelCC = mModuleSaveData.GetInt("modwheelcc(1or74)");
}

namespace
{
   const int kSaveStateRev = 0;
}

void VSTPlugin::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   if (mPlugin)
   {
      out << true;
      juce::MemoryBlock vstState;
      mPlugin->getStateInformation(vstState);
      out << (int)vstState.getSize();
      out.WriteGeneric(vstState.getData(), (int)vstState.getSize());
   }
   else
   {
      out << false;
   }
}

void VSTPlugin::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev == kSaveStateRev);
   
   bool hasPlugin;
   in >> hasPlugin;
   if (hasPlugin)
   {
      int size;
      in >> size;
      char* data = new char[size];
      in.ReadGeneric(data, size);
      if (mPlugin != nullptr)
      {
         ofLog() << "loading vst state for " << mPlugin->getName();
         mPlugin->setStateInformation(data, size);
      }
      else
      {
         TheSynth->LogEvent("Couldn't instantiate plugin to load state for "+mModuleSaveData.GetString("vst"), kLogEventType_Error);
      }
   }
}
