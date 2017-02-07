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
   
   typedef string VstDirExtPair[2];
   const int kNumVstTypes = 1;
   //const VstDirExtPair vstDirs[kNumVstTypes] =
   //                                 {{"/Library/Audio/Plug-Ins/VST3","vst3"},
   //                                  {"/Library/Audio/Plug-Ins/VST","vst"}};
   const VstDirExtPair vstDirs[kNumVstTypes] = {{"/Library/Audio/Plug-Ins/VST","vst"}};
   
   void FillVSTList(DropdownList* list)
   {
      assert(list);
      vector<string> vsts;
      for (int i=0; i<kNumVstTypes; ++i)
      {
         const VstDirExtPair& pair = vstDirs[i];
         string dirPath = pair[0];
         string ext = pair[1];
         DirectoryIterator dir(File(dirPath), true, "*."+ext, File::findFilesAndDirectories);
         while(dir.next())
         {
            File file = dir.getFile();
            vsts.push_back(file.getRelativePathFrom(File(dirPath)).toStdString());
         }
      }
      for (int i=0; i<vsts.size(); ++i)
         list->AddLabel(vsts[i].c_str(), i);
   }
   
   string GetVSTPath(string vstName)
   {
      for (int i=0; i<kNumVstTypes; ++i)
      {
         const VstDirExtPair& pair = vstDirs[i];
         string dirPath = pair[0];
         string ext = pair[1];
         if (ofIsStringInString(vstName, ext))
            return dirPath+"/"+vstName;
      }
      return "";
   }
}

VSTPlugin::VSTPlugin()
: mVol(1)
, mVolSlider(NULL)
, mPlugin(nullptr)
, mChannel(1)
, mPitchBendRange(2)
, mModwheelCC(1)  //or 74 in Multidimensional Polyphonic Expression (MPE) spec
, mUseVoiceAsChannel(false)
, mProgramChangeSelector(NULL)
, mProgramChange(0)
, mOpenEditorButton(NULL)
, mWindowOverlay(NULL)
, mDisplayMode(kDisplayMode_PluginOverlay)
{
   mWriteBuffer = new float[gBufferSize];
   mInputBufferSize = gBufferSize;
   mInputBuffer = new float[mInputBufferSize];
   Clear(mInputBuffer, mInputBufferSize);
   
   mFormatManager.addDefaultFormats();
   
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
   delete[] mWriteBuffer;
   delete[] mInputBuffer;
}

float* VSTPlugin::GetBuffer(int& bufferSize)
{
   bufferSize = mInputBufferSize;
   return mInputBuffer;
}

void VSTPlugin::Exit()
{
   if (mWindow)
   {
      VSTWindow* window = mWindow.release();
      delete window;
   }
}

void VSTPlugin::SetVST(string path)
{
   if (mPlugin != NULL && mPlugin->getPluginDescription().fileOrIdentifier == path)
      return;  //this VST is already loaded! we're all set
   
   if (mPlugin != NULL && mWindow != NULL)
   {
      VSTWindow* window = mWindow.release();
      delete window;
      delete mWindowOverlay;
      mWindowOverlay = NULL;
   }
   
   juce::PluginDescription desc;
   desc.fileOrIdentifier = path;
   desc.uid = 0;
   
   juce::String errorMessage;
   mVSTMutex.lock();
   for (int i=0; i<mFormatManager.getNumFormats(); ++i)
   {
      if (mFormatManager.getFormat(i)->fileMightContainThisPluginType(path))
         mPlugin = mFormatManager.getFormat(i)->createInstanceFromDescription(desc, gSampleRate, gBufferSize);
   }
   assert(mPlugin != NULL);
   mPlugin->prepareToPlay(gSampleRate, gBufferSize);
   mPlugin->setPlayHead(&mPlayhead);
   mNumInputs = mPlugin->getTotalNumInputChannels();
   mNumOutputs = mPlugin->getTotalNumOutputChannels();
   ofLog() << "vst inputs: " << mNumInputs << "  vst outputs: " << mNumOutputs;
   mVSTMutex.unlock();
   
   CreateParameterSliders();
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
   
   mParameterSliders.resize(mPlugin->getNumParameters());
   for (int i=0; i<mPlugin->getNumParameters(); ++i)
   {
      mParameterSliders[i].mValue = mPlugin->getParameter(i);
      juce::String name = mPlugin->getParameterName(i);
      string label(name.getCharPointer());
      try
      {
         while (FindUIControl(label.c_str()))
            label += ".";
      }
      catch(UnknownUIControlException& e)
      {
         
      }
      mParameterSliders[i].mSlider = new FloatSlider(this, label.c_str(), 3, 20, 200, 15, &mParameterSliders[i].mValue, 0, 1);
      if (i > 0)
      {
         const int kRows = 20;
         if (i % kRows == 0)
            mParameterSliders[i].mSlider->PositionTo(mParameterSliders[i-kRows].mSlider, kAnchorDirection_Right);
         else
            mParameterSliders[i].mSlider->PositionTo(mParameterSliders[i-1].mSlider, kAnchorDirection_Below);
      }
      mParameterSliders[i].mParameterIndex = i;
   }
}

