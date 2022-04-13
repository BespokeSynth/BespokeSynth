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
//  VSTScanner.h
//  Bespoke
//
//  Created by Ryan Challinor on 10/16/21.
//
//

#pragma once

#include "WindowCloseListener.h"

#include "juce_audio_processors/juce_audio_processors.h"

constexpr const char* kScanProcessUID = "bespokesynth";
constexpr const char* kScanModeKey = "pluginScanMode";

class CustomPluginScanner : public juce::KnownPluginList::CustomScanner,
                            private juce::ChangeListener
{
public:
   CustomPluginScanner();
   ~CustomPluginScanner() override;

   bool findPluginTypesFor(juce::AudioPluginFormat& format,
                           juce::OwnedArray<juce::PluginDescription>& result,
                           const juce::String& fileOrIdentifier) override;
   void scanFinished() override { superprocess = nullptr; }

private:
   class Superprocess : public juce::ChildProcessCoordinator
   {
   public:
      explicit Superprocess(CustomPluginScanner& o);

   private:
      void handleMessageFromWorker(const juce::MemoryBlock& mb) override;
      void handleConnectionLost() override;

      CustomPluginScanner& owner;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Superprocess)
   };

   void changeListenerCallback(juce::ChangeBroadcaster*) override;

   std::unique_ptr<Superprocess> superprocess;
   std::mutex mutex;
   std::condition_variable condvar;
   std::unique_ptr<juce::XmlElement> pluginDescription;
   bool gotResponse = false;
   bool connectionLost = false;

   std::atomic<bool> scanWithExternalProcess{ true };

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomPluginScanner)
};

//==============================================================================
class CustomPluginListComponent : public juce::PluginListComponent
{
public:
   CustomPluginListComponent(juce::AudioPluginFormatManager& manager,
                             juce::KnownPluginList& listToRepresent,
                             const juce::File& pedal,
                             juce::PropertiesFile* props,
                             bool async);

   void OnResize();

private:
   juce::Label validationModeLabel{ {}, "Scan mode" };
   juce::ComboBox validationModeBox;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomPluginListComponent)
};

class PluginListWindow : public juce::DocumentWindow
{
public:
   PluginListWindow(juce::AudioPluginFormatManager& pluginFormatManager, WindowCloseListener* listener);
   ~PluginListWindow() override;

   void resized() override;
   void closeButtonPressed() override;

private:
   WindowCloseListener* mListener;
   CustomPluginListComponent* mComponent;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};

class PluginScannerSubprocess : private juce::ChildProcessWorker,
                                private juce::AsyncUpdater
{
public:
   PluginScannerSubprocess();

   using juce::ChildProcessWorker::initialiseFromCommandLine;

private:
   void handleMessageFromCoordinator(const juce::MemoryBlock& mb) override;
   void handleConnectionLost() override;

   // It's important to run the plugin scan on the main thread!
   void handleAsyncUpdate() override;

   juce::AudioPluginFormatManager formatManager;

   std::mutex mutex;
   std::queue<juce::MemoryBlock> pendingBlocks;
};
