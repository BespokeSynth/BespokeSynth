//
//  VelocityStepSequencer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/14/14.
//
//

#include "VelocityStepSequencer.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "VelocityStepSequencer.h"
#include "LaunchpadInterpreter.h"
#include "FillSaveDropdown.h"
#include "MidiController.h"

#define ARP_REST -100
#define ARP_HOLD -101

VelocityStepSequencer::VelocityStepSequencer()
: mInterval(kInterval_16n)
, mArpIndex(-1)
, mIntervalSelector(nullptr)
, mLength(VSS_MAX_STEPS)
, mLengthSlider(nullptr)
, mResetOnDownbeat(true)
, mResetOnDownbeatCheckbox(nullptr)
, mCurrentVelocity(80)
, mController(nullptr)
{
   TheTransport->AddListener(this, mInterval, OffsetInfo(-.1f, true), false);
}

void VelocityStepSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mIntervalSelector = new DropdownList(this,"interval",115,2,(int*)(&mInterval));
   mLengthSlider = new IntSlider(this,"len",115,20,40,15,&mLength,1,VSS_MAX_STEPS);
   mResetOnDownbeatCheckbox = new Checkbox(this,"downbeat",5,20,&mResetOnDownbeat);
   
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   
   for (int i=0;i<VSS_MAX_STEPS;++i)
   {
      mVels[i] = 80;
      mVelSliders[i] = new IntSlider(this,("vel"+ofToString(i+1)).c_str(),10,35+i*15,80,15,&(mVels[i]),1,127);
   }
}

VelocityStepSequencer::~VelocityStepSequencer()
{
   TheTransport->RemoveListener(this);
}

void VelocityStepSequencer::SetMidiController(string name)
{
   if (mController)
      mController->RemoveListener(this);
   mController = TheSynth->FindMidiController(name);
   if (mController)
      mController->AddListener(this, 0);
}

void VelocityStepSequencer::DrawModule()
{

   if (Minimized() || IsVisible() == false)
      return;
   
   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mResetOnDownbeatCheckbox->Draw();
   
   ofPushStyle();
   ofSetColor(0,255,0,gModuleDrawAlpha);
   ofFill();
   ofRect(10,35+mArpIndex*15,80,15);
   ofPopStyle();
   
   for (int i=0;i<VSS_MAX_STEPS;++i)
      mVelSliders[i]->Draw();
}

void VelocityStepSequencer::CheckboxUpdated(Checkbox* checkbox)
{
   if (checkbox == mEnabledCheckbox)
      mNoteOutput.Flush(gTime);
}

void VelocityStepSequencer::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
   if (!mEnabled)
   {
      PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
      return;
   }
   
   PlayNoteOutput(time, pitch, velocity > 0 ? mCurrentVelocity : 0, voiceIdx, modulation);
}

void VelocityStepSequencer::OnTimeEvent(double time)
{
   ++mArpIndex;
   
   if (mArpIndex >= mLength)
      mArpIndex = 0;
   
   if (mResetOnDownbeat && TheTransport->GetQuantized(time, mInterval) == 0)
      mArpIndex = 0;
   
   mCurrentVelocity = mVels[mArpIndex];
}

void VelocityStepSequencer::OnMidiNote(MidiNote& note)
{
}

void VelocityStepSequencer::OnMidiControl(MidiControl& control)
{
   if (!mEnabled)
      return;
   
   if (control.mControl >= 41 && control.mControl <= 48)
   {
      int step = control.mControl - 41;
      if (control.mValue >= 1)
         mVels[step] = control.mValue;
   }
}

void VelocityStepSequencer::ButtonClicked(ClickButton* button)
{
}

void VelocityStepSequencer::DropdownUpdated(DropdownList* list, int oldVal)
{
   if (list == mIntervalSelector)
      TheTransport->UpdateListener(this, mInterval, OffsetInfo(-.1f, true));
}

void VelocityStepSequencer::IntSliderUpdated(IntSlider* slider, int oldVal)
{
}

void VelocityStepSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadBool("chord_progression_mode", moduleInfo);
   mModuleSaveData.LoadString("controller", moduleInfo, "", FillDropdown<MidiController*>);
   
   SetUpFromSaveData();
}

void VelocityStepSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   SetMidiController(mModuleSaveData.GetString("controller"));
}




