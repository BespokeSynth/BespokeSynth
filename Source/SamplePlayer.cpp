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
#include "UIControlMacros.h"

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
, mWidth(300)
, mHeight(150)
, mNoteInputBuffer(this)
, mAdsr(10,1,1,10)
{
   mYoutubeSearch[0] = 0;
   mSampleCuePoints[0].speed = 1;
}

void SamplePlayer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   UIBLOCK0();
   UIBLOCK_SHIFTX(20);
   FLOATSLIDER(mVolumeSlider, "volume",&mVolume,0,2); UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mSpeedSlider,"speed",&mSpeed,-2,2); UIBLOCK_NEWLINE();
   DROPDOWN(mSampleList,"samples",&mSampleIndex, 100); UIBLOCK_SHIFTRIGHT();
   TEXTENTRY(mDownloadYoutubeSearch,"yt:",30,mYoutubeSearch); UIBLOCK_NEWLINE();
   mDownloadYoutubeSearch->DrawLabel(true);
   mDownloadYoutubeSearch->SetRequireEnter(true);
   BUTTON(mPlayButton,"play"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mPauseButton,"pause"); UIBLOCK_SHIFTRIGHT();
   BUTTON(mStopButton,"stop"); UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mLoopCheckbox,"loop",&mLoop); UIBLOCK_SHIFTRIGHT();
   BUTTON(mDownloadYoutubeButton,"youtube");
   UIBLOCK_SHIFTX(140);
   UIBLOCK_NEWCOLUMN();
   //UIBLOCK_SAVEPOSITION();
   for (int i=0; i<(int)mSampleCuePoints.size(); ++i)
   {
      //UIBLOCK_RESTOREPOSITION();
      FLOATSLIDER_DIGITS(mSampleCuePoints[i].mStartSlider, ("start"+ofToString(i)).c_str(), &mSampleCuePoints[i].startSeconds, 0, 100, 3);
      FLOATSLIDER_DIGITS(mSampleCuePoints[i].mLengthSlider, ("length"+ofToString(i)).c_str(), &mSampleCuePoints[i].lengthSeconds, 0, 100, 3);
      FLOATSLIDER(mSampleCuePoints[i].mSpeedSlider, ("speed"+ofToString(i)).c_str(), &mSampleCuePoints[i].speed, 0, 2);
      UIBLOCK_NEWCOLUMN();
   }
   ENDUIBLOCK0();
   
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
   
   mNoteInputBuffer.Process(time);
   
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
   if (mPlay && mSample->ConsumeData(time, &gWorkChannelBuffer, bufferSize, true))
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
            gWorkChannelBuffer.GetChannel(ch)[i] *= volSq * mAdsr.Value(time + i * gInvSampleRateMs);
         Add(GetTarget()->GetBuffer()->GetChannel(ch), gWorkChannelBuffer.GetChannel(ch), bufferSize);
         GetVizBuffer()->WriteChunk(gWorkChannelBuffer.GetChannel(ch), bufferSize, ch);
      }
   }
   else
   {
      for (int ch=0; ch<gWorkChannelBuffer.NumActiveChannels(); ++ch)
         GetVizBuffer()->WriteChunk(gZeroBuffer, bufferSize, ch);
      mAdsr.Stop(time);
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

void SamplePlayer::PlayNote(double time, int pitch, int velocity, int voiceIdx /*= -1*/, ModulationParameters modulation /*= ModulationParameters()*/)
{
   if (!NoteInputBuffer::IsTimeWithinFrame(time))
   {
      mNoteInputBuffer.QueueNote(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (velocity > 0)
   {
      float startSeconds, lengthSeconds, speed;
      GetPlayInfoForPitch(pitch + (modulation.pitchBend ? modulation.pitchBend->GetValue(0) : 0), startSeconds, lengthSeconds, speed);
      mSample->SetPlayPosition(startSeconds * gSampleRate * mSample->GetSampleRateRatio());
      mPlay = true;
      mSpeed = speed;
      mAdsr.Start(time, velocity / 127.0f);
      mAdsr.Stop(time + lengthSeconds*1000);
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
   
   float lengthSeconds = mSample->LengthInSamples() / (gSampleRate * mSample->GetSampleRateRatio());
   for (size_t i=0; i<mSampleCuePoints.size(); ++i)
   {
      mSampleCuePoints[i].mStartSlider->SetExtents(0, lengthSeconds);
      mSampleCuePoints[i].mLengthSlider->SetExtents(0, lengthSeconds);
   }
}

void SamplePlayer::ButtonClicked(ClickButton *button)
{
   if (button == mPlayButton)
   {
      mPlay = true;
      mAdsr.Start(gTime, 1);
   }
   if (button == mPauseButton)
      mPlay = false;
   if (button == mStopButton)
   {
      mPlay = false;
      mSample->SetPlayPosition(0);
   }
   if (button == mDownloadYoutubeButton)
      DownloadYoutube("https://www.youtube.com/watch?v="+mYoutubeId, "");
}

void SamplePlayer::TextEntryComplete(TextEntry* entry)
{
   if (entry == mDownloadYoutubeSearch)
   {
      int index = 0;
      vector<string> tokens = ofSplitString(mYoutubeSearch,",");
      if (tokens.size() > 1)
         index = ofToInt(tokens[1]);
      DownloadYoutube(string("\"ytsearch"+ofToString(index+1)+":")+tokens[0]+"\"", "--match-filter \"duration < 610\" --playlist-items "+ofToString(index+1));
   }
}

void SamplePlayer::DownloadYoutube(string search, string options)
{
   auto file = juce::File(ofToDataPath("youtube.wav"));
   if (file.existsAsFile())
      file.deleteFile();
   
   char command[2048];
   sprintf(command, "export PATH=/opt/local/bin:$PATH; youtube-dl %s -x --audio-format wav -f bestaudio -o %s -w %s", search.c_str(), ofToDataPath("youtube.m4a").c_str(), options.c_str());
   FILE* output = popen(command, "r");
   
   char c;
   do
   {
      c = fgetc(output);
      printf("%c",c);
   } while (c != EOF);
   
   pclose(output);
   
   Sample* sample = new Sample();
   if (juce::File(ofToDataPath("youtube.wav")).existsAsFile())
      sample->Read(ofToDataPath("youtube.wav").c_str());
   UpdateSample(sample, true);
}

void SamplePlayer::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x,y,right);
   
   if (right)
      return;
   
   if (y > 60 && y < mHeight - 20 && mSample != nullptr)
   {
      mSample->SetPlayPosition(int(GetPlayPositionForMouse(x)));
      mAdsr.Start(gTime, 1);
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

void SamplePlayer::GetPlayInfoForPitch(int pitch, float& startSeconds, float& lengthSeconds, float& speed) const
{
   if (pitch < mSampleCuePoints.size() && mSampleCuePoints[pitch].speed > 0)
   {
      startSeconds = mSampleCuePoints[pitch].startSeconds;
      lengthSeconds = mSampleCuePoints[pitch].lengthSeconds;
      speed = mSampleCuePoints[pitch].speed;
   }
   else
   {
      startSeconds = 0;
      lengthSeconds = 1;
      speed = 1;
   }
}

void SamplePlayer::SetCuePoint(int pitch, float startSeconds, float lengthSeconds, float speed)
{
   if (pitch < mSampleCuePoints.size())
   {
      mSampleCuePoints[pitch].startSeconds = startSeconds;
      mSampleCuePoints[pitch].lengthSeconds = lengthSeconds;
      mSampleCuePoints[pitch].speed = speed;
   }
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
   mDownloadYoutubeSearch->Draw();
   for (size_t i=0; i<mSampleCuePoints.size(); ++i)
   {
      mSampleCuePoints[i].mStartSlider->Draw();
      mSampleCuePoints[i].mLengthSlider->Draw();
      mSampleCuePoints[i].mSpeedSlider->Draw();
   }

   ofPushMatrix();
   ofTranslate(5,60);
   if (mSample)
   {
      float sampleWidth = mWidth - 10;
      DrawAudioBuffer(sampleWidth, mHeight - 65, &mDrawBuffer, 0, mDrawBuffer.BufferSize(), mSample->GetPlayPosition());
      
      ofPushStyle();
      ofFill();
      float lengthSeconds = mSample->LengthInSamples() / (gSampleRate * mSample->GetSampleRateRatio());
      for (size_t i=0; i<mSampleCuePoints.size(); ++i)
      {
         if (mSampleCuePoints[i].speed > 0)
         {
            float x = mSampleCuePoints[i].startSeconds / lengthSeconds * sampleWidth;
            float xEnd = (mSampleCuePoints[i].startSeconds + mSampleCuePoints[i].lengthSeconds) / lengthSeconds * sampleWidth;
            ofSetColor(0, 0, 0, 100);
            ofRect(x-5, 0, (xEnd - x) + 10, 10);
            ofRect(x-5, 0, 10, 10);
            ofSetColor(255, 255, 255);
            DrawTextNormal(ofToString((int)i), x-3, 8, 11);
         }
      }
      ofPopStyle();
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
   for (const OSCBundle::Element* element = bundle.begin(); element != bundle.end(); ++element)
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

void SamplePlayer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
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
   const int kSaveStateRev = 1;
}

void SamplePlayer::SaveState(FileStreamOut& out)
{
   IDrawableModule::SaveState(out);
   
   out << kSaveStateRev;
   
   bool hasSample = (mSample != nullptr);
   out << hasSample;
   if (hasSample)
      mSample->SaveState(out);
   
   out << (int)mSampleCuePoints.size();
   for (size_t i=0; i<mSampleCuePoints.size(); ++i)
   {
      out << mSampleCuePoints[i].startSeconds;
      out << mSampleCuePoints[i].lengthSeconds;
      out << mSampleCuePoints[i].speed;
   }
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
   
   if (rev >= 1)
   {
      int size;
      in >> size;
      mSampleCuePoints.resize(size);
      for (size_t i = 0; i < size; ++i)
      {
         in >> mSampleCuePoints[i].startSeconds;
         in >> mSampleCuePoints[i].lengthSeconds;
         in >> mSampleCuePoints[i].speed;
      }
   }
}

vector<IUIControl*> SamplePlayer::ControlsToIgnoreInSaveState() const
{
   vector<IUIControl*> ignore;
   ignore.push_back(mDownloadYoutubeSearch);
   return ignore;
}
