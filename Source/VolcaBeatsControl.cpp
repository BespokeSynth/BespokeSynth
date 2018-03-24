/*
  ==============================================================================

    VolcaBeatsControl.cpp
    Created: 28 Jan 2017 10:48:14pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "VolcaBeatsControl.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"

VolcaBeatsControl::VolcaBeatsControl()
: mClapSpeed(.5f)
, mClaveSpeed(.5f)
, mAgogoSpeed(.5f)
, mCrashSpeed(.5f)
, mStutterTime(.5f)
, mStutterDepth(0)
, mTomDecay(.5f)
, mClosedHatDecay(.5f)
, mOpenHatDecay(.5f)
, mHatGrain(.5f)
, mClapSpeedSlider(nullptr)
, mClaveSpeedSlider(nullptr)
, mAgogoSpeedSlider(nullptr)
, mCrashSpeedSlider(nullptr)
, mStutterTimeSlider(nullptr)
, mStutterDepthSlider(nullptr)
, mTomDecaySlider(nullptr)
, mClosedHatDecaySlider(nullptr)
, mOpenHatDecaySlider(nullptr)
, mHatGrainSlider(nullptr)
{
   for (int i=0; i<10; ++i)
   {
      mLevels[i] = 1;
      mLevelSliders[i] = nullptr;
   }
}

VolcaBeatsControl::~VolcaBeatsControl()
{
}

void VolcaBeatsControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   
   mClapSpeedSlider = new FloatSlider(this,"clap speed",5,5,140,15,&mClapSpeed,0,1);
   mClaveSpeedSlider = new FloatSlider(this,"clave speed",mClapSpeedSlider, kAnchor_Below,140,15,&mClaveSpeed,0,1);
   mAgogoSpeedSlider = new FloatSlider(this,"agogo speed",mClaveSpeedSlider, kAnchor_Below,140,15,&mAgogoSpeed,0,1);
   mCrashSpeedSlider = new FloatSlider(this,"crash speed",mAgogoSpeedSlider, kAnchor_Below,140,15,&mCrashSpeed,0,1);
   mStutterTimeSlider = new FloatSlider(this,"stutter time",mCrashSpeedSlider, kAnchor_Below,140,15,&mStutterTime,0,1);
   mStutterDepthSlider = new FloatSlider(this,"stutter depth",mStutterTimeSlider, kAnchor_Below,140,15,&mStutterDepth,0,1);
   mTomDecaySlider = new FloatSlider(this,"tom decay",mStutterDepthSlider, kAnchor_Below,140,15,&mTomDecay,0,1);
   mClosedHatDecaySlider = new FloatSlider(this,"closed hat decay",mTomDecaySlider, kAnchor_Below,140,15,&mClosedHatDecay,0,1);
   mOpenHatDecaySlider = new FloatSlider(this,"open hat decay",mClosedHatDecaySlider, kAnchor_Below,140,15,&mOpenHatDecay,0,1);
   mHatGrainSlider = new FloatSlider(this,"hat grain",mOpenHatDecaySlider, kAnchor_Below,140,15,&mHatGrain,0,1);
   
   for (int i=0; i<10; ++i)
   {
      mLevelSliders[i] = new FloatSlider(this,("level "+ofToString(i)).c_str(),155,5,100,15,&mLevels[i],0,1);
      if (i > 0)
         mLevelSliders[i]->PositionTo(mLevelSliders[i-1], kAnchor_Below);
   }
}

void VolcaBeatsControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;
   
   mClapSpeedSlider->Draw();
   mClaveSpeedSlider->Draw();
   mAgogoSpeedSlider->Draw();
   mCrashSpeedSlider->Draw();
   mStutterTimeSlider->Draw();
   mStutterDepthSlider->Draw();
   mTomDecaySlider->Draw();
   mClosedHatDecaySlider->Draw();
   mOpenHatDecaySlider->Draw();
   mHatGrainSlider->Draw();
   for (int i=0; i<10; ++i)
      mLevelSliders[i]->Draw();
}

void VolcaBeatsControl::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   if (pitch < 10)
   {
      mLevelSliders[pitch]->Compute();
      velocity *= mLevels[pitch];
   }
   
   switch (pitch)
   {
      case 0:  //kick
         pitch = 36;
         if (velocity > 0)
            SendCC(40, velocity);
         break;
      case 1:  //snare
         pitch = 38;
         if (velocity > 0)
            SendCC(41, velocity);
         break;
      case 2:  //closed
         pitch = 42;
         if (velocity > 0)
         {
            SendCC(44, velocity);
            mClosedHatDecaySlider->Compute();
            mHatGrainSlider->Compute();
         }
         break;
      case 3:  //ride
         pitch = 67;
         if (velocity > 0)
         {
            SendCC(48, velocity);
            mAgogoSpeedSlider->Compute();
         }
         break;
      case 4:  //clap
         pitch = 39;
         if (velocity > 0)
         {
            SendCC(46, velocity);
            mClapSpeedSlider->Compute();
         }
         break;
      case 5:  //crash
         pitch = 49;
         if (velocity > 0)
         {
            SendCC(49, velocity);
            mCrashSpeedSlider->Compute();
         }
         break;
      case 6:  //open
         pitch = 46;
         if (velocity > 0)
         {
            SendCC(45, velocity);
            mOpenHatDecaySlider->Compute();
            mHatGrainSlider->Compute();
         }
         break;
      case 7:  //stick
         pitch = 75;
         if (velocity > 0)
         {
            SendCC(47, velocity);
            mClaveSpeedSlider->Compute();
         }
         break;
      case 8:  //floor
         pitch = 43;
         if (velocity > 0)
         {
            SendCC(42, velocity);
            mTomDecaySlider->Compute();
         }
         break;
      case 9:  //low
         pitch = 50;
         if (velocity > 0)
         {
            SendCC(43, velocity);
            mTomDecaySlider->Compute();
         }
         break;
      default:
         pitch = -1;
   }
   
   mStutterTimeSlider->Compute();
   mStutterDepthSlider->Compute();
   
   if (pitch != -1)
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void VolcaBeatsControl::FloatSliderUpdated(FloatSlider* slider, float oldVal)
{
   if (slider == mClapSpeedSlider)
      SendCC(50, (int)(mClapSpeed * 127));
   if (slider == mClaveSpeedSlider)
      SendCC(51, (int)(mClaveSpeed * 127));
   if (slider == mAgogoSpeedSlider)
      SendCC(52, (int)(mAgogoSpeed * 127));
   if (slider == mCrashSpeedSlider)
      SendCC(53, (int)(mCrashSpeed * 127));
   if (slider == mStutterTimeSlider)
      SendCC(54, (int)(mStutterTime * 127));
   if (slider == mStutterDepthSlider)
      SendCC(55, (int)(mStutterDepth * 127));
   if (slider == mTomDecaySlider)
      SendCC(56, (int)(mTomDecay * 127));
   if (slider == mClosedHatDecaySlider)
      SendCC(57, (int)(mClosedHatDecay * 127));
   if (slider == mOpenHatDecaySlider)
      SendCC(58, (int)(mOpenHatDecay * 127));
   if (slider == mHatGrainSlider)
      SendCC(59, (int)(mHatGrain * 127));
}

void VolcaBeatsControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   
   SetUpFromSaveData();
}

void VolcaBeatsControl::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}
