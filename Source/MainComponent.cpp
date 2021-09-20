#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED


#ifdef BESPOKE_WINDOWS
// juce::gl supercedes this
//#include <GL/glew.h>
#endif

#include <JuceHeader.h>
using namespace juce::gl;


#include "nanovg/nanovg.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "Push2Control.h"  //TODO(Ryan) remove
#include "SpaceMouseControl.h"

#ifdef JUCE_WINDOWS
#include <Windows.h>
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
#if BESPOKE_WINDOWS
   , mSpaceMouseReader(&mSynth)
#endif
   {
      ofLog() << "bespoke synth " << JUCEApplication::getInstance()->getApplicationVersion();
      
      openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
      openGLContext.setContinuousRepainting(false);

#ifndef BESPOKE_WINDOWS //windows crash handler is set up in ModularSynth() constructor
      SystemStats::setApplicationCrashHandler(ModularSynth::CrashHandler);
#endif
      
      int screenWidth, screenHeight;
      {
         const MessageManagerLock lock;
         mPixelRatio = Desktop::getInstance().getDisplays().getMainDisplay().scale;
         TheSynth->SetPixelRatio(mPixelRatio);
         auto bounds = Desktop::getInstance().getDisplays().getTotalBounds(true);
         screenWidth = bounds.getWidth();
         screenHeight = bounds.getHeight();
         ofLog() << "pixel ratio: " << mPixelRatio << " screen width: " << screenWidth << " screen height: " << screenHeight;
      }
      
      int width = 1700;
      int height = 1100;
      mDesiredInitialPosition.setXY(INT_MAX, INT_MAX);
      ofxJSONElement userPrefs;
      bool loaded = userPrefs.open(ModularSynth::GetUserPrefsPath(false));
      if (loaded)
      {
         width = userPrefs["width"].asInt();
         height = userPrefs["height"].asInt();
         if (!userPrefs["position_x"].isNull())
            mDesiredInitialPosition.setXY(userPrefs["position_x"].asInt(), userPrefs["position_y"].asInt());
      }
      
      if (mDesiredInitialPosition.x == INT_MAX)
      {
         if (width + getTopLevelComponent()->getPosition().x > screenWidth)
            width = screenWidth - getTopLevelComponent()->getPosition().x;
         if (height + getTopLevelComponent()->getPosition().y + 20 > screenHeight)
            height = screenHeight - getTopLevelComponent()->getPosition().y - 20;

         setSize(width, height);
      }
      
      setSize(width, height);
      setWantsKeyboardFocus(true);
      Desktop::setScreenSaverEnabled(false);
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
      
#if DEBUG || BESPOKE_LINUX
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
         mPixelRatio = Desktop::getInstance().getDisplays().findDisplayForRect(getScreenBounds()).scale; //adjust pixel ratio based on which screen has the majority of the window
         TheSynth->SetPixelRatio(mPixelRatio);
      }
      
      mScreenPosition = getScreenPosition();

#if BESPOKE_WINDOWS
      mSpaceMouseReader.Poll();
#endif
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

      mGlobalManagers.mDeviceManager.getAvailableDeviceTypes();   //scans for device types ("Windows Audio", "DirectSound", etc)

      ofxJSONElement userPrefs;
      const string kAutoDevice = "auto";
      const string kNoneDevice = "none";
      string outputDevice = kAutoDevice;
      string inputDevice = kNoneDevice;
      int sampleRate = 48000;
      int bufferSize = 256;
      bool loaded = userPrefs.open(ModularSynth::GetUserPrefsPath(false));
      if (loaded)
      {
         if (!userPrefs["samplerate"].isNull())
            sampleRate = userPrefs["samplerate"].asInt();
         if (!userPrefs["buffersize"].isNull())
            bufferSize = userPrefs["buffersize"].asInt();
         if (!userPrefs["devicetype"].isNull() && userPrefs["devicetype"].asString() != "auto")
            mGlobalManagers.mDeviceManager.setCurrentAudioDeviceType(userPrefs["devicetype"].asString(), true);
         if (!userPrefs["audio_output_device"].isNull())
            outputDevice = userPrefs["audio_output_device"].asString();
         if (!userPrefs["audio_input_device"].isNull())
            inputDevice = userPrefs["audio_input_device"].asString();
      }

      SetGlobalSampleRateAndBufferSize(sampleRate, bufferSize);
      
      mSynth.Setup(&mGlobalManagers, this, &openGLContext);

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
      
      startTimerHz(60);
   }
   
   void shutdown() override
   {
      nvgDeleteGLES2(mVG);
      nvgDeleteGLES2(mFontBoundsVG);
   }

   void setInitialState(const String& statePath) {
       mSynth.SetInitialState(statePath.toStdString());
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
      
      ofVec3f bgColor(.09f,.09f,.09f);
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
   void mouseDown(const MouseEvent& e) override
   {
      mSynth.MousePressed(e.getMouseDownX(), e.getMouseDownY(), e.mods.isPopupMenu() ? 2 : 1);
   }
   
   void mouseUp(const MouseEvent& e) override
   {
      mSynth.MouseReleased(e.getPosition().x, e.getPosition().y, e.mods.isPopupMenu() ? 2 : 1);
   }
   
   void mouseDrag(const MouseEvent& e) override
   {
      mSynth.MouseDragged(e.getPosition().x, e.getPosition().y, e.mods.isPopupMenu() ? 2 : 1);
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
      mSynth.MouseMagnify(e.getPosition().x, e.getPosition().y, scaleFactor);
   }
   
   bool keyPressed(const KeyPress& key) override
   {
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
      return true;
   }
   
   bool isInterestedInFileDrag(const StringArray& files) override
   {
      //TODO_PORT(Ryan)
      return true;
   }
   
   void filesDropped(const StringArray& files, int x, int y) override
   {
      vector<string> strFiles;
      for (auto file : files)
         strFiles.push_back(file.toStdString());
      mSynth.FilesDropped(strFiles, x, y);
   }
   
   string GetAudioDevices()
   {
      string ret;
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

   GlobalManagers mGlobalManagers;
   
   ModularSynth mSynth;
   
   NVGcontext* mVG;
   NVGcontext* mFontBoundsVG;
   int64 mLastFpsUpdateTime;
   int mFrameCountAccum;
   list<int> mPressedKeys;
   double mPixelRatio;
   juce::Point<int> mScreenPosition;
   juce::Point<int> mDesiredInitialPosition;
#if BESPOKE_WINDOWS
   SpaceMouseMessageWindow mSpaceMouseReader;
#endif
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }

// This function is called when opening the app with a state file.
void setInitialState(const String& statePath, Component* component) {
    auto* mainComponent = dynamic_cast<MainContentComponent*>(component);
    if(mainComponent == nullptr){
        ofLog() << "Non main component sent to setInitialState";
    }
    mainComponent->setInitialState(statePath);
}


#endif  // MAINCOMPONENT_H_INCLUDED
