/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

namespace CommandIDs
{
   static const int showPluginListEditor = 0x30100;
   static const int saveForBespoke = 0x30101;
}

//==============================================================================
class BespokeVSTScannerApplication : public JUCEApplication
{
public:
   //==============================================================================
   BespokeVSTScannerApplication() {}

   const String getApplicationName() override { return ProjectInfo::projectName; }
   const String getApplicationVersion() override { return ProjectInfo::versionString; }
   bool moreThanOneInstanceAllowed() override { return true; }

   //==============================================================================
   void initialise(const String& commandLine) override
   {
      // This method is where you should put your application's initialisation code..

      PropertiesFile::Options options;
      options.applicationName = "Bespoke VST Scanner";
      options.filenameSuffix = "settings";
      options.osxLibrarySubFolder = "Preferences";

      appProperties.reset(new ApplicationProperties());
      appProperties->setStorageParameters(options);

      mainWindow.reset(new MainWindow(getApplicationName()));
   }

   void shutdown() override
   {
      // Add your application's shutdown code here..

      mainWindow = nullptr; // (deletes our window)
   }

   //==============================================================================
   void systemRequestedQuit() override
   {
      // This is called when the app is being asked to quit: you can ignore this
      // request and let the app carry on running, or call quit() to allow the app to close.
      quit();
   }

   void anotherInstanceStarted(const String& commandLine) override
   {
      // When another instance of the app is launched while this one is running,
      // this method is invoked, and the commandLine parameter tells you what
      // the other instance's command-line arguments were.
   }

   ApplicationCommandManager commandManager;
   std::unique_ptr<ApplicationProperties> appProperties;

   static BespokeVSTScannerApplication& getApp() { return *dynamic_cast<BespokeVSTScannerApplication*>(JUCEApplication::getInstance()); }

   class PluginListWindow;

   //==============================================================================
   /*
       This class implements the desktop window that contains an instance of
       our MainComponent class.
   */
   class MainWindow : public DocumentWindow, 
                      public MenuBarModel,
                      public ApplicationCommandTarget
   {
   public:
      AudioPluginFormatManager formatManager;
      KnownPluginList knownPluginList;
      std::unique_ptr<PluginListWindow> pluginListWindow;

      MainWindow(String name) : DocumentWindow(name,
         Desktop::getInstance().getDefaultLookAndFeel()
         .findColour(ResizableWindow::backgroundColourId),
         DocumentWindow::allButtons)
      {
         setUsingNativeTitleBar(true);
         setContentOwned (new MainComponent(), true);

         getApp().commandManager.registerAllCommandsForTarget(this);

         /*juce::AudioPluginFormatManager pluginFormatManager;
         juce::KnownPluginList knownPluginList;
         pluginFormatManager.addDefaultFormats();
         auto deadMansPedalFile = getApp().appProperties->getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");

         setContentOwned(new juce::PluginListComponent(pluginFormatManager,
            knownPluginList,
            deadMansPedalFile,
            nullptr, true), true);*/

         setResizable(true, true);
         centreWithSize(getWidth(), getHeight());
         setMenuBar(this);
         formatManager.addDefaultFormats();

         setVisible(true);
      }

      virtual ~MainWindow()
      {
         setMenuBar(nullptr);
      }

      void closeButtonPressed() override
      {
         // This is called when the user tries to close this window. Here, we'll just
         // ask the app to quit when this happens, but you can change this to do
         // whatever you need.
         JUCEApplication::getInstance()->systemRequestedQuit();
      }

      StringArray getMenuBarNames() override
      {
         StringArray names;
         names.add("File");
         return names;
      }

      PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& /*menuName*/)
      {
         PopupMenu menu;

         if (topLevelMenuIndex == 0)
         {
            // "File" menu
            menu.addCommandItem(&getApp().commandManager, CommandIDs::showPluginListEditor);
            menu.addCommandItem(&getApp().commandManager, CommandIDs::saveForBespoke);
            menu.addSeparator();
            menu.addCommandItem(&getApp().commandManager, StandardApplicationCommandIDs::quit);
         }

         return menu;
      }

      void menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
      {
      }

      ApplicationCommandTarget* getNextCommandTarget() override
      {
         return findFirstTargetParentComponent();
      }

      void getAllCommands(Array<CommandID>& commands) override
      {
         // this returns the set of all commands that this target can perform..
         const CommandID ids[] = {
                                   CommandIDs::showPluginListEditor,
                                   CommandIDs::saveForBespoke
         };

         commands.addArray(ids, numElementsInArray(ids));
      }

      void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override
      {
         const String category("General");

         switch (commandID)
         {
         case CommandIDs::showPluginListEditor:
            result.setInfo("Edit the List of Available Plug-ins...", {}, category, 0);
            result.addDefaultKeypress('p', ModifierKeys::commandModifier);
            break;

         case CommandIDs::saveForBespoke:
            result.setInfo("Export plugins list to Bespoke", {}, category, 0);
            //result.addDefaultKeypress('p', ModifierKeys::commandModifier);
            break;
         }
      }

      bool perform(const InvocationInfo& info) override
      {
         switch (info.commandID)
         {
         case CommandIDs::showPluginListEditor:
         {
            if (pluginListWindow == nullptr)
               pluginListWindow.reset(new BespokeVSTScannerApplication::PluginListWindow(*this, formatManager));

            pluginListWindow->toFront(true);
            break;
         }
         case CommandIDs::saveForBespoke:
         {
            String dataDir = File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("BespokeSynth").getFullPathName().toStdString();
            dataDir.replace("\\", "/");

            knownPluginList.createXml()->writeTo(juce::File(dataDir + "/vst/found_vsts.xml"));
            break;
         }
         default:
            return false;
         }

         return true;
      }

      /* Note: Be careful if you override any DocumentWindow methods - the base
         class uses a lot of them, so by overriding you might break its functionality.
         It's best to do all your work in your content component instead, but if
         you really have to override any DocumentWindow methods, make sure your
         subclass also calls the superclass's method.
      */

   private:
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
   };

   class PluginListWindow : public DocumentWindow
   {
   public:
      PluginListWindow(MainWindow& mw, AudioPluginFormatManager& pluginFormatManager)
         : DocumentWindow("Available Plugins",
            LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::minimiseButton | DocumentWindow::closeButton),
         owner(mw)
      {
         auto deadMansPedalFile = getApp().appProperties->getUserSettings()
            ->getFile().getSiblingFile("RecentlyCrashedPluginsList");

         setContentOwned(new PluginListComponent(pluginFormatManager,
            owner.knownPluginList,
            deadMansPedalFile,
            getApp().appProperties->getUserSettings(), true), true);

         setResizable(true, false);
         setResizeLimits(300, 400, 800, 1500);
         setTopLeftPosition(60, 60);

         restoreWindowStateFromString(getApp().appProperties->getUserSettings()->getValue("listWindowPos"));
         setVisible(true);
      }

      ~PluginListWindow() override
      {
         getApp().appProperties->getUserSettings()->setValue("listWindowPos", getWindowStateAsString());
         clearContentComponent();
      }

      void closeButtonPressed() override
      {
         owner.pluginListWindow = nullptr;
      }

   private:
      MainWindow& owner;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
   };

private:
   std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(BespokeVSTScannerApplication)
