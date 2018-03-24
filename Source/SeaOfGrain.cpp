//
//  SeaOfGrain.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 11/8/14.
//
//

#include "SeaOfGrain.h"
#include "IAudioReceiver.h"
#include "Sample.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "ModulationChain.h"

const float mBufferX = 5;
const float mBufferY = 100;
const float mBufferW = 1600;
const float mBufferH = 200;

SeaOfGrain::SeaOfGrain()
: mVolume(.6f)
, mVolumeSlider(nullptr)
, mSample(nullptr)
, mLoading(false)
, mEverythingButton(nullptr)
, mFancyButton(nullptr)
, mStarWarsButton(nullptr)
, mGodOnlyKnowsButton(nullptr)
, mKeyOffset(40)
, mKeyOffsetSlider(nullptr)
, mDisplayKeys(61)
, mDisplayKeysSlider(nullptr)
, mKeyboardBaseNote(36)
, mKeyboardBaseNoteSelector(nullptr)
, mSliceMode(1)
, mSliceModeSelector(nullptr)
{
   mWriteBuffer = new float[gBufferSize];
   Clear(mWriteBuffer, gBufferSize);
   mSample = new Sample();
   
   vector<string> fake;
   fake.push_back(ofToDataPath("01 - Reflektor.mp3",true));
   FilesDropped(fake, 0, 0);
}

void SeaOfGrain::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mVolumeSlider = new FloatSlider(this,"volume",5,20,150,15,&mVolume,0,2);
   mEverythingButton = new ClickButton(this,"everything",300,10);
   mFancyButton = new ClickButton(this,"fancy",300,30);
   mStarWarsButton = new ClickButton(this,"star wars",300,50);
   mGodOnlyKnowsButton = new ClickButton(this,"god only knows",300,70);
   mKeyOffsetSlider = new IntSlider(this,"key offset", 5,40,150,15,&mKeyOffset,0,1000);
   mDisplayKeysSlider = new IntSlider(this,"num display keys", 5,60,150,15,&mDisplayKeys,0,200);
   mKeyboardBaseNoteSelector = new DropdownList(this,"keyboard base note", 5, 80, &mKeyboardBaseNote);
   mSliceModeSelector = new DropdownList(this,"slice mode",200,80,&mSliceMode);
   
   mKeyboardBaseNoteSelector->AddLabel("0", 0);
   mKeyboardBaseNoteSelector->AddLabel("12", 12);
   mKeyboardBaseNoteSelector->AddLabel("24", 24);
   mKeyboardBaseNoteSelector->AddLabel("36", 36);
   mKeyboardBaseNoteSelector->AddLabel("48", 48);
   mKeyboardBaseNoteSelector->AddLabel("60", 60);
   
   mSliceModeSelector->AddLabel("beats",0);
   mSliceModeSelector->AddLabel("tatums",1);
}

SeaOfGrain::~SeaOfGrain()
{
   delete[] mWriteBuffer;
   delete mSample;
}

void SeaOfGrain::Poll()
{
}

void SeaOfGrain::Process(double time)
{
   Profiler profiler("SeaOfGrain");
   
   if (!mEnabled || GetTarget() == nullptr || mSample == nullptr || mLoading)
      return;
   
   ComputeSliders(0);
   
   int bufferSize = GetTarget()->GetBuffer()->BufferSize();
   float* out = GetTarget()->GetBuffer()->GetChannel(0);
   assert(bufferSize == gBufferSize);
   
   Clear(mWriteBuffer, bufferSize);
   for (int i=0; i<NUM_SEAOFGRAIN_VOICES; ++i)
      mVoices[i].Process(mWriteBuffer, bufferSize, mSample->Data()->GetChannel(0), mSample->LengthInSamples(), mKeyOffset, GetSlices());
   GetVizBuffer()->WriteChunk(mWriteBuffer, bufferSize, 0);
   
   Add(out, mWriteBuffer, bufferSize);
}

void SeaOfGrain::DrawModule()
{  

   if (Minimized() || IsVisible() == false)
      return;
   
   mVolumeSlider->Draw();
   mEverythingButton->Draw();
   mFancyButton->Draw();
   mStarWarsButton->Draw();
   mGodOnlyKnowsButton->Draw();
   mKeyOffsetSlider->Draw();
   mDisplayKeysSlider->Draw();
   mKeyboardBaseNoteSelector->Draw();
   mSliceModeSelector->Draw();
   
   if (mSample)
   {
      ofPushMatrix();
      ofTranslate(mBufferX,mBufferY);
      ofPushStyle();
      
      int length = mSample->LengthInSamples();
      int sampleStart = GetSlices()[mKeyOffset] * length;
      int sampleLength = (GetSlices()[mKeyOffset + mDisplayKeys] - GetSlices()[mKeyOffset]) * length;
      sampleLength = MAX(1, sampleLength);
      
      mSample->LockDataMutex(true);
      DrawAudioBuffer(mBufferW, mBufferH, mSample->Data(), sampleStart, sampleStart + sampleLength, 0);
      mSample->LockDataMutex(false);
      
      ofPushStyle();
      ofFill();
      for (int i=0; i<mDisplayKeys; ++i)
      {
         ofSetColor(i%2 * 200, 200, 0);
         ofRect(mBufferW * float(i)/mDisplayKeys, mBufferH, mBufferW/mDisplayKeys, 10);
      }
      ofPopStyle();
      
      for (int i=0; i<NUM_SEAOFGRAIN_VOICES; ++i)
         mVoices[i].Draw(mBufferW, mBufferH, sampleStart, sampleLength, mDisplayKeys);
      
      ofPopStyle();
      ofPopMatrix();
   }
}

