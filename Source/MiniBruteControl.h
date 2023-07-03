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

    MiniBruteControl.h
    Created: 27 June 2023
    Author:  Jóhann Berentsson

  ==============================================================================

    Parameter               MIDI CC             Value
    Receive Channel         102                 1 to 16, 17=All
    Send Channel            103                 1 to 16
    Sync Source             108                 0 to 41 = Auto
                                                42 to 83 = Int
                                                84 to 127 = Ext
    Env Legato Mode         109                 0 to 63 = Off
                                               64 to 127 = On
    LFO Retrig Mode         110                 0 to 63 = Off
                                               64 to 127 = On
    Note Priority           111                 0 to 41 = Last
                                               42 to 83 = Low
                                               84 to 127 = High
    Velocity Curve          112                 0 to 41 = Lin
                                               42 to 83 = Log
                                               84 to 127 = Anti Log
    Audio In Threshold      115                 0 to 41 = Low
                                               42 to 83 = Mid
                                               84 to 127 = High
    Aftertouch Curve        116                 0 to 41 = Exponential
                                               42 to 82 = Logarithmic
                                               83 to 127 = Linear
    Arpeggiator Hold        117                 0 to 63 = Off
                                               64 to 127 = On
    Local ON/OFF            122                 0 = turn off
                                              127 = turn on

  ==============================================================================
*/

#ifndef MINIBRUTECONTROL_H_INCLUDED
#define MINIBRUTECONTROL_H_INCLUDED

#include <iostream>
#include "IDrawableModule.h"
#include "DropdownList.h"
#include "NoteEffectBase.h"
#include "Checkbox.h"
#include "RadioButton.h"
#include "ClickButton.h"

class MiniBruteControl : public NoteEffectBase, public IDrawableModule, public IDropdownListener, public IButtonListener, public IRadioButtonListener
{
public:
   MiniBruteControl();
   ~MiniBruteControl();

   static IDrawableModule* Create() { return new MiniBruteControl(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }

   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal, double time) override;

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 240;
      height = 90;
   }

   int mReceiveChannel{ 0 };
   int mSendChannel{ 0 };
   int mSyncSource{ 0 };
   int mEnvLegatoMode{ 0 };
   int mLFORetrigMode{ 0 };
   int mNotePriority{ 0 };
   int mVelocityCurve{ 0 };
   int mAudioInThreshold{ 0 };
   int mAftertouchCurve{ 0 };
   int mArpeggiatorHold{ 0 };
   int mLocalOnOff{ 0 };

   const int ccReciveChannel= 102;
   const int ccSendChannel= 103;
   const int ccSyncSource = 108;
   const int ccEnvLegatoMode = 109;
   const int ccLFORetrigMode = 110;
   const int ccNotePriority = 111;
   const int ccVelocityCurve = 112;
   const int ccAudioInThreshold = 115;
   const int ccAftertouchCurve = 116;
   const int ccArpeggiatorHold = 117;
   const int ccLocalOnOff = 122;
   const int ccNoteOff = 123;

   int twoButtons[2] = { 0, 127 };
   int threeButtons[3] = { 0, 63, 127 };

   ClickButton* panicButton{ nullptr };
   ClickButton* sendButton{ nullptr };

   DropdownList* receiveChannel{ nullptr };
   DropdownList* sendChannel{ nullptr };
   
   RadioButton* syncSource{ nullptr };
   RadioButton* envLegatoMode{ nullptr };
   RadioButton* lfoRetrigMode{ nullptr };
   RadioButton* notePriority{ nullptr };
   RadioButton* velocityCurve{ nullptr };
   RadioButton* audioInThreshold{ nullptr };
   RadioButton* aftertouchCurve{ nullptr };
   RadioButton* arpeggiatorHold{ nullptr };
   RadioButton* localOnOff{ nullptr };
};

#endif // MINIBRUTECONTROL_H_INCLUDED
