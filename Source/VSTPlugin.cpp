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

//static
bool VSTPlugin::sIsRescanningVsts = false;

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
         VSTPlugin::sIsRescanningVsts = true;
         sPluginList.clear();
         juce::File deadMansPedalFile(ofToDataPath("vst/deadmanspedal.txt"));
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
         sPluginList.createXml()->writeTo(juce::File(ofToDataPath("vst/found_vsts.xml")));
         VSTPlugin::sIsRescanningVsts = false;
      }
      else
      {
         auto file = juce::File(ofToDataPath("vst/found_vsts.xml"));
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
      
      if (juce::File(ofToDataPath("vst/used_vsts.json")).existsAsFile())
      {
         ofxJSONElement root;
         root.open(ofToDataPath("vst/used_vsts.json"));
         ofxJSONElement jsonList = root["vsts"];

         for (auto it = jsonList.begin(); it != jsonList.end(); ++it)
         {
            string key = it.key().asString();
            lastUsedTimes[key] = jsonList[key].asDouble();
         }
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
, mPluginReady(false)
, mPlugin(nullptr)
, mNumInputs(2)
, mNumOutputs(2)
, mChannel(1)
, mPitchBendRange(2)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
, mUseVoiceAsChannel(false)
, mPresetFileSelector(nullptr)
, mPresetFileIndex(-1)
, mOpenEditorButton(nullptr)
//, mWindowOverlay(nullptr)
, mDisplayMode(kDisplayMode_Sliders)
, mShowParameterIndex(-1)
, mTemporarilyDisplayedParamIndex(-1)
{
   juce::File(ofToDataPath("vst")).createDirectory();
   juce::File(ofToDataPath("vst/presets")).createDirectory();
   
   if (VSTLookup::sFormatManager.getNumFormats() == 0)
      VSTLookup::sFormatManager.addDefaultFormats();
   
   mChannelModulations.resize(kGlobalModulationIdx+1);
}

void VSTPlugin::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolSlider = new FloatSlider(this,"vol",3,3,80,15,&mVol,0,4);
   mOpenEditorButton = new ClickButton(this, "open", mVolSlider, kAnchor_Right_Padded);
   mPresetFileSelector = new DropdownList(this,"preset",3,21,&mPresetFileIndex,110);
   mSavePresetFileButton = new ClickButton(this,"save as",-1,-1);
   mShowParameterDropdown = new DropdownList(this,"show parameter",3,38,&mShowParameterIndex);
   
   mPresetFileSelector->DrawLabel(true);
   mSavePresetFileButton->PositionTo(mPresetFileSelector,kAnchor_Right);
   
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

string VSTPlugin::GetPluginId()
{
   if (mPlugin)
   {
      const auto& desc = dynamic_cast<juce::AudioPluginInstance*>(mPlugin.get())->getPluginDescription();
      return mPlugin->getName().toStdString() + "_" + ofToString(desc.uid);
   }
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
      root.open(ofToDataPath("vst/used_vsts.json"));
      
      Time time = Time::getCurrentTime();
      root["vsts"][path] = (double)time.currentTimeMillis();

      root.save(ofToDataPath("vst/used_vsts.json"), true);
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
   
   auto types = VSTLookup::sPluginList.getTypes();
   bool found = false;
   for (int i=0; i<types.size(); ++i)
   {
      if (path == types[i].fileOrIdentifier)
      {
         found = true;
         PluginDescription desc = types[i];
         LoadVST(desc);
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
            PluginDescription desc = types[i];
            LoadVST(desc);
            break;
         }
      }
   }
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

         sFormatManager.getFormat(i)->createPluginInstanceAsync(desc, gSampleRate, gBufferSize, completionCallback);*/

   mVSTMutex.lock();
   juce::String errorMessage;
   mPlugin = VSTLookup::sFormatManager.createPluginInstance(desc, gSampleRate, gBufferSize, errorMessage);
   if (mPlugin != nullptr)
   {
      mPlugin->enableAllBuses();
      mPlugin->prepareToPlay(gSampleRate, gBufferSize);
      mPlugin->setPlayHead(&mPlayhead);
      mNumInputs = MIN(mPlugin->getTotalNumInputChannels(), 4);
      mNumOutputs = MIN(mPlugin->getTotalNumOutputChannels(), 4);
      ofLog() << "vst inputs: " << mNumInputs << "  vst outputs: " << mNumOutputs;

      CreateParameterSliders();
      
      mPluginReady = true;
   }
   else
   {
      TheSynth->LogEvent("error loading VST: " + errorMessage.toStdString(), kLogEventType_Error);
   }
   mVSTMutex.unlock();
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
   
   mShowParameterDropdown->Clear();
   
   const auto& parameters = mPlugin->getParameters();
   
   int numParameters = MIN(1000, parameters.size());
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
      mParameterSliders[i].mSlider = new FloatSlider(this, label.c_str(), -1, -1, 200, 15, &mParameterSliders[i].mValue, 0, 1);
      mParameterSliders[i].mParameter = parameters[i];
      mParameterSliders[i].mShowing = false;
      if (numParameters <= 20)   //only show parameters in list if there are a small number. if there are many, make the user adjust them in the VST before they can be controlled
      {
         mShowParameterDropdown->AddLabel(label.c_str(), i);
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
      for (int i=0; i<mParameterSliders.size(); ++i)
      {
         float value = mParameterSliders[i].mParameter->getValue();
         if (mParameterSliders[i].mValue != value)
         {
            mParameterSliders[i].mValue = value;
            if (!mParameterSliders[i].mInSelectorList && mTemporarilyDisplayedParamIndex != i)
            {
               if (mTemporarilyDisplayedParamIndex != -1)
                  mShowParameterDropdown->RemoveLabel(mTemporarilyDisplayedParamIndex);
               mTemporarilyDisplayedParamIndex = i;
               mShowParameterDropdown->AddLabel(mParameterSliders[i].mSlider->Name(), i);
            }
         }
      }
   }
}

void VSTPlugin::Process(double time)
{
   if (!mPluginReady)
      return;
   
#if BESPOKE_LINUX //HACK: weird race condition, which this seems to fix for now
   if (mPlugin == nullptr)
      return;
#endif

   PROFILER(VSTPlugin);
   
   int inputChannels = MAX(2, mNumInputs);
   GetBuffer()->SetNumActiveChannels(inputChannels);
   
   SyncBuffers();
   
   const int kSafetyMaxChannels = 16; //hitting a crazy issue (memory stomp?) where numchannels is getting blown out sometimes
   
   int bufferSize = GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   juce::AudioBuffer<float> buffer(inputChannels, bufferSize);
   for (int i=0; i<inputChannels && i < kSafetyMaxChannels; ++i)
      buffer.copyFrom(i, 0, GetBuffer()->GetChannel(MIN(i,GetBuffer()->NumActiveChannels()-1)), GetBuffer()->BufferSize());

   IAudioReceiver* target = GetTarget();
   
   if (mEnabled && mPlugin != nullptr)
   {
      mVSTMutex.lock();

      ComputeSliders(0);

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
         if (target)
            Add(target->GetBuffer()->GetChannel(outputChannel), GetBuffer()->GetChannel(outputChannel), bufferSize);
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(outputChannel), bufferSize, outputChannel);
      }
   }
   else
   {
      //bypass
      for (int ch=0; ch<GetBuffer()->NumActiveChannels(); ++ch)
      {
         if (target)
            Add(target->GetBuffer()->GetChannel(ch), GetBuffer()->GetChannel(ch), GetBuffer()->BufferSize());
         GetVizBuffer()->WriteChunk(GetBuffer()->GetChannel(ch),GetBuffer()->BufferSize(), ch);
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
   mShowParameterDropdown->Draw();
   
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

vector<IUIControl*> VSTPlugin::ControlsToIgnoreInSaveState() const
{
   vector<IUIControl*> ignore;
   ignore.push_back(mPresetFileSelector);
   return ignore;
}

namespace
{
   const int kSaveStateRev = 1;
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

         unique_ptr<FileInputStream> input(resourceFile.createInputStream());

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
      }
   }
   
   if (list == mShowParameterDropdown)
   {
      mParameterSliders[mShowParameterIndex].mShowing = true;
      mParameterSliders[mShowParameterIndex].mInSelectorList = true;
      mShowParameterIndex = -1;
      mTemporarilyDisplayedParamIndex = -1;
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
   
   if (button == mSavePresetFileButton && mPlugin != nullptr)
   {
      juce::File(ofToDataPath("vst/presets/"+GetPluginId())).createDirectory();
      FileChooser chooser("Save preset as...", File(ofToDataPath("vst/presets/"+ GetPluginId()+"/preset.vstp")), "*.vstp", true, false, TheSynth->GetMainComponent()->getTopLevelComponent());
      if (chooser.browseForFileToSave(true))
      {
         string path = chooser.getResult().getFullPathName().toStdString();
         
         File resourceFile (path);
         TemporaryFile tempFile (resourceFile);

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
            
            output.writeInt(kSaveStateRev);
            output.writeInt64(vstState.getSize());
            output.write(vstState.getData(), vstState.getSize());
            output.writeInt64(vstProgramState.getSize());
            if (vstProgramState.getSize() > 0)
               output.write(vstProgramState.getData(), vstProgramState.getSize());
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
         
         for (size_t i=0; i<mPresetFilePaths.size(); ++i)
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
   
   juce::File(ofToDataPath("vst/presets/"+ GetPluginId())).createDirectory();
   DirectoryIterator dir(File(ofToDataPath("vst/presets/"+ GetPluginId())), false);
   mPresetFilePaths.clear();
   mPresetFileSelector->Clear();
   while(dir.next())
   {
      File file = dir.getFile();
      if (file.getFileExtension() ==  ".vstp")
      {
         mPresetFileSelector->AddLabel(file.getFileName().toStdString(), (int)mPresetFilePaths.size());
         mPresetFilePaths.push_back(file.getFullPathName().toStdString());
      }
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

      juce::MemoryBlock vstProgramState;
      mPlugin->getCurrentProgramStateInformation(vstProgramState);
      out << (int)vstProgramState.getSize();
      if (vstProgramState.getSize() > 0)
         out.WriteGeneric(vstProgramState.getData(), (int)vstProgramState.getSize());
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
   LoadStateValidate(rev <= kSaveStateRev);
   
   bool hasPlugin;
   in >> hasPlugin;
   if (hasPlugin)
   {
      int vstStateSize;
      in >> vstStateSize;
      char* vstState = new char[vstStateSize];
      in.ReadGeneric(vstState, vstStateSize);

      int vstProgramStateSize;
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
         TheSynth->LogEvent("Couldn't instantiate plugin to load state for "+mModuleSaveData.GetString("vst"), kLogEventType_Error);
      }
   }
}