void SeaOfGrain::FilesDropped(vector<string> files, int x, int y)
{
   mLoading = true;
   
   mSample->Reset();
   
   mSample->Read(files[0].c_str());
   
   ResetRead();
   
   vector<string> tokens = ofSplitString(files[0].c_str(),"/");
   string cachedFilename = tokens[tokens.size()-1].c_str();
   tokens = ofSplitString(cachedFilename, ".");
   cachedFilename = tokens[0]+".cached";
   bool hasCached = File(ofToDataPath(cachedFilename)).existsAsFile();
   
   ofLog() << cachedFilename << " exists: " << (hasCached ? "true" : "false");
   
   FILE* output;
   FILE* cachedFile;
   if (!hasCached)   //have to look it up with echonest
   {
      char command[2048];
      sprintf(command,"export ECHO_NEST_API_KEY=SUZ3W7PAIVQQXZCAW; export PATH=/usr/local/bin:$PATH; python \"%s\" \"%s\"", ofToDataPath("get_echonest_remix_data.py",true).c_str(), files[0].c_str());
      output = popen(command, "r");
      cachedFile = fopen(ofToDataPath(cachedFilename).c_str(), "w");
   }
   else
   {
      output = fopen(ofToDataPath(cachedFilename).c_str(),"r");
   }
   
   char c;
   char line[512];
   bzero(line,512);
   int linepos = 0;
   do
   {
      c = fgetc(output);
      if (!hasCached)
         fputc(c,cachedFile);
      //printf("%c",c);
      if (c == '\n' || c == EOF)
      {
         ReadEchonestLine(line);
         
         linepos = 0;
         bzero(line,512);
      }
      else
      {
         line[linepos++] = c;
      }
   } while (c != EOF);
   
   if (hasCached)
   {
      fclose(output);
   }
   else
   {
      pclose(output);
      fclose(cachedFile);
   }
   
   mLoading = false;
}

void SeaOfGrain::ResetRead()
{
   mReadState = kReadState_Start;
   mBeats.clear();
   mTatums.clear();
}

void SeaOfGrain::ReadEchonestLine(const char* line)
{
   vector<string> tokens = ofSplitString(line," ");
   if (tokens.size() == 1)
   {
      if (tokens[0] == "bars")
         mReadState = kReadState_Bars;
      if (tokens[0] == "beats")
         mReadState = kReadState_Beats;
      if (tokens[0] == "tatums")
         mReadState = kReadState_Tatums;
      if (tokens[0] == "sections")
         mReadState = kReadState_Sections;
      if (tokens[0] == "segments")
         mReadState = kReadState_Segments;
   }
   else
   {
      float lengthInSeconds = mSample->LengthInSamples() / gSampleRate;
      
      if (mReadState == kReadState_Bars || mReadState == kReadState_Beats || mReadState == kReadState_Tatums)
         assert(tokens.size() == 3);
      if (mReadState == kReadState_Sections || mReadState == kReadState_Segments)
         assert(tokens.size() == 2);
      
      float adjustSeconds = -.062f;
      float start = (ofToFloat(tokens[0]) + adjustSeconds) / lengthInSeconds;
      //float duration = ofToFloat(tokens[1]) / lengthInSeconds;
      //float confidence = tokens.size() == 3 ? ofToFloat(tokens[2]) : 1;
      
      if (mReadState == kReadState_Bars)
      {
      }
      if (mReadState == kReadState_Beats)
      {
         ofLog() << "beat " << start;
         mBeats.push_back(start);
      }
      if (mReadState == kReadState_Tatums)
      {
         mTatums.push_back(start);
      }
      if (mReadState == kReadState_Sections)
      {
      }
      if (mReadState == kReadState_Segments)
      {
      }
   }
}

void SeaOfGrain::DropdownClicked(DropdownList* list)
{
}

void SeaOfGrain::DropdownUpdated(DropdownList* list, int oldVal)
{
}

void SeaOfGrain::UpdateSample()
{
}

