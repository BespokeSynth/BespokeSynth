//
//  VSTWindow.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/23/16.
//
//

#ifndef __Bespoke__VSTWindow__
#define __Bespoke__VSTWindow__

#include "../JuceLibraryCode/JuceHeader.h"

class VSTPlugin;

//==============================================================================
class VSTWindow  : public juce::DocumentWindow
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
   
   static VSTWindow* CreateWindow(VSTPlugin* vst, WindowFormatType);
   
   static void closeCurrentlyOpenWindowsFor (const juce::uint32 nodeId);
   static void closeAllCurrentlyOpenWindows();
   
   void moved() override;
   void closeButtonPressed() override;

#ifdef JUCE_MAC
   juce::NSViewComponent* GetNSViewComponent() { return mNSViewComponent; }
#endif
   
private:
   WindowFormatType mType;
   VSTPlugin* mOwner;
#ifdef JUCE_MAC
   juce::NSViewComponent* mNSViewComponent;
#endif
   
   float getDesktopScaleFactor() const override     { return 1.0f; }
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTWindow)
};

inline juce::String toString (VSTWindow::WindowFormatType type)
{
   switch (type)
   {
      case VSTWindow::Normal:     return "Normal";
      case VSTWindow::Generic:    return "Generic";
      case VSTWindow::Programs:   return "Programs";
      case VSTWindow::Parameters: return "Parameters";
      default:                    return juce::String();
   }
}

inline juce::String getLastXProp (VSTWindow::WindowFormatType type)    { return "uiLastX_" + toString (type); }
inline juce::String getLastYProp (VSTWindow::WindowFormatType type)    { return "uiLastY_" + toString (type); }
inline juce::String getOpenProp  (VSTWindow::WindowFormatType type)    { return "uiopen_"  + toString (type); }

class ProcessorProgramPropertyComp : public juce::PropertyComponent,
private juce::AudioProcessorListener
{
public:
   ProcessorProgramPropertyComp (const juce::String& name, juce::AudioProcessor& p, int index_)
   : PropertyComponent (name),
   owner (p),
   index (index_)
   {
      owner.addListener (this);
   }
   
   ~ProcessorProgramPropertyComp()
   {
      owner.removeListener (this);
   }
   
   void refresh() { }
   void audioProcessorChanged (juce::AudioProcessor*) { }
   void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) { }
   
private:
   juce::AudioProcessor& owner;
   const int index;
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorProgramPropertyComp)
};

class ProgramAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
   ProgramAudioProcessorEditor (juce::AudioProcessor* const p)
   : juce::AudioProcessorEditor (p)
   {
      jassert (p != nullptr);
      setOpaque (true);
      
      addAndMakeVisible (panel);
      
      juce::Array<juce::PropertyComponent*> programs;
      
      const int numPrograms = p->getNumPrograms();
      int totalHeight = 0;
      
      for (int i = 0; i < numPrograms; ++i)
      {
         juce::String name (p->getProgramName (i).trim());
         
         if (name.isEmpty())
            name = "Unnamed";
         
         ProcessorProgramPropertyComp* const pc = new ProcessorProgramPropertyComp (name, *p, i);
         programs.add (pc);
         totalHeight += pc->getPreferredHeight();
      }
      
      panel.addProperties (programs);
      
      setSize (400, juce::jlimit (25, 400, totalHeight));
   }
   
   void paint (juce::Graphics& g)
   {
      g.fillAll (juce::Colours::grey);
   }
   
   void resized()
   {
      panel.setBounds (getLocalBounds());
   }
   
private:
   juce::PropertyPanel panel;
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramAudioProcessorEditor)
};

#endif /* defined(__Bespoke__VSTWindow__) */
