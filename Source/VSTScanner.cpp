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
//  VSTScanner.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 10/16/21.
//
//

#include "VSTScanner.h"
#include "VSTPlugin.h"
#include "ModularSynth.h"

#include "juce_gui_basics/juce_gui_basics.h"

extern juce::ApplicationProperties& getAppProperties();

CustomPluginScanner::CustomPluginScanner()
{
   if (auto* file = getAppProperties().getUserSettings())
      file->addChangeListener(this);

   changeListenerCallback(nullptr);
}

CustomPluginScanner::~CustomPluginScanner()
{
   if (auto* file = getAppProperties().getUserSettings())
      file->removeChangeListener(this);
}

bool CustomPluginScanner::findPluginTypesFor(juce::AudioPluginFormat& format,
                                             juce::OwnedArray<juce::PluginDescription>& result,
                                             const juce::String& fileOrIdentifier)
{
   if (!scanWithExternalProcess)
   {
      superprocess = nullptr;
      format.findAllTypesForFile(result, fileOrIdentifier);
      return true;
   }

   if (superprocess == nullptr)
   {
      superprocess = std::make_unique<Superprocess>(*this);

      std::unique_lock<std::mutex> lock(mutex);
      connectionLost = false;
   }

   juce::MemoryBlock block;
   juce::MemoryOutputStream stream{ block, true };
   stream.writeString(format.getName());
   stream.writeString(fileOrIdentifier);

   if (superprocess->sendMessageToWorker(block))
   {
      std::unique_lock<std::mutex> lock(mutex);
      gotResponse = false;
      pluginDescription = nullptr;

      for (;;)
      {
         if (condvar.wait_for(lock,
                              std::chrono::milliseconds(50),
                              [this]
                              {
                                 return gotResponse || shouldExit();
                              }))
         {
            break;
         }
      }

      if (shouldExit())
      {
         superprocess = nullptr;
         return true;
      }

      if (connectionLost)
      {
         superprocess = nullptr;
         return false;
      }

      if (pluginDescription != nullptr)
      {
         for (const auto* item : pluginDescription->getChildIterator())
         {
            auto desc = std::make_unique<juce::PluginDescription>();

            if (desc->loadFromXml(*item))
               result.add(std::move(desc));
         }
      }

      return true;
   }

   superprocess = nullptr;
   return false;
}

void CustomPluginScanner::changeListenerCallback(juce::ChangeBroadcaster*)
{
   if (auto* file = getAppProperties().getUserSettings())
      scanWithExternalProcess = (file->getIntValue(kScanModeKey) == 0);
}

CustomPluginScanner::Superprocess::Superprocess(CustomPluginScanner& o)
: owner(o)
{
   launchWorkerProcess(juce::File::getSpecialLocation(juce::File::currentExecutableFile), kScanProcessUID, 0, 0);
}

void CustomPluginScanner::Superprocess::handleMessageFromWorker(const juce::MemoryBlock& mb)
{
   auto xml = parseXML(mb.toString());

   const std::lock_guard<std::mutex> lock(owner.mutex);
   owner.pluginDescription = std::move(xml);
   owner.gotResponse = true;
   owner.condvar.notify_one();
}