void SeaOfGrain::ButtonClicked(ClickButton *button)
{
   if (button == mEverythingButton)
   {
      mKeyOffset = 172;
      mSliceMode = 1;
      vector<string> fake;
      fake.push_back(ofToDataPath("1. Everything In Its Right Place.mp3",true));
      FilesDropped(fake, 0, 0);
   }
   if (button == mFancyButton)
   {
      mKeyOffset = 1;
      mSliceMode = 1;
      vector<string> fake;
      fake.push_back(ofToDataPath("Iggy Azalea - Fancy ft. Charli XCX.mp3",true));
      FilesDropped(fake, 0, 0);
   }
   if (button == mStarWarsButton)
   {
      mKeyOffset = 0;
      mSliceMode = 1;
      vector<string> fake;
      fake.push_back(ofToDataPath("John Williams - Star Wars Main Theme (FULL).mp3",true));
      FilesDropped(fake, 0, 0);
   }
   if (button == mGodOnlyKnowsButton)
   {
      mKeyOffset = 41;
      mSliceMode = 1;
      vector<string> fake;
      fake.push_back(ofToDataPath("God Only Knows - The Beach Boys, a cappella.mp3",true));
      FilesDropped(fake, 0, 0);
   }
}

void SeaOfGrain::OnClicked(int x, int y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void SeaOfGrain::MouseReleased()
{
   IDrawableModule::MouseReleased();
}

bool SeaOfGrain::MouseMoved(float x, float y)
{
   return IDrawableModule::MouseMoved(x,y);
}

void SeaOfGrain::CheckboxUpdated(Checkbox *checkbox)
{
}

void SeaOfGrain::GetModuleDimensions(int& x, int& y)
{
   x = mBufferW + 10;
   y = mBufferY + mBufferH + 100;
}

void SeaOfGrain::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
}

void SeaOfGrain::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void SeaOfGrain::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (voiceIdx == -1 || voiceIdx >= NUM_SEAOFGRAIN_VOICES)
      return;
   
   pitch -= mKeyboardBaseNote;
   if (velocity > 0)
      mVoices[voiceIdx].mADSR.Start(time, 1);
   else
      mVoices[voiceIdx].mADSR.Stop(time);
   mVoices[voiceIdx].mPitch = pitch;
   mVoices[voiceIdx].mPlay = 0;
   mVoices[voiceIdx].mPitchBend = modulation.pitchBend;
   mVoices[voiceIdx].mPressure = modulation.pressure;
}

void SeaOfGrain::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void SeaOfGrain::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

SeaOfGrain::GrainVoice::GrainVoice()
: mADSR(100,0,1,100)
, mPitch(0)
, mPitchBend(0)
, mPressure(0)
, mGain(0)
{
   mGranulator.mGrainLengthMs = 150;
}

void SeaOfGrain::GrainVoice::Process(float* out, float outLength, float* sample, int sampleLength, int keyOffset, const vector<float>& beats)
{
   if (!mADSR.IsDone(gTime))
   {
      double time = gTime;
      for (int i=0; i<outLength; ++i)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(i) : 0;
         float pressure = mPressure ? mPressure->GetValue(i) : 0;
         if (pressure > 0)
         {
            mGranulator.mGrainSpacing = ofMap(pressure * pressure, 0, 1, .3f, 1.0f / MAX_GRAINS);
            mGranulator.mPosRandomizeMs = ofMap(pressure * pressure, 0, 1, 100, .03f);
         }
         
         float position = keyOffset + mPitch + pitchBend + MIN(.125f, mPlay);
         int intPosition = position;
         float positionPart = position - intPosition;
         
         float beat = ofMap(positionPart, 0, 1, beats[intPosition] * sampleLength, beats[intPosition+1] * sampleLength);
         
         float blend = .0005f;
         mGain = mGain * (1-blend) + pressure * blend;
         
         ChannelBuffer temp(sample, sampleLength);
         float outSample[1];
         mGranulator.Process(time, &temp, sampleLength, beat, outSample);
         outSample[0] *= sqrtf(mGain);
         outSample[0] *= mADSR.Value(gTime);
         out[i] += outSample[0];
         time += gInvSampleRateMs;
         mPlay += .001f;
      }
   }
   else
   {
      mGranulator.ClearGrains();
   }
}

void SeaOfGrain::GrainVoice::Draw(float w, float h, float offset, float length, int numPitches)
{
   if (!mADSR.IsDone(gTime))
   {
      if (mPitch < numPitches)
      {
         float pitchBend = mPitchBend ? mPitchBend->GetValue(0) : 0;
         float pressure = mPressure ? mPressure->GetValue(0) : 0;
         
         ofPushStyle();
         ofFill();
         float keyX = mPitch / numPitches * w;
         float keyXTop = keyX + pitchBend * w / numPitches;
         ofBeginShape();
         ofVertex(keyX, h);
         ofVertex(keyXTop, h - pressure * h);
         ofVertex(keyXTop +10, h - pressure * h);
         ofVertex(keyX+10, h);
         ofEndShape();
         ofPopStyle();
      }
      
      mGranulator.Draw(0, 0, w, h, offset, length, false);
      DrawText(ofToString(mGranulator.mPosRandomizeMs), 0, h+22);
      DrawText(ofToString(mGranulator.mGrainLengthMs), 0, h+42);
      DrawText(ofToString(mPitch), 0, h+62);
   }
}


