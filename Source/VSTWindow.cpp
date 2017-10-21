//
//  VSTWindow.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 1/23/16.
//
//

#include "VSTWindow.h"
#include "VSTPlugin.h"

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
   
   setTopLeftPosition (juce::Random::getSystemRandom().nextInt (500),
                       juce::Random::getSystemRandom().nextInt (500));
   
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
