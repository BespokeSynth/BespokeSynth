#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_opengl/juce_opengl.h"
using namespace juce::gl;
using namespace juce;

#include "VersionInfo.h"

#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "Push2Control.h"  //TODO(Ryan) remove
#include "SpaceMouseControl.h"
#include "UserPrefs.h"

#ifdef JUCE_WINDOWS
#include <windows.h>
#endif

//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
class MainContentComponent   : public OpenGLAppComponent,
                               public AudioIODeviceCallback,
                               public FileDragAndDropTarget,
                               private Timer
{
public:
   //==============================================================================
   MainContentComponent()
   : mLastFpsUpdateTime(0)
   , mFrameCountAccum(0)
   , mPixelRatio(1)
   , mSpaceMouseReader(mSynth)
   {
      ofLog() << "bespoke synth " << GetBuildInfoString();

      // sigh ofLog isn't a stream so std::hex doesn't work so
      char jv[256];
      snprintf(jv, 255, "%x", JUCE_VERSION);
      ofLog() << "   juce version    : " << jv;
      ofLog() << "   python version  : " << Bespoke::PYTHON_VERSION;
#if BESPOKE_LINUX
      ofLog() << "   install prefix  : '" << Bespoke::CMAKE_INSTALL_PREFIX << "'";
#endif
      ofLog() << "   git hash        : " << Bespoke::GIT_HASH;
      ofLog() << "   git branch      : " << Bespoke::GIT_BRANCH;
      ofLog() << "   build time      : " << Bespoke::BUILD_DATE << " at " << Bespoke::BUILD_TIME;
      ofLog() << "   command line    : " << JUCEApplication::getCommandLineParameters();

      openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
      openGLContext.setContinuousRepainting(false);
#if BESPOKE_LINUX_HIGH_FPS_WITH_THREAD_PROBLEMS
      openGLContext.setComponentPaintingEnabled(false);
#endif

#ifndef BESPOKE_WINDOWS //windows crash handler is set up in ModularSynth() constructor
      SystemStats::setApplicationCrashHandler(ModularSynth::CrashHandler);
#endif

      UserPrefs.Init();
      
      int screenWidth, screenHeight;
      {
         const MessageManagerLock lock;
         if (const auto* dpy = Desktop::getInstance().getDisplays().getPrimaryDisplay())
         {
            mPixelRatio = dpy->scale;
            TheSynth->SetPixelRatio(mPixelRatio);
         }
         auto bounds = Desktop::getInstance().getDisplays().getTotalBounds(true);
         screenWidth = bounds.getWidth();
         screenHeight = bounds.getHeight();
         ofLog() << "pixel ratio: " << mPixelRatio << " screen width: " << screenWidth << " screen height: " << screenHeight;
      }
      
      int width = UserPrefs.width.Get();
      int height = UserPrefs.height.Get();
      mDesiredInitialPosition.setXY(INT_MAX, INT_MAX);
      
      if (UserPrefs.set_manual_window_position.Get())
      {
         mDesiredInitialPosition.setXY(UserPrefs.position_x.Get(), UserPrefs.position_y.Get());
      }
      else
      {
         if (width + getTopLevelComponent()->getPosition().x > screenWidth)
            width = screenWidth - getTopLevelComponent()->getPosition().x;
         if (height + getTopLevelComponent()->getPosition().y + 20 > screenHeight)
            height = screenHeight - getTopLevelComponent()->getPosition().y - 20;
      }
      
      setSize(width, height);
      setWantsKeyboardFocus(true);
      Desktop::setScreenSaverEnabled(false);
      mGlobalManagers.mDeviceManager.getAvailableDeviceTypes();   //scans for device types ("Windows Audio", "DirectSound", etc)
   }
   
   ~MainContentComponent()
   {
      shutdownOpenGL();
      shutdownAudio();
   }
   
   void timerCallback() override
   {
      static int sRenderFrame = 0;
      if (sRenderFrame == 0 && mDesiredInitialPosition.x != INT_MAX)
         getTopLevelComponent()->setTopLeftPosition(mDesiredInitialPosition);

      static bool sHasGrabbedFocus = false;
      if (!sHasGrabbedFocus && !hasKeyboardFocus(true) && isVisible())
      {
         grabKeyboardFocus();
         sHasGrabbedFocus = true;
      }
      
      mSynth.Poll();
      
#if DEBUG || (BESPOKE_LINUX && !BESPOKE_LINUX_HIGH_FPS_WITH_THREAD_PROBLEMS)
      if (sRenderFrame % 2 == 0)
#else
      if (true)
#endif
      {
         openGLContext.triggerRepaint();
      }
      ++sRenderFrame;

      if (sRenderFrame % 30 == 0)
      {
         if (const auto* dpy = Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds()))
         {
            mPixelRatio = dpy->scale; //adjust pixel ratio based on which screen has the majority of the window
            TheSynth->SetPixelRatio(mPixelRatio);
         }
      }
      
      mScreenPosition = getScreenPosition();

      mSpaceMouseReader.Poll();
   }
   
   //==============================================================================
   void audioDeviceAboutToStart(AudioIODevice* device) override
   {
      // This function will be called when the audio device is started, or when
      // its settings (i.e. sample rate, block size, etc) are changed.
      
      // You can use this function to initialise any resources you might need,
      // but be careful - it will be called on the audio thread, not the GUI thread.
   }
   
   void audioDeviceIOCallback(const float** inputChannelData,
                              int numInputChannels,
                              float** outputChannelData,
                              int numOutputChannels,
                              int numSamples) override
   {
      mSynth.AudioIn(inputChannelData, numSamples, numInputChannels);
      mSynth.AudioOut(outputChannelData, numSamples, numOutputChannels);
   }
   
   void audioDeviceStopped() override
   {
      
   }
   
   void shutdownAudio()
   {
      mGlobalManagers.mDeviceManager.removeAudioCallback(this);
      mGlobalManagers.mDeviceManager.closeAudioDevice();
   }
   
   void initialise() override
   {
#ifdef JUCE_WINDOWS
      // glewInit();
#endif
      
      mVG = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
      mFontBoundsVG = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
      
      if (mVG == nullptr)
         printf("Could not init nanovg.\n");
      if (mFontBoundsVG == nullptr)
         printf("Could not init font bounds nanovg.\n");

      Push2Control::CreateStaticFramebuffer();
      
      mSynth.LoadResources(mVG, mFontBoundsVG);
      
      /*for (auto deviceType : mGlobalManagers.mDeviceManager.getAvailableDeviceTypes())
      {
         ofLog() << "inputs:";
         for (auto input : deviceType->getDeviceNames(true))
            ofLog() << input.toStdString();
         ofLog() << "outputs:";
         for (auto output : deviceType->getDeviceNames(false))
            ofLog() << output.toStdString();
      }*/

      const std::string kAutoDevice = "auto";
      const std::string kNoneDevice = "none";

      if (UserPrefs.devicetype.Get() != kAutoDevice)
         mGlobalManagers.mDeviceManager.setCurrentAudioDeviceType(UserPrefs.devicetype.Get(), true);

      SetGlobalSampleRateAndBufferSize(UserPrefs.samplerate.Get(), UserPrefs.buffersize.Get());
      
      mSynth.Setup(&mGlobalManagers.mDeviceManager, &mGlobalManagers.mAudioFormatManager, this, &openGLContext);

      std::string outputDevice = UserPrefs.audio_output_device.Get();
      std::string inputDevice = UserPrefs.audio_input_device.Get();
      if (!mGlobalManagers.mDeviceManager.getCurrentDeviceTypeObject()->hasSeparateInputsAndOutputs())
         inputDevice = outputDevice;    //asio must have identical input and output
      
      AudioDeviceManager::AudioDeviceSetup preferredSetupOptions;
      preferredSetupOptions.sampleRate = gSampleRate;
      preferredSetupOptions.bufferSize = gBufferSize;
      if (outputDevice != kAutoDevice && outputDevice != kNoneDevice)
         preferredSetupOptions.outputDeviceName = outputDevice;
      if (inputDevice != kAutoDevice && inputDevice != kNoneDevice)
         preferredSetupOptions.inputDeviceName = inputDevice;

#ifdef JUCE_WINDOWS
      HRESULT hr;
      hr = CoInitializeEx(0, COINIT_MULTITHREADED);
#endif
      
      int inputChannels = 16;
      int outputChannels = 16;
      
      if (inputDevice == kNoneDevice)
         inputChannels = 0;
      if (outputDevice == kNoneDevice)
         outputChannels = 0;

      String audioError = mGlobalManagers.mDeviceManager.initialise(inputChannels,
                                                                    outputChannels,
                                                                    nullptr,
                                                                    true,
                                                                    "",
                                                                    &preferredSetupOptions);

      if (audioError.isEmpty())
      {
         auto loadedSetup = mGlobalManagers.mDeviceManager.getAudioDeviceSetup();
         if (outputDevice != kAutoDevice && outputDevice != kNoneDevice &&
             loadedSetup.outputDeviceName.toStdString() != outputDevice)
         {
            mSynth.SetFatalError("error setting output device to '"+outputDevice+"', fix this in userprefs.json (use \"auto\" for default device)"+
                                 "\n\n\nvalid devices:\n"+GetAudioDevices());
         }
         else if (inputDevice != kAutoDevice && inputDevice != kNoneDevice &&
                  loadedSetup.inputDeviceName.toStdString() != inputDevice)
         {
            mSynth.SetFatalError("error setting input device to '"+inputDevice+"', fix this in userprefs.json (use \"auto\" for default device, or \"none\" for no device)"+
                                 "\n\n\nvalid devices:\n"+GetAudioDevices());
         }
         else if (loadedSetup.bufferSize != gBufferSize)
         {
            mSynth.SetFatalError("error setting buffer size to "+ofToString(gBufferSize)+" on device '"+ loadedSetup.outputDeviceName.toStdString()+"', fix this in userprefs.json" +
                                 "\n\n(a valid buffer size might be: " + ofToString(loadedSetup.bufferSize) + ")");
         }
         else if (loadedSetup.sampleRate != gSampleRate)
         {
            mSynth.SetFatalError("error setting sample rate to "+ofToString(gSampleRate) + " on device '" + loadedSetup.outputDeviceName.toStdString() + "', fix this in userprefs.json"+
                                 "\n\n(a valid sample rate might be: "+ofToString(loadedSetup.sampleRate)+")");
         }
         else
         {            
            ofLog() << "output: " << loadedSetup.outputDeviceName << "   input: " << loadedSetup.inputDeviceName;

            int numInputChannels = 0;
            int64 inputMask = loadedSetup.inputChannels.toInteger();
            while (inputMask != 0)
            {
               ++numInputChannels;
               inputMask >>= 1;
            }

            int numOutputChannels = 0;
            int64 outputMask = loadedSetup.outputChannels.toInteger();
            while (outputMask != 0)
            {
               ++numOutputChannels;
               outputMask >>= 1;
            }

            mSynth.InitIOBuffers(numInputChannels, numOutputChannels);

            mGlobalManagers.mDeviceManager.addAudioCallback(this);
         }
      }
      else
      {
         if (audioError.startsWith("No such device"))
            audioError += "\n\nfix this in userprefs.json (you can use \"auto\" for the default device)";
         else
            audioError += juce::String("\n\nattempted to set output to: "+outputDevice+" and input to: "+inputDevice+"\n\ninitialization errors could potentially be fixed by changing buffer size, sample rate, or input/output devices in userprefs.json\nto use no input device, specify \"none\" for \"audio_input_device\"");
         mSynth.SetFatalError("error initializing audio device: "+audioError.toStdString() +
                              "\n\n\nvalid devices:\n" + GetAudioDevices());
      }

      for (int i = 0; i < JUCEApplication::getCommandLineParameterArray().size(); ++i)
      {
         juce::String argument = JUCEApplication::getCommandLineParameterArray()[i];
         if (argument.endsWith(".bsk"))
         {
            mSynth.SetStartupSaveStateFile(argument.toStdString());
            break;
         }
      }
      
      startTimerHz(60);
   }
   
   void shutdown() override
   {
      nvgDeleteGLES2(mVG);
      nvgDeleteGLES2(mFontBoundsVG);
   }

   void SetStartupSaveStateFile(const juce::String& bskPath)
   {
      mSynth.SetStartupSaveStateFile(bskPath.toStdString());
   }
   
   void render() override
   {
      if (mSynth.IsLoadingState())
         return;
      
      mSynth.LockRender(true);
      
      juce::Point<int> mouse = Desktop::getMousePosition();
      mouse -= mScreenPosition;
      mSynth.MouseMoved(mouse.x, mouse.y);
      
      float width = getWidth();
      float height = getHeight();
      
      static float kMotionTrails = .4f;
      
      ofVec3f bgColor(ModularSynth::sBackgroundR, ModularSynth::sBackgroundG, ModularSynth::sBackgroundB);
      glViewport(0, 0, width*mPixelRatio, height*mPixelRatio);
      glClearColor(bgColor.x,bgColor.y,bgColor.z,0);
      if (kMotionTrails <= 0)
         glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
      
      nvgBeginFrame(mVG, width, height, mPixelRatio);
      
      if (kMotionTrails > 0)
      {
         ofSetColor(bgColor.x*255,bgColor.y*255,bgColor.z*255,(1-kMotionTrails*(ofGetFrameRate()/60.0f))*255);
         ofFill();
         ofRect(0,0,width,height);
      }
      
      nvgLineCap(mVG, NVG_ROUND);
      nvgLineJoin(mVG, NVG_ROUND);
      static float sSpacing = -.3f;
      nvgTextLetterSpacing(mVG, sSpacing);
      nvgTextLetterSpacing(mFontBoundsVG, sSpacing);
      
      mSynth.Draw(mVG);
      
      nvgEndFrame(mVG);
      
      mSynth.PostRender();
      
      mSynth.LockRender(false);
      
      ++mFrameCountAccum;
      int64 time = Time::currentTimeMillis();
      
      const int64 kCalcFpsIntervalMs = 1000;
      if (time - mLastFpsUpdateTime >= kCalcFpsIntervalMs)
      {
         mSynth.UpdateFrameRate(mFrameCountAccum / (kCalcFpsIntervalMs / 1000.0f));
         
         mFrameCountAccum = 0;
         mLastFpsUpdateTime = time;
      }
   }
   
   //==============================================================================
   void paint(Graphics& g) override
   {
   }
   
   void resized() override
   {
      // This is called when the MainContentComponent is resized.
      // If you add any child components, this is where you should
      // update their positions.
   }