void VSTPlugin::Poll()
{
   if (mDisplayMode == kDisplayMode_Sliders)
   {
      for (int i=0; i<mParameterSliders.size(); ++i)
      {
         mParameterSliders[i].mValue = mPlugin->getParameter(mParameterSliders[i].mParameterIndex);
      }
   }
}

void VSTPlugin::Process(double time)
{
   Profiler profiler("VSTPlugin");
   
   if (!mEnabled || GetTarget() == NULL || mPlugin == NULL)
      return;
   
   ComputeSliders(0);
   
   int bufferSize;
   float* out = GetTarget()->GetBuffer(bufferSize);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, gBufferSize);
   
   int inputChannels = MAX(2, mNumInputs);
   juce::AudioBuffer<float> buffer(inputChannels, gBufferSize);
   for (int i=0; i<inputChannels; ++i)
      buffer.copyFrom(i, 0, mInputBuffer, mInputBufferSize);
   
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
         
         float bend = mod.mPitchBend ? mod.mPitchBend->GetValue(0) : 0;
         if (bend != mod.mLastPitchBend)
         {
            mod.mLastPitchBend = bend;
            mMidiBuffer.addEvent(juce::MidiMessage::pitchWheel(channel, (int)ofMap(bend,-mPitchBendRange,mPitchBendRange,0,16383,K(clamp))), 0);
         }
         float modWheel = mod.mModWheel ? mod.mModWheel->GetValue(0) : 0;
         if (modWheel != mod.mLastModWheel)
         {
            mod.mLastModWheel = modWheel;
            mMidiBuffer.addEvent(juce::MidiMessage::controllerEvent(channel, mModwheelCC, ofClamp(modWheel * 127,0,127)), 0);
         }
         float pressure = mod.mPressure ? mod.mPressure->GetValue(0) : 0;
         if (pressure != mod.mLastPressure)
         {
            mod.mLastPressure = pressure;
            mMidiBuffer.addEvent(juce::MidiMessage::channelPressureChange(channel, ofClamp(pressure*127,0,127)), 0);
         }
      }
      
      mPlugin->processBlock(buffer, mMidiBuffer);
      
      mMidiBuffer.clear();
   }
   mVSTMutex.unlock();
   
   for (int channel=0; channel < buffer.getNumChannels(); ++channel)
   {
      for (int sampleIndex=0; sampleIndex < buffer.getNumSamples(); ++sampleIndex)
      {
         mWriteBuffer[sampleIndex] += buffer.getSample(channel, sampleIndex) * mVol;
      }
   }
   
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize);
   
   Add(out, mWriteBuffer, bufferSize);
   
   Clear(mInputBuffer, mInputBufferSize);
}

