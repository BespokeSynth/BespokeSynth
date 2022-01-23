/*
 ==============================================================================
 
 This file was auto-generated!
 
 It contains the basic startup code for a Juce application.
 
 ==============================================================================
 */

#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
#include "VSTScanner.h"

#include "VersionInfo.h"

using namespace juce;

Component* createMainContentComponent();
std::unique_ptr<juce::ApplicationProperties> appProperties;
void SetStartupSaveStateFile(const String& bskPath, Component* mainComponent);

juce::ApplicationProperties& getAppProperties()
{
   return *appProperties;
}

//==============================================================================
class BespokeApplication  : public JUCEApplication
{
public:
   //==============================================================================
   BespokeApplication() = default;
   
   const String getApplicationName() override       { return Bespoke::APP_NAME; }
   const String getApplicationVersion() override    { return Bespoke::VERSION; }
   bool moreThanOneInstanceAllowed() override       { return true; }
   
   //==============================================================================
   void initialise (const String& commandLine) override
   {
      auto scannerSubprocess = std::make_unique<PluginScannerSubprocess>();

      if (scannerSubprocess->initialiseFromCommandLine(commandLine, kScanProcessUID))
      {
         storedScannerSubprocess = std::move(scannerSubprocess);
         return;
      }
      
      mainWindow = std::make_unique<MainWindow>("bespoke synth");

      juce::PropertiesFile::Options options;
      options.applicationName = "Bespoke Synth";
      options.filenameSuffix = "settings";
      options.osxLibrarySubFolder = "Preferences";

      appProperties = std::make_unique<juce::ApplicationProperties>();
      appProperties->setStorageParameters(options);
   }
   
   void shutdown() override
   {
      // Add your application's shutdown code here..
      mainWindow.reset();
      appProperties.reset();
   }
   
   //==============================================================================
   void systemRequestedQuit() override
   {
      // This is called when the app is being asked to quit: you can ignore this
      // request and let the app carry on running, or call quit() to allow the app to close.
      quit();
   }
   
   void anotherInstanceStarted (const String& commandLine) override
   {
      // When another instance of the app is launched while this one is running,
      // this method is invoked, and the commandLine parameter tells you what
      // the other instance's command-line arguments were.

      // This is also called when opening the app with a file.
      if(commandLine.isNotEmpty() && commandLine.endsWith(".bsk"))
        SetStartupSaveStateFile(commandLine, mainWindow->getContentComponent());
   }
   
   //==============================================================================
   /*
    This class implements the desktop window that contains an instance of
    our MainContentComponent class.
    */
   class MainWindow    : public DocumentWindow
   {
   public:
      MainWindow (String name)  : DocumentWindow (name,
                                                  Colours::lightgrey,
                                                  DocumentWindow::allButtons)
      {
         setUsingNativeTitleBar (true);
         setContentOwned (createMainContentComponent(), true);
         setResizable (true, true);
         
         centreWithSize (getWidth(), getHeight());
         setVisible (true);
      }
      
      void closeButtonPressed() override
      {
         // This is called when the user tries to close this window. Here, we'll just
         // ask the app to quit when this happens, but you can change this to do
         // whatever you need.
         JUCEApplication::getInstance()->systemRequestedQuit();
      }
      
      /* Note: Be careful if you override any DocumentWindow methods - the base
       class uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in your content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       subclass also calls the superclass's method.
       */
      
   private:
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
   };
   
private:
   std::unique_ptr<MainWindow> mainWindow;
   std::unique_ptr<PluginScannerSubprocess> storedScannerSubprocess;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (BespokeApplication)