void CustomPluginScanner::Superprocess::handleConnectionLost()
{
   const std::lock_guard<std::mutex> lock(owner.mutex);
   owner.pluginDescription = nullptr;
   owner.gotResponse = true;
   owner.connectionLost = true;
   owner.condvar.notify_one();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CustomPluginListComponent::CustomPluginListComponent(juce::AudioPluginFormatManager& manager,
                                                     juce::KnownPluginList& listToRepresent,
                                                     const juce::File& pedal,
                                                     juce::PropertiesFile* props,
                                                     bool async)
: juce::PluginListComponent(manager, listToRepresent, pedal, props, async)
{
   addAndMakeVisible(validationModeLabel);
   addAndMakeVisible(validationModeBox);

   validationModeLabel.attachToComponent(&validationModeBox, true);
   validationModeLabel.setJustificationType(juce::Justification::right);
   validationModeLabel.setSize(100, 30);

   auto unusedId = 1;

   for (const auto mode : { "Avoid crashes", "Within process" })
      validationModeBox.addItem(mode, unusedId++);

   validationModeBox.setSelectedItemIndex(getAppProperties().getUserSettings()->getIntValue(kScanModeKey));

   validationModeBox.onChange = [this]
   {
      getAppProperties().getUserSettings()->setValue(kScanModeKey, validationModeBox.getSelectedItemIndex());
   };

   OnResize();
}

void CustomPluginListComponent::OnResize()
{
   const auto& buttonBounds = getOptionsButton().getBounds();
   validationModeBox.setBounds(buttonBounds.withWidth(130).withRightX(getWidth() - buttonBounds.getX()));

   //validationModeBox.setBounds(juce::Rectangle(5,5,130,40));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

PluginListWindow::PluginListWindow(juce::AudioPluginFormatManager& pluginFormatManager, WindowCloseListener* listener)
: juce::DocumentWindow("Plugin Manager",
                       juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                       juce::DocumentWindow::closeButton)
, mListener(listener)
{
   auto deadMansPedalFile(juce::File(ofToDataPath("vst/deadmanspedal.txt")));

   TheSynth->GetKnownPluginList().setCustomScanner(std::make_unique<CustomPluginScanner>());

   mComponent = new CustomPluginListComponent(pluginFormatManager,
                                              TheSynth->GetKnownPluginList(),
                                              deadMansPedalFile,
                                              getAppProperties().getUserSettings(), true);

   setContentOwned(mComponent, true);

   setResizable(true, false);
   setResizeLimits(300, 400, 1500, 1500);
   setSize(600, 600);
   //setAlwaysOnTop(true);

   if (const auto* dpy = juce::Desktop::getInstance().getDisplays().getDisplayForRect(TheSynth->GetMainComponent()->getScreenBounds()))
   {
      const auto& mainMon = dpy->userArea;
      setTopLeftPosition(mainMon.getX() + mainMon.getWidth() / 4,
                         mainMon.getY() + mainMon.getHeight() / 4);
   }

   restoreWindowStateFromString(getAppProperties().getUserSettings()->getValue("listWindowPos"));
   setVisible(true);
}

PluginListWindow::~PluginListWindow()
{
   getAppProperties().getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
   clearContentComponent();
}

void PluginListWindow::resized()
{
   juce::DocumentWindow::resized();

   mComponent->OnResize();
}

void PluginListWindow::closeButtonPressed()
{
   mListener->OnWindowClosed();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

PluginScannerSubprocess::PluginScannerSubprocess()
{
   formatManager.addDefaultFormats();
}

void PluginScannerSubprocess::handleMessageFromCoordinator(const juce::MemoryBlock& mb)
{
   {
      const std::lock_guard<std::mutex> lock(mutex);
      pendingBlocks.emplace(mb);
   }

   triggerAsyncUpdate();
}

void PluginScannerSubprocess::handleConnectionLost()
{
   juce::JUCEApplicationBase::quit();
}

// It's important to run the plugin scan on the main thread!
void PluginScannerSubprocess::handleAsyncUpdate()
{
   for (;;)
   {
      const auto block = [&]() -> juce::MemoryBlock
      {
         const std::lock_guard<std::mutex> lock(mutex);

         if (pendingBlocks.empty())
            return {};

         auto out = std::move(pendingBlocks.front());
         pendingBlocks.pop();
         return out;
      }();

      if (block.isEmpty())
         return;

      juce::MemoryInputStream stream{ block, false };
      const auto formatName = stream.readString();
      const auto identifier = stream.readString();

      juce::OwnedArray<juce::PluginDescription> results;

      for (auto* format : formatManager.getFormats())
         if (format->getName() == formatName)
            format->findAllTypesForFile(results, identifier);

      juce::XmlElement xml("LIST");

      for (const auto& desc : results)
         xml.addChildElement(desc->createXml().release());

      const auto str = xml.toString();
      sendMessageToCoordinator({ str.toRawUTF8(), str.getNumBytesAsUTF8() });
   }
}