void VSTPlugin::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationChain* pitchBend /*= NULL*/, ModulationChain* modWheel /*= NULL*/, ModulationChain* pressure /*= NULL*/)
{
   if (mPlugin == NULL)
      return;
   
   if (pitch < 0 || pitch > 127)
      return;
   
   int channel = voiceIdx + 1;
   if (voiceIdx == -1)
      channel = 1;
   
   const juce::ScopedLock lock(mMidiInputLock);
   
   if (velocity > 0)
   {
      mMidiBuffer.addEvent(juce::MidiMessage::noteOn(mUseVoiceAsChannel ? channel : mChannel, pitch, (uint8)velocity), 0);
      //ofLog() << "+ vst note on: " << (mUseVoiceAsChannel ? channel : mChannel) << " " << pitch << " " << (uint8)velocity;
   }
   else
   {
      mMidiBuffer.addEvent(juce::MidiMessage::noteOff(mUseVoiceAsChannel ? channel : mChannel, pitch), 0);
      //ofLog() << "- vst note off: " << (mUseVoiceAsChannel ? channel : mChannel) << " " << pitch;
   }
   
   int modIdx = voiceIdx;
   if (voiceIdx == -1)
      modIdx = kGlobalModulationIdx;
   
   mChannelModulations[modIdx].mPitchBend = pitchBend;
   mChannelModulations[modIdx].mModWheel = modWheel;
   mChannelModulations[modIdx].mPressure = pressure;
}

void VSTPlugin::SendCC(int control, int value, int voiceIdx /*=-1*/)
{
   if (mPlugin == NULL)
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
   if (mDisplayMode == kDisplayMode_PluginOverlay && mWindowOverlay)
   {
      //mWindowOverlay->GetDimensions(mOverlayWidth, mOverlayHeight);
      /*if (mWindow)
      {
         mOverlayWidth = 500;
         mOverlayHeight = 500;
         float retinaFactor = gIsRetina ? 2 : 1;
         float contentMult = gDrawScale/retinaFactor;
         float width = mOverlayWidth * contentMult;
         float height = mOverlayHeight * contentMult;
         mWindow->setSize(width, height);
      }*/
      //mWindowOverlay->UpdatePosition(this);
   }
}

void VSTPlugin::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   if (mPlugin)
      DrawText(mPlugin->getName().toStdString(), 200, 15);
   
   mVolSlider->Draw();
   mProgramChangeSelector->Draw();
   mOpenEditorButton->Draw();
   
   if (mDisplayMode == kDisplayMode_Sliders)
   {
      for (auto& slider : mParameterSliders)
         slider.mSlider->Draw();
   }
}

void VSTPlugin::GetModuleDimensions(int& width, int& height)
{
   if (mDisplayMode == kDisplayMode_PluginOverlay)
   {
      if (mWindowOverlay)
      {
         width = mOverlayWidth;
         height = mOverlayHeight+20;
      }
      else
      {
         width = 300;
         height = 30;
      }
   }
   else
   {
      width = 250;
      height = 350;
      if (!mParameterSliders.empty())
      {
         FloatSlider* slider = mParameterSliders[mParameterSliders.size()-1].mSlider;
         width = slider->GetRect(true).x + slider->GetRect(true).width + 3;
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
         mPlugin->setParameter(mParameterSliders[i].mParameterIndex, mParameterSliders[i].mValue);
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
      if (mWindow == NULL)
         mWindow = VSTWindow::CreateWindow(this, VSTWindow::Normal);
      mWindow->toFront (true);
      
      //if (mWindow->GetNSViewComponent())
      //   mWindowOverlay = new NSWindowOverlay(mWindow->GetNSViewComponent()->getView());
   }
}

void VSTPlugin::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("vst", moduleInfo, "", FillVSTList);
   
   mModuleSaveData.LoadString("target", moduleInfo);
   
   mModuleSaveData.LoadInt("channel",moduleInfo,1,0,16);
   mModuleSaveData.LoadBool("usevoiceaschannel", moduleInfo, false);
   mModuleSaveData.LoadFloat("pitchbendrange",moduleInfo,2,1,24,K(isTextField));
   mModuleSaveData.LoadInt("modwheelcc(1or74)",moduleInfo,1,0,127,K(isTextField));
   
   SetUpFromSaveData();
}

void VSTPlugin::SetUpFromSaveData()
{
   string vstName = mModuleSaveData.GetString("vst");
   if (vstName != "")
      SetVST(GetVSTPath(vstName));
   
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
      out.Write(vstState.getData(), vstState.getSize());
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
      assert(mPlugin != NULL);
      int size;
      in >> size;
      char* data = new char[size];
      in.Read(data, size);
      mPlugin->setStateInformation(data, size);
   }
}
