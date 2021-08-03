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
//
//  VSTWindow.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/23/16.
//
//

#include "VSTWindow.h"
#include "VSTPlugin.h"
#include "ModularSynth.h"

VSTWindow::VSTWindow (VSTPlugin* vst,
                      Component* const pluginEditor,
                      WindowFormatType t)
: DocumentWindow (pluginEditor->getName(), juce::Colours::lightblue,
                  DocumentWindow::minimiseButton | DocumentWindow::closeButton)
, mType(t)
, mOwner(vst)
#ifdef JUCE_MAC
, mNSViewComponent(nullptr)
#endif
{
   setSize (400, 300);
   
   setContentOwned (pluginEditor, true);
   
   auto mainMon = Desktop::getInstance().getDisplays().findDisplayForRect(TheSynth->GetMainComponent()->getScreenBounds()).userArea;

   setTopLeftPosition(mainMon.getX() + mainMon.getWidth() / 4,
                      mainMon.getY() + mainMon.getHeight() / 4);
   
   setVisible (true);
   
#ifdef JUCE_MAC
   if (pluginEditor->getNumChildComponents() > 0)
      mNSViewComponent = dynamic_cast<juce::NSViewComponent*>(pluginEditor->getChildComponent(0));
#endif

}

//static
VSTWindow* VSTWindow::CreateWindow(VSTPlugin* vst, WindowFormatType type)
{
   juce::AudioProcessorEditor* ui = nullptr;
   
   if (type == Normal)
   {
      ui = vst->GetAudioProcessor()->createEditorIfNeeded();
      
      if (ui == nullptr)
         type = Generic;
   }
   
   if (ui == nullptr)
   {
      if (type == Generic || type == Parameters)
         ui = new juce::GenericAudioProcessorEditor (vst->GetAudioProcessor());
      else if (type == Programs)
         ui = new ProgramAudioProcessorEditor (vst->GetAudioProcessor());
   }
   
   if (ui != nullptr)
   {
      if (juce::AudioPluginInstance* const plugin = dynamic_cast<juce::AudioPluginInstance*> (vst->GetAudioProcessor()))
         ui->setName (plugin->getName());
      
      return new VSTWindow(vst, ui, type);
   }
   
   return nullptr;
}

VSTWindow::~VSTWindow()
{
   clearContentComponent();
}

void VSTWindow::moved()
{
   //   owner->properties.set (getLastXProp (mType), getX());
   //   owner->properties.set (getLastYProp (mType), getY());
}

void VSTWindow::closeButtonPressed()
{
   mOwner->OnVSTWindowClosed();
   delete this;
}
