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
/*
  ==============================================================================

    MiniBruteControl.cpp
    Created: 27 June 2023
    Author:  Jóhann Berentsson

  ==============================================================================
*/

#include "MiniBruteControl.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"

MiniBruteControl::MiniBruteControl()
{
}

MiniBruteControl::~MiniBruteControl()
{
}

std::string createLabel(int i)
{
   if (i == 0)
   {
      return "";
   }
   else if (i == 17)
   {
      return "All";
   }
   else
   {
        return std::to_string(i);
    }
}

void MiniBruteControl::CreateUIControls()
{
    IDrawableModule::CreateUIControls();

    /* Send All MIDI CC Button */
    sendButton = new ClickButton(this, "send", 160, 55);

    /* Panic Button */
    panicButton = new ClickButton(this, "panic", 200, 55);

    /* Receive Channel */
    /*
    receiveChannel = new DropdownList(this, "receive channel", 115, 55, &mReceiveChannel);

    for(int i = 0; i < 18; i++){
        receiveChannel->AddLabel(createLabel(i), i);
    }
    */

    /* Send Channel */
    /*
    sendChannel = new DropdownList(this, "send channel", 115, 75, &mSendChannel);
    
    for(int i = 0; i < 17; i++){
        sendChannel->AddLabel(createLabel(i), i);
    }
    */

    /* Sync Source */
    syncSource = new RadioButton(this, "sync source", 5, 5, &mSyncSource);
    syncSource->AddLabel("Auto", 0);
    syncSource->AddLabel("Int", 1);
    syncSource->AddLabel("Ext", 2);

    /* Env Legato Mode */
    envLegatoMode = new RadioButton(this, "envelope legato mode", 60, 55, &mEnvLegatoMode);
    envLegatoMode->AddLabel("Off", 0);
    envLegatoMode->AddLabel("On", 1);

    /* LFO Retrig Mode */
    lfoRetrigMode = new RadioButton(this, "lfo retrig mode", 35, 55, &mLFORetrigMode);
    lfoRetrigMode->AddLabel("Off", 0);
    lfoRetrigMode->AddLabel("On", 1);

    /* Note Priority */
    notePriority = new RadioButton(this, "note priority", 95, 5, &mNotePriority);
    notePriority->AddLabel("Last", 0);
    notePriority->AddLabel("Low", 1);
    notePriority->AddLabel("High", 2);

    /* Velocity Curve */
    velocityCurve = new RadioButton(this, "velocity curve", 40, 5, &mVelocityCurve);
    velocityCurve->AddLabel("Lin", 0);
    velocityCurve->AddLabel("Log", 1);
    velocityCurve->AddLabel("Anti Log", 2);

    /* Audio In Threshold */
    audioInThreshold = new RadioButton(this, "audio in threshold", 205, 5, &mAudioInThreshold);
    audioInThreshold->AddLabel("Low", 0);
    audioInThreshold->AddLabel("Mid", 1);
    audioInThreshold->AddLabel("High", 2);

    /* Aftertouch Curve */
    aftertouchCurve = new RadioButton(this, "aftertouch curve", 130, 5, &mAftertouchCurve);
    aftertouchCurve->AddLabel("Exponential", 0);
    aftertouchCurve->AddLabel("Logarithmic", 1);
    aftertouchCurve->AddLabel("Linear", 2);
    
    /* Arpeggiator Hold */
    arpeggiatorHold = new RadioButton(this, "arpeggiator hold", 5, 55, &mArpeggiatorHold);
    arpeggiatorHold->AddLabel("ON", 0);
    arpeggiatorHold->AddLabel("OFF", 1);
    
    /* Local ON/OFF */
    localOnOff = new RadioButton(this, "local on/off", 85, 55, &mLocalOnOff);
    localOnOff->AddLabel("ON", 0);
    localOnOff->AddLabel("OFF", 1);
}

void MiniBruteControl::DrawModule()
{
    if (Minimized() || IsVisible() == false)
        return;

    panicButton->Draw();
    sendButton->Draw();
    //receiveChannel->Draw();
    //sendChannel->Draw();
    syncSource->Draw();
    envLegatoMode->Draw();
    lfoRetrigMode->Draw();
    notePriority->Draw();
    velocityCurve->Draw();
    audioInThreshold->Draw();
    aftertouchCurve->Draw();
    arpeggiatorHold->Draw();
    localOnOff->Draw();
}

void MiniBruteControl::PlayNote(double time, int pitch, int velocity, int voiceIdx, ModulationParameters modulation)
{
    if (pitch != -1)
        PlayNoteOutput(time, pitch, velocity, voiceIdx, modulation);
}

void MiniBruteControl::ButtonClicked(ClickButton* button, double time)
{
    if (button == panicButton){
        SendCC(ccNoteOff, 127);
    }

    if (button == sendButton){
        if (mReceiveChannel > 0){
            SendCC(ccReciveChannel, mReceiveChannel);
        }

        if (mSendChannel > 0){
            SendCC(ccSendChannel, mSendChannel);
        }

        SendCC(ccSyncSource, threeButtons[mSyncSource]);
        SendCC(ccEnvLegatoMode, twoButtons[mEnvLegatoMode]);
        SendCC(ccLFORetrigMode, twoButtons[mLFORetrigMode]);
        SendCC(ccNotePriority, threeButtons[mNotePriority]);
        SendCC(ccVelocityCurve, threeButtons[mVelocityCurve]);
        SendCC(ccAudioInThreshold, threeButtons[mAudioInThreshold]);
        SendCC(ccAftertouchCurve, threeButtons[mAftertouchCurve]);
        SendCC(ccArpeggiatorHold, twoButtons[mArpeggiatorHold]);
        SendCC(ccLocalOnOff, twoButtons[mLocalOnOff]);
    }
}

void MiniBruteControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
    mModuleSaveData.LoadString("target", moduleInfo);

    SetUpFromSaveData();
}

void MiniBruteControl::SetUpFromSaveData()
{
    SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void MiniBruteControl::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
    if (list == receiveChannel)
        SendCC(ccReciveChannel, (int) mReceiveChannel);
    if (list == sendChannel)
        SendCC(ccSendChannel, (int) mSendChannel);
}

void MiniBruteControl::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
    if (radio == syncSource)
        SendCC(ccSyncSource, threeButtons[(int) mSyncSource]);
    if (radio == envLegatoMode)
        SendCC(ccEnvLegatoMode, twoButtons[(int) mEnvLegatoMode]);
    if (radio == lfoRetrigMode)
        SendCC(ccLFORetrigMode, twoButtons[(int) mLFORetrigMode]);
    if (radio == notePriority)
        SendCC(ccNotePriority, threeButtons[(int) mNotePriority]);
    if (radio == velocityCurve)
        SendCC(ccVelocityCurve, threeButtons[(int) mVelocityCurve]);
    if (radio == audioInThreshold)
        SendCC(ccAudioInThreshold, threeButtons[(int) mAudioInThreshold]);
    if (radio == aftertouchCurve)
        SendCC(ccAftertouchCurve, threeButtons[(int) mAftertouchCurve]);
    if (radio == arpeggiatorHold)
        SendCC(ccArpeggiatorHold, twoButtons[(int) mArpeggiatorHold]);
    if (radio == localOnOff)
        SendCC(ccLocalOnOff, twoButtons[(int) mLocalOnOff]);
}
