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
//  VSTWindow.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/23/16.
//
//

#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

class VSTPlugin;

namespace juce
{
   class NSViewComponent;
}

//==============================================================================
class VSTWindow : public juce::DocumentWindow
{
public:
   enum WindowFormatType
   {
      Normal = 0,
      Generic,
      Programs,
      Parameters,
      NumTypes
   };

   VSTWindow(VSTPlugin* vst, Component* pluginEditor, WindowFormatType);
   ~VSTWindow();

   void ShowWindow();

   static VSTWindow* CreateVSTWindow(VSTPlugin* vst, WindowFormatType);

   void moved() override;
   void closeButtonPressed() override;

#ifdef JUCE_MAC
   juce::NSViewComponent* GetNSViewComponent()
   {
      return mNSViewComponent;
   }
#endif

private:
   WindowFormatType mType{ WindowFormatType::Normal };
   VSTPlugin* mOwner{ nullptr };
#ifdef JUCE_MAC
   juce::NSViewComponent* mNSViewComponent{ nullptr };
#endif

   float getDesktopScaleFactor() const override
   {
      return 1.0f;
   }

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VSTWindow)
};

inline juce::String toString(VSTWindow::WindowFormatType type)
{
   switch (type)
   {
      case VSTWindow::Normal: return "Normal";
      case VSTWindow::Generic: return "Generic";
      case VSTWindow::Programs: return "Programs";
      case VSTWindow::Parameters: return "Parameters";
      default: return juce::String();
   }
}

inline juce::String getLastXProp(VSTWindow::WindowFormatType type) { return "uiLastX_" + toString(type); }
inline juce::String getLastYProp(VSTWindow::WindowFormatType type) { return "uiLastY_" + toString(type); }
inline juce::String getOpenProp(VSTWindow::WindowFormatType type) { return "uiopen_" + toString(type); }

class ProcessorProgramPropertyComp : public juce::PropertyComponent,
                                     private juce::AudioProcessorListener
{
public:
   ProcessorProgramPropertyComp(const juce::String& name, juce::AudioProcessor& p, int index_)
   : PropertyComponent(name)
   , owner(p)
   , index(index_)
   {
      owner.addListener(this);
   }

   ~ProcessorProgramPropertyComp()
   {
      owner.removeListener(this);
   }

   void refresh() {}
   void audioProcessorChanged(juce::AudioProcessor*, const juce::AudioProcessorListener::ChangeDetails&) {}
   void audioProcessorParameterChanged(juce::AudioProcessor*, int, float) {}

private:
   juce::AudioProcessor& owner;
   const int index;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorProgramPropertyComp)
};

class ProgramAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
   ProgramAudioProcessorEditor(juce::AudioProcessor* const p)
   : juce::AudioProcessorEditor(p)
   {
      jassert(p != nullptr);
      setOpaque(true);

      addAndMakeVisible(panel);

      juce::Array<juce::PropertyComponent*> programs;

      const int numPrograms = p->getNumPrograms();
      int totalHeight = 0;

      for (int i = 0; i < numPrograms; ++i)
      {
         juce::String name(p->getProgramName(i).trim());

         if (name.isEmpty())
            name = "Unnamed";

         ProcessorProgramPropertyComp* const pc = new ProcessorProgramPropertyComp(name, *p, i);
         programs.add(pc);
         totalHeight += pc->getPreferredHeight();
      }

      panel.addProperties(programs);

      setSize(400, juce::jlimit(25, 400, totalHeight));
   }

   void paint(juce::Graphics& g)
   {
      g.fillAll(juce::Colours::grey);
   }

   void resized()
   {
      panel.setBounds(getLocalBounds());
   }

private:
   juce::PropertyPanel panel;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgramAudioProcessorEditor)
};
