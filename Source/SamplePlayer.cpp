/*
  ==============================================================================

    SamplePlayer.cpp
    Created: 19 Oct 2017 10:10:15pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SamplePlayer.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SampleBank.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "FillSaveDropdown.h"
#include "PatchCableSource.h"
#include "Scale.h"

SamplePlayer::SamplePlayer()
: mVolume(1)
, mVolumeSlider(nullptr)
, mSpeed(1)
, mSpeedSlider(nullptr)
, mSample(nullptr)
, mSampleIndex(-1)
, mSampleList(nullptr)
, mPlay(false)
, mLoop(false)
, mLoopCheckbox(nullptr)
, mBank(nullptr)
, mSampleBankCable(nullptr)
, mDrawBuffer(0)
, mPlayButton(nullptr)
, mPauseButton(nullptr)
, mStopButton(nullptr)
, mDownloadYoutubeButton(nullptr)
, mScrubbingSample(false)
, mOscWheelGrabbed(false)
, mOscWheelSpeed(0)
, mPlaySpeed(1)
, mWidth(210)
, mHeight(125)
{
}

void SamplePlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",20,3,90,15,&mVolume,0,2);
   mSpeedSlider = new FloatSlider(this,"speed",mVolumeSlider, kAnchor_Right,90,15,&mSpeed,-2,2);
   mSampleList = new DropdownList(this,"samples",5,20,&mSampleIndex);

   mPlayButton = new ClickButton(this,"play",5,40);
   mPauseButton = new ClickButton(this,"pause",mPlayButton, kAnchor_Right);
   mStopButton = new ClickButton(this,"stop",mPauseButton, kAnchor_Right);
   mLoopCheckbox = new Checkbox(this,"loop",mStopButton, kAnchor_Right,&mLoop);
   mDownloadYoutubeButton = new ClickButton(this,"youtube",mLoopCheckbox, kAnchor_Right);
   
   mSampleBankCable = new PatchCableSource(this, kConnectionType_Special);
   mSampleBankCable->SetManualPosition(8, 8);
   mSampleBankCable->AddTypeFilter("samplebank");
   AddPatchCableSource(mSampleBankCable);
}

SamplePlayer::~SamplePlayer()
{
   if (mOwnsSample)
      delete mSample;
}

void SamplePlayer::Init()
{
   IDrawableModule::Init();
   
   if (OSCReceiver::connect(12345))
      OSCReceiver::addListener(this);
}

void SamplePlayer::Poll()
{
   IDrawableModule::Poll();
   
   juce::String clipboard = SystemClipboard::getTextFromClipboard();
   if (clipboard.contains("youtube"))
   {
      juce::String clipId = clipboard.substring(clipboard.indexOf("v=")+2, clipboard.length());
      mYoutubeId = clipId.toStdString();
      mDownloadYoutubeButton->SetShowing(true);
   }
   else
   {
      mYoutubeId = "";
      mDownloadYoutubeButton->SetShowing(false);
   }
}

void SamplePlayer::Process(double time)
{
   PROFILER(SamplePlayer);
   
   if (!mEnabled || GetTarget() == nullptr || mSample == nullptr)
      return;
   
   ComputeSliders(0);
   SyncOutputBuffer(mSample->NumChannels());
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);
   
   float volSq = mVolume * mVolume * .25f;
   
   const float kBlendSpeed = .02f;
   if (mOscWheelGrabbed)
   {
      mPlaySpeed = ofLerp(mPlaySpeed, mOscWheelSpeed, kBlendSpeed);
      mPlaySpeed = ofClamp(mPlaySpeed, -5, 5);
   }
   else
   {
      mPlaySpeed = ofLerp(mPlaySpeed, mSpeed, kBlendSpeed);
   }
   mSample->SetRate(mPlaySpeed);
   
   gWorkChannelBuffer.SetNumActiveChannels(mSample->NumChannels());
   if (mPlay && mSample->ConsumeData(&gWorkChannelBuffer, bufferSize, true))
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
      {
         Mult(gWorkChannelBuffer.GetChannel(ch), volSq, bufferSize);
         Add(GetTarget()->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      }
   }
   else
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
         GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, ch);
   }
}

void SamplePlayer::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mSampleBankCable)
   {
      mBank = dynamic_cast<SampleBank*>(mSampleBankCable->GetTarget());
      
      UpdateSampleList();
   }
}

void SamplePlayer::UpdateSampleList()
{
   if (mBank == nullptr)
      return;
   
   mSampleList->Clear();
   vector<SampleInfo> samples = mBank->GetSamples();
   for (int i=0; i<samples.size(); ++i)
   {
      mSampleList->AddLabel(samples[i].mSample->Name(), i);
   }
}

void SamplePlayer::DropdownClicked(DropdownList* list)
{
   if (list == mSampleList)
   {
      UpdateSampleList();
   }
}

void SamplePlayer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mSampleList)
   {
      UpdateSample(mBank->GetSampleInfo(mSampleIndex).mSample, false);
   }
}

void SamplePlayer::FilesDropped(vector<string> files, int x, int y)
{
   Sample* sample = new Sample();
   sample->Read(files[0].c_str());
   UpdateSample(sample, true);
}

void SamplePlayer::UpdateSample(Sample* sample, bool ownsSample)
{
   if (mOwnsSample)
      delete mSample;
   
   mSample = sample;
   mSample->SetPlayPosition(0);
   mSample->SetLooping(mLoop);
   mSample->SetRate(mSpeed);
   mVolume = 1;
   mPlay = false;
   mOwnsSample = ownsSample;
   
   mSample->LockDataMutex(true);
   mDrawBuffer.Resize(mSample->LengthInSamples());
   mDrawBuffer.CopyFrom(mSample->Data());
   mSample->LockDataMutex(false);
}

void SamplePlayer::ButtonClicked(ClickButton *button)
{
   if (button == mPlayButton)
      mPlay = true;
   if (button == mPauseButton)
      mPlay = false;
   if (button == mStopButton)
   {
      mPlay = false;
      mSample->SetPlayPosition(0);
   }
   if (button == mDownloadYoutubeButton)
   {
      char command[2048];
      sprintf(command, "export PATH=/opt/local/bin:$PATH; youtube-dl %s -x --audio-format wav -o %s -w", mYoutubeId.c_str(), ofToDataPath("youtube.m4a").c_str());
      FILE* output = popen(command, "r");
      
      char c;
      do
      {
         c = fgetc(output);
         printf("%c",c);
      } while (c != EOF);
      
      pclose(output);
      
      Sample* sample = new Sample();
      sample->Read(ofToDataPath("youtube.wav").c_str());
      UpdateSample(sample, true);
   }
}

void SamplePlayer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (y > 60 && y < mHeight - 20 && mSample != nullptr)
   {
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
      mScrubbingSample = true;
   }
}

bool SamplePlayer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   if (mScrubbingSample && mSample != nullptr)
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
   return true;
}

void SamplePlayer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mScrubbingSample = false;
}

float SamplePlayer::GetPlayPositionForMouse(float mouseX) const
{
   if (mSample != nullptr)
      return ofMap(mouseX, 5, mWidth-10 ,0, mSample->LengthInSamples(), true);
   return 0;
}

void SamplePlayer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mSpeedSlider->Draw();
   mSampleList->Draw();
   mLoopCheckbox->Draw();
   mPlayButton->Draw();
   mPauseButton->Draw();
   mStopButton->Draw();
   mDownloadYoutubeButton->Draw();

   ofPushMatrix();
   ofTranslate(5,60);
   if (mSample)
   {
      DrawAudioBuffer(mWidth-10, mHeight - 65, &mDrawBuffer, 0, mDrawBuffer.BufferSize(), mSample->GetPlayPosition());
   }
   else
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,255,255,50);
      ofRect(0, 0, mWidth-10, mHeight - 65);
      ofPopStyle();
   }
   ofPopMatrix();
}

void SamplePlayer::oscMessageReceived(const OSCMessage& msg)
{
   if (msg.getAddressPattern().toString() == "/wheel/z")
   {
      bool grabbed = msg[0].getFloat32() > 0;
      if (grabbed)
      {
         mOscWheelPos = FLT_MAX;
         mOscWheelSpeed = 0;
         mOscWheelGrabbed = true;
      }
      else
      {
         mOscWheelGrabbed = false;
      }
   }
   else if (msg.getAddressPattern().toString() == "/wheel/x")
   {
      float pos = msg[0].getFloat32();
      if (mOscWheelPos == FLT_MAX)
      {
         mOscWheelPos = pos;
      }
      
      mOscWheelSpeed = (pos - mOscWheelPos) * 70;
      mOscWheelPos = pos;
   }
   else if (msg.getAddressPattern().toString() == "/Fader/x")
   {
      float pos = msg[0].getFloat32();
      mSpeed = ofLerp(mSpeedSlider->GetMin(), mSpeedSlider->GetMax(), pos);
   }
}

void SamplePlayer::oscBundleReceived(const OSCBundle& bundle)
{
   for (OSCBundle::Element* element = bundle.begin(); element != bundle.end(); ++element)
   {
      if (element->isMessage())
         oscMessageReceived(element->getMessage());
      else if (element->isBundle())
         oscBundleReceived(element->getBundle());
   }
}

void SamplePlayer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mLoopCheckbox)
   {
      if (mSample != nullptr)
         mSample->SetLooping(mLoop);
   }
}

void SamplePlayer::GetModuleDimensions(int& x, int&y)
{
   x = mWidth;
   y = mHeight;
}

void SamplePlayer::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SamplePlayer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SamplePlayer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadString("samplebank", moduleInfo,"",FillDropdown<SampleBank*>);
   mModuleSaveData.LoadFloat("width", moduleInfo, 210);
   mModuleSaveData.LoadFloat("height", moduleInfo, 125);
   
   SetUpFromSaveData();
}

void SamplePlayer::SaveLayout(ofxJSONElement& moduleInfo)
{
   IDrawableModule::SaveLayout(moduleInfo);
   moduleInfo["samplebank"] = mBank ? mBank->Name() : "";
   moduleInfo["width"] = mWidth;
   moduleInfo["height"] = mHeight;
}

void SamplePlayer::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
   mSampleBankCable->SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("samplebank"),false));
   Resize(mModuleSaveData.GetFloat("width"), mModuleSaveData.GetFloat("height"));
}

namespace
{
   const int kSaveStateRev = 0;
}

void SamplePlayer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   bool hasSample = (mSample != nullptr);
   out << hasSample;
   if (hasSample)
      mSample->SaveState(out);
}

void SamplePlayer::LoadState(FileStreamIn& in)
{
   IDrawableModule::LoadState(in);
   
   int rev;
   in >> rev;
   LoadStateValidate(rev <= kSaveStateRev);
   
   bool hasSample;
   in >> hasSample;
   if (hasSample)
   {
      Sample* sample = new Sample();
      sample->LoadState(in);
      UpdateSample(sample, true);
   }
}
