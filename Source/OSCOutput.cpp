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

    OSCOutput.cpp
    Created: 27 Sep 2018 9:36:01pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "OSCOutput.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"

OSCOutput::OSCOutput()
{
   for (int i = 0; i < OSC_OUTPUT_MAX_PARAMS; ++i)
   {
      mParams[i] = 0;
      mLabels[i] = "slider" + ofToString(i);
   }
}

OSCOutput::~OSCOutput()
{
}

void OSCOutput::Init()
{
   IDrawableModule::Init();

   mOscOut.connect(mOscOutAddress, mOscOutPort);
}

void OSCOutput::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   TEXTENTRY(mOscOutAddressEntry, "osc out address", 16, &mOscOutAddress);
   UIBLOCK_SHIFTRIGHT();
   TEXTENTRY_NUM(mOscOutPortEntry, "osc out port", 6, &mOscOutPort, 0, 99999);
   UIBLOCK_NEWLINE();
   UIBLOCK_SHIFTY(5);
   for (int i = 0; i < 8; ++i)
   {
      TextEntry* labelEntry;
      TEXTENTRY(labelEntry, ("label" + ofToString(i)).c_str(), 10, &mLabels[i]);
      mLabelEntry.push_back(labelEntry);
      UIBLOCK_SHIFTRIGHT();

      FloatSlider* oscSlider;
      FLOATSLIDER(oscSlider, mLabels[i].c_str(), &mParams[i], 0, 1);
      mSliders.push_back(oscSlider);
      UIBLOCK_NEWLINE();
   }
   UIBLOCK_SHIFTY(5);
   TEXTENTRY(mNoteOutLabelEntry, "note out address", 10, &mNoteOutLabel);
   ENDUIBLOCK(mWidth, mHeight);

   mNoteOutLabelEntry->DrawLabel(true);
}

void OSCOutput::Poll()
{
   ComputeSliders(0);
}

void OSCOutput::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mOscOutAddressEntry->Draw();
   mOscOutPortEntry->Draw();

   for (auto* entry : mLabelEntry)
      entry->Draw();
   for (auto* slider : mSliders)
      slider->Draw();

   mNoteOutLabelEntry->Draw();
}

void OSCOutput::PlayNote(NoteMessage note)
{
   if (mNoteOutLabel.size() > 0)
   {
      juce::OSCMessage msg(("/bespoke/" + mNoteOutLabel).c_str());
      float pitchOut = note.pitch;
      if (note.modulation.pitchBend != nullptr)
         pitchOut += note.modulation.pitchBend->GetValue(0);
      msg.addFloat32(pitchOut);
      msg.addFloat32(note.velocity);
      mOscOut.send(msg);
   }
}

void OSCOutput::SendFloat(std::string address, float val)
{
   juce::OSCMessage msg(address.c_str());
   msg.addFloat32(val);
   mOscOut.send(msg);
}

void OSCOutput::SendInt(std::string address, int val)
{
   juce::OSCMessage msg(address.c_str());
   msg.addInt32(val);
   mOscOut.send(msg);
}

void OSCOutput::SendString(std::string address, std::string val)
{
   juce::OSCMessage msg(address.c_str());
   msg.addString(val);
   mOscOut.send(msg);
}

void OSCOutput::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void OSCOutput::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   juce::String address = "/bespoke/";
   address += slider->Name();
   juce::OSCMessage msg(address);
   msg.addFloat32(slider->GetValue());
   mOscOut.send(msg);
}

void OSCOutput::TextEntryComplete(TextEntry* entry)
{
   int i = 0;
   for (auto* iter : mLabelEntry)
   {
      if (iter == entry)
      {
         auto sliderIter = mSliders.begin();
         advance(sliderIter, i);
         (*sliderIter)->SetName(mLabels[i].c_str());
      }
      ++i;
   }

   if (entry == mOscOutAddressEntry || entry == mOscOutPortEntry)
   {
      mOscOut.disconnect();
      mOscOut.connect(mOscOutAddress, mOscOutPort);
   }
}

void OSCOutput::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void OSCOutput::SetUpFromSaveData()
{
}

void OSCOutput::SaveLayout(ofxJSONElement& moduleInfo)
{
}