private:
   int GetMouseButton(const MouseEvent& e)
   {
      if (e.mods.isPopupMenu())
         return 2;
      if (e.mods.isMiddleButtonDown())
         return 3;
      return 1;
   }

   void mouseDown(const MouseEvent& e) override
   {
      mSynth.MousePressed(e.getMouseDownX(), e.getMouseDownY(), GetMouseButton(e), e.source);
   }
   
   void mouseUp(const MouseEvent& e) override
   {
      mSynth.MouseReleased(e.getPosition().x, e.getPosition().y, GetMouseButton(e), e.source);
   }
   
   void mouseDrag(const MouseEvent& e) override
   {
      mSynth.MouseDragged(e.getPosition().x, e.getPosition().y, GetMouseButton(e), e.source);
   }
   
   void mouseMove(const MouseEvent& e) override
   {
      //Don't do mouse move in here, it really slows UI responsiveness in some scenarios. We do it when we render instead.
      //mSynth.MouseMoved(e.getPosition().x, e.getPosition().y);
   }
   
   void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override
   {
      float invert = 1;
      if (wheel.isReversed)
         invert = -1;

      float scale = 6;
      if (wheel.isSmooth)
         scale = 30;

      if (!wheel.isInertial)
         mSynth.MouseScrolled(wheel.deltaX * scale, wheel.deltaY * scale * invert, true);
   }
   
   void mouseMagnify(const MouseEvent& e, float scaleFactor) override
   {
      mSynth.MouseMagnify(e.getPosition().x, e.getPosition().y, scaleFactor, e.source);
   }
   
   bool keyPressed(const KeyPress& key) override
   {
      /*
       * This is a temporary fix for 1.0.1. This keyPressed handler
       * always returns true whether or not Bespoke handles the event
       * and with juce 6.1.1 it gets all the events. That 'return true'
       * therefore suppresses the cmd-q/alt-f4 to quit.
       *
       * The correct fix is take every key handler and make it have
       * a return type bool and then at the end return true if any
       * subordinate key handler returns true, and false if not,
       * but that touches the entire codebase, so to fix the 1.0 to
       * 1.0.1. regression with command q on macos, just for now do this
       * and if it gets merged, open an issue.
       */
#if BESPOKE_MAC
      if (key.getKeyCode() == 'Q' && key.getModifiers().isCommandDown())
      {
         return false;
      }
#else
      if (key.getKeyCode() == KeyPress::F4Key && key.getModifiers().isAltDown())
      {
         return false;
      }
#endif

      int keyCode = key.getTextCharacter();
      if (keyCode < 32)
         keyCode = key.getKeyCode();
      bool isRepeat = true;
      if (find(mPressedKeys.begin(), mPressedKeys.end(), keyCode) == mPressedKeys.end())
      {
         mPressedKeys.push_back(keyCode);
         isRepeat = false;
      }
      mSynth.KeyPressed(keyCode, isRepeat);
      return true;
   }
   
   bool keyStateChanged(bool isKeyDown) override
   {
      if (!isKeyDown)
      {
         for (int keyCode : mPressedKeys)
         {
            if (!KeyPress::isKeyCurrentlyDown(keyCode))
            {
               mPressedKeys.remove(keyCode);
               mSynth.KeyReleased(keyCode);
               break;
            }
         }
      }
      return false;
   }

   void focusGained(FocusChangeType cause) override
   {
      mSynth.Focus();
   }

   bool isInterestedInFileDrag(const StringArray& files) override
   {
      //TODO_PORT(Ryan)
      return true;
   }
   
   void filesDropped(const StringArray& files, int x, int y) override
   {
      std::vector<std::string> strFiles;
      for (auto file : files)
         strFiles.push_back(file.toStdString());
      mSynth.FilesDropped(strFiles, x, y);
   }
   
   std::string GetAudioDevices()
   {
      std::string ret;
      OwnedArray<AudioIODeviceType> types;
      mGlobalManagers.mDeviceManager.createAudioDeviceTypes(types);
      for (int i = 0; i < types.size(); ++i)
      {
         String typeName(types[i]->getTypeName());  // This will be things like "DirectSound", "CoreAudio", etc.
         types[i]->scanForDevices();                 // This must be called before getting the list of devices

         ret += "output:\n";
         {
            StringArray deviceNames(types[i]->getDeviceNames(false));
            for (int j = 0; j < deviceNames.size(); ++j)
               ret += typeName.toStdString() + ": " + deviceNames[j].toStdString() + "\n";
         }

         ret += "\ninput:\n";
         {
            StringArray deviceNames(types[i]->getDeviceNames(true));
            for (int j = 0; j < deviceNames.size(); ++j)
               ret += typeName.toStdString() + ": " + deviceNames[j].toStdString() + "\n";
         }

         ret += "\n";
      }
      return ret;
   }

   struct
   {
      juce::AudioDeviceManager mDeviceManager;
      juce::AudioFormatManager mAudioFormatManager;
   } mGlobalManagers;

   ModularSynth mSynth;
   
   NVGcontext* mVG;
   NVGcontext* mFontBoundsVG;
   int64 mLastFpsUpdateTime;
   int mFrameCountAccum;
   std::list<int> mPressedKeys;
   double mPixelRatio;
   juce::Point<int> mScreenPosition;
   juce::Point<int> mDesiredInitialPosition;
   SpaceMouseMessageWindow mSpaceMouseReader;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }

// This function is called when opening the app with a bsk file.
void SetStartupSaveStateFile(const juce::String& bskFilePath, Component* component)
{
   auto* mainComponent = dynamic_cast<MainContentComponent*>(component);
   if(mainComponent == nullptr)
      ofLog() << "Non main component sent to SetStartupSaveStateFile";
   else
      mainComponent->SetStartupSaveStateFile(bskFilePath);
}


#endif  // MAINCOMPONENT_H_INCLUDED
