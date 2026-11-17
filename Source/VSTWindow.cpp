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
#include "UserPrefs.h"

#include "juce_gui_extra/juce_gui_extra.h"

VSTWindow::VSTWindow(VSTPlugin* vst,
                     Component* const pluginEditor,
                     WindowFormatType t)
: DocumentWindow(pluginEditor->getName(), juce::Colours::lightblue,
                 DocumentWindow::minimiseButton | DocumentWindow::closeButton)
, mType(t)
, mOwner(vst)
{
#if BESPOKE_IOS
   setContentOwned(pluginEditor, true);

   //no free-floating windows on iOS: cover the safe part of the screen, and the title bar's close button brings the patch back
   setResizable(false, false);
   setBounds(GetIOSSafeWindowBounds());
#else
   setSize(400, 300);
   setResizable(true, true);

   setContentOwned(pluginEditor, true);

   if (const auto* dpy = juce::Desktop::getInstance().getDisplays().getDisplayForRect(TheSynth->GetMainComponent()->getScreenBounds()))
   {
      const auto& mainMon = dpy->userArea;
      setTopLeftPosition(mainMon.getX() + mainMon.getWidth() / 4,
                         mainMon.getY() + mainMon.getHeight() / 4);
   }
#endif

   setVisible(true);
#if BESPOKE_LINUX
   setUsingNativeTitleBar(true);
#endif

#ifdef JUCE_MAC
   if (pluginEditor->getNumChildComponents() > 0)
      mNSViewComponent = dynamic_cast<juce::NSViewComponent*>(pluginEditor->getChildComponent(0));
#endif
}

//static
bool VSTWindow::IsAUv3(juce::AudioProcessor* processor)
{
   auto* instance = dynamic_cast<juce::AudioPluginInstance*>(processor);
   if (instance == nullptr)
      return false;
   auto desc = instance->getPluginDescription();
   auto& formatManager = TheSynth->GetAudioPluginFormatManager();
   for (int i = 0; i < formatManager.getNumFormats(); ++i)
   {
      auto* format = formatManager.getFormat(i);
      if (format->getName() == desc.pluginFormatName)
         return format->requiresUnblockedMessageThreadDuringCreation(desc);
   }
   return false;
}

//static
VSTWindow* VSTWindow::CreateVSTWindow(VSTPlugin* vst, WindowFormatType type)
{
   if (juce::AudioProcessorEditor* ui = CreateEditor(vst->GetAudioProcessor(), type))
      return new VSTWindow(vst, ui, type);
   return nullptr;
}

//static
juce::AudioProcessorEditor* VSTWindow::CreateEditor(juce::AudioProcessor* processor, WindowFormatType& type)
{
   juce::AudioProcessorEditor* ui = nullptr;

#if BESPOKE_IOS
   //on iOS only AUv3s bring their own UI. apple's built-in (v2) units claim a view controller but never deliver one, which
   //leaves a blank window, so show those with the generic editor
   if (type == Normal && !IsAUv3(processor))
      type = Generic;
#endif

   if (type == Normal)
   {
      ui = processor->createEditorIfNeeded();

      if (ui == nullptr)
         type = Generic;
   }

   if (ui == nullptr)
   {
      if (type == Generic || type == Parameters)
         ui = new juce::GenericAudioProcessorEditor(*processor);
      else if (type == Programs)
         ui = new ProgramAudioProcessorEditor(processor);
   }

   if (ui != nullptr)
   {
      if (juce::AudioPluginInstance* const plugin = dynamic_cast<juce::AudioPluginInstance*>(processor))
         ui->setName(plugin->getName());
   }

   return ui;
}

VSTWindow::~VSTWindow()
{
   clearContentComponent();
}

void VSTWindow::ShowWindow()
{
#if !BESPOKE_LINUX && !BESPOKE_IOS
   setAlwaysOnTop(UserPrefs.vst_always_on_top.Get());
#endif
   toFront(true);
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
