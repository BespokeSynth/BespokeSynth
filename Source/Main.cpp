/*
 ==============================================================================

 This file was auto-generated!

 It contains the basic startup code for a Juce application.

 ==============================================================================
 */

#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
#include "VSTScanner.h"
#include "SynthGlobals.h"

#include "VersionInfo.h"
#include "ModularSynth.h"
#if BESPOKE_IOS
#include "IOSDocuments.h"
#include "IOSDiagnostics.h"
#endif
#if BESPOKE_ANDROID
#include "AndroidSupport.h"
#endif

#if BESPOKE_WINDOWS
#include <dwmapi.h>
#include <windows.h>
#endif

using namespace juce;

Component* createMainContentComponent();
std::unique_ptr<juce::ApplicationProperties> appProperties;
void SetStartupSaveStateFile(const String& bskPath, Component* mainComponent);

juce::ApplicationProperties& getAppProperties()
{
   return *appProperties;
}

//==============================================================================
class BespokeApplication : public JUCEApplication
{
public:
   //==============================================================================
   BespokeApplication() = default;

   const String getApplicationName() override { return Bespoke::APP_NAME; }
   const String getApplicationVersion() override { return Bespoke::VERSION; }
   bool moreThanOneInstanceAllowed() override { return true; }

   //==============================================================================
   void initialise(const String& commandLine) override
   {
      // Parse command line arguments that should cause us to exit
      auto cliArgv = JUCEApplication::getCommandLineParameterArray();
      for (int i = 0; i < cliArgv.size(); ++i)
      {
         bool should_exit = false;
         juce::String argument = cliArgv[i];
         if (argument == "-h" || argument == "--help")
         {
            std::cout << "A modular DAW for Mac, Windows, and Linux.\n"
                      << "\n"
                      << "Usage: BespokeSynth [OPTIONS] [path].json [path].bsk(t)\n"
                      << "\n"
                      << "Arguments:\n"
                      << "  [path].bsk(t)   the project file to open (must end in .bsk or .bskt)\n"
                      << "  [path].json     path to userprefs.json (must end in .json)\n"
                      << "\n"
                      << "Options:\n"
                      << "  -o, --option <option> <value>   Temporarily override settings in preferences file\n"
                      << "  -h, --help                      Print help\n"
                      << "  -v, --version                   Print version\n"
                      << std::flush;
            should_exit = true;
         }
         else if (argument == "-v" || argument == "--version")
         {
            std::cout << "bespoke synth " << GetBuildInfoString() << std::endl;
            should_exit = true;
         }
         else if (argument == "-o" || argument == "--option")
         {
            if ((cliArgv[i + 1].isEmpty()) || (cliArgv[i + 2].isEmpty()))
            {
               CliErrorExpectedOpt(argument);
               should_exit = true;
            }
         }

         if (should_exit == true)
         {
            JUCEApplicationBase::quit();
            return;
         }
      }

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

#if BESPOKE_IOS
      IOSDiagnostics::LogUncaughtExceptions([](const juce::String& message)
                                            {
                                               ofLog() << message.toStdString();
                                            });
      IOSDiagnostics::LogAudioSessionEvents([](const juce::String& message)
                                            {
                                               ofLog() << message.toStdString();
                                            });
      IOSDocuments::SetOpenFileHandler([](const juce::File& file)
                                       {
                                          OpenDocumentFromOtherApp(file);
                                       });
#endif
#if BESPOKE_ANDROID
      mOpenedDocumentChecker.startTimer(500);
#endif

#if BESPOKE_WINDOWS
      HWND hwnd = (HWND)this->mainWindow.get()->getWindowHandle();

      BOOL darkMode = Desktop::getInstance().isDarkModeActive();
      DwmSetWindowAttribute((HWND)hwnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
      UpdateWindow(hwnd);
#endif
   }

#if BESPOKE_MOBILE
   //files opened from other apps may only be readable temporarily, so copy them into our savestate folder, then open the copy
   static void OpenDocumentFromOtherApp(const juce::File& source)
   {
      if (!source.hasFileExtension(".bsk") || TheSynth == nullptr)
      {
         ofLog() << "can't open " << source.getFileName() << ", bespoke can only open .bsk files";
         return;
      }

      juce::File savestateDir(ofToDataPath("savestate"));
      juce::File dest = source;
      if (!source.isAChildOf(savestateDir))
      {
         savestateDir.createDirectory();
         dest = savestateDir.getChildFile(source.getFileName());
         if (dest.existsAsFile() && !dest.hasIdenticalContentTo(source)) //don't overwrite a different patch with the same name
            dest = savestateDir.getNonexistentChildFile(source.getFileNameWithoutExtension(), ".bsk", true);
         if (!dest.existsAsFile() && !source.copyFileTo(dest))
            return;

#if BESPOKE_IOS
         //iOS puts some opened files in Documents/Inbox for us to take, so clean those up
         juce::File inbox = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Inbox");
         if (source.isAChildOf(inbox))
            source.deleteFile();
#endif
      }

      TheSynth->OpenSaveStateFile(dest.getFullPathName().toStdString());
   }
#endif

   // Prints an error for arguments that expected an argument but were not given one
   void CliErrorExpectedOpt(String argument)
   {
      std::cout << "Error: value is required for '" << argument << "' but none was supplied"
                << "\n\nFor more information, try '--help'"
                << std::endl;
   }

   void shutdown() override
   {
      // Add your application's shutdown code here..
#if BESPOKE_ANDROID
      mOpenedDocumentChecker.stopTimer();
#endif
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

   void anotherInstanceStarted(const String& commandLine) override
   {
      // When another instance of the app is launched while this one is running,
      // this method is invoked, and the commandLine parameter tells you what
      // the other instance's command-line arguments were.

      // This is also called when opening the app with a file.
      if (commandLine.isNotEmpty() && commandLine.endsWith(".bsk"))
         SetStartupSaveStateFile(commandLine, mainWindow->getContentComponent());
   }

   //==============================================================================
   /*
    This class implements the desktop window that contains an instance of
    our MainContentComponent class.
    */
   class MainWindow : public DocumentWindow
   {
   public:
      MainWindow(String name)
      : DocumentWindow(name,
                       Colours::lightgrey,
                       DocumentWindow::allButtons)
      {
         setUsingNativeTitleBar(true);
         setContentOwned(createMainContentComponent(), true);
#if BESPOKE_MOBILE
         setFullScreen(true);
#else
         setResizable(true, true);

         centreWithSize(getWidth(), getHeight());
#endif
         setVisible(true);
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
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
   };

private:
#if BESPOKE_ANDROID
   //other apps open documents with bespoke through BespokeActivity.java, which holds on to them until this collects them
   struct OpenedDocumentChecker : public juce::Timer
   {
      void timerCallback() override
      {
         if (TheSynth == nullptr)
            return; //leave it waiting until there's a synth to open it in

         juce::URL document = AndroidSupport::TakeOpenedDocument();
         if (document.isEmpty())
            return;

         //content:// documents can only be read while bespoke has the permission that came with them, so copy it now
         auto result = AndroidSupport::CopyDocumentToMirror(document);
         if (result.failed())
         {
            ofLog() << "couldn't open document: " << result.getErrorMessage();
            return;
         }
         OpenDocumentFromOtherApp(WithBskExtension(AndroidSupport::GetDocumentMirror(document)));
      }

      //the user chose to open this with bespoke, so treat it as a .bsk even if its name doesn't say so. android's
      //downloads name duplicates like "song.bsk (1)", which becomes "song (1).bsk"
      static juce::File WithBskExtension(const juce::File& file)
      {
         juce::String name = file.getFileName();
         if (name.endsWithIgnoreCase(".bsk"))
            return file;

         if (name.containsIgnoreCase(".bsk"))
            name = name.upToLastOccurrenceOf(".bsk", false, true) + name.fromLastOccurrenceOf(".bsk", false, true) + ".bsk";
         else
            name += ".bsk";

         juce::File renamed = file.getSiblingFile(juce::File::createLegalFileName(name));
         return file.moveFileTo(renamed) ? renamed : file;
      }
   };
   OpenedDocumentChecker mOpenedDocumentChecker;
#endif

   std::unique_ptr<MainWindow> mainWindow;
   std::unique_ptr<PluginScannerSubprocess> storedScannerSubprocess;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(BespokeApplication)
