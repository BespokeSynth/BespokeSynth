#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#ifdef BESPOKE_WINDOWS
#include <GL/glew.h>
#endif

#include "../JuceLibraryCode/JuceHeader.h"
#include "nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg/nanovg_gl.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"

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
   {
      openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
      
      setSize(1280, 1024);
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
      if (!hasKeyboardFocus(true) && isVisible())
         grabKeyboardFocus();
      
      mSynth.Poll();
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
      glewInit();
#endif
      
      mVG = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
      
      if (mVG == NULL)
         printf("Could not init nanovg.\n");
      
      mSynth.LoadResources(mVG);
      
      /*for (auto deviceType : mGlobalManagers.mDeviceManager.getAvailableDeviceTypes())
      {
         ofLog() << "inputs:";
         for (auto input : deviceType->getDeviceNames(true))
            ofLog() << input.toStdString();
         ofLog() << "outputs:";
         for (auto output : deviceType->getDeviceNames(false))
            ofLog() << output.toStdString();
      }*/
      
      mSynth.Setup(&mGlobalManagers, this);
      
      String preferredDefaultDeviceName = "";
      AudioDeviceManager::AudioDeviceSetup preferredSetupOptions;
      preferredSetupOptions.sampleRate = gSampleRate;
      preferredSetupOptions.bufferSize = gBufferSize;

#ifdef JUCE_WINDOWS
      HRESULT hr;
      hr = CoInitializeEx(0, COINIT_MULTITHREADED);
#endif

      String audioError = mGlobalManagers.mDeviceManager.initialise(MAX_INPUT_CHANNELS,
                                                                    MAX_OUTPUT_CHANNELS,
                                                                    nullptr,
                                                                    true,
                                                                    preferredDefaultDeviceName,
                                                                    &preferredSetupOptions);
      jassert (audioError.isEmpty());
      mGlobalManagers.mDeviceManager.addAudioCallback(this);
      
      startTimerHz(60);
   }
   
   void shutdown() override
   {
      nvgDeleteGL3(mVG);
   }
   
   void render() override
   {
      mSynth.LockRender(true);
      
      Point<int> mouse = Desktop::getMousePosition();
      mouse -= getScreenPosition();
      mSynth.MouseMoved(mouse.x, mouse.y);
      
      float width = getWidth();
      float height = getHeight();
      float pixelRatio = 2;
#ifdef JUCE_WINDOWS
      pixelRatio = 1;
#endif
      
      glViewport(0, 0, width*pixelRatio, height*pixelRatio);
      glClearColor(0,0,0,0);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
      
      nvgBeginFrame(mVG, width, height, pixelRatio);
      
      nvgLineCap(mVG, NVG_ROUND);
      nvgLineJoin(mVG, NVG_ROUND);
      static float sSpacing = -.3f;
      nvgTextLetterSpacing(mVG, sSpacing);
      
      mSynth.Draw(mVG);
      
      nvgEndFrame(mVG);
      
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
      mSynth.MousePressed(e.getMouseDownX(), e.getMouseDownY(), e.mods.isRightButtonDown() ? 2 : 1);
   }
   
   void mouseUp(const MouseEvent& e) override
   {
      mSynth.MouseReleased(e.getPosition().x, e.getPosition().y, e.mods.isRightButtonDown() ? 2 : 1);
   }
   
   void mouseDrag(const MouseEvent& e) override
   {
      mSynth.MouseDragged(e.getPosition().x, e.getPosition().y, e.mods.isRightButtonDown() ? 2 : 1);
   }
   
   void mouseMove(const MouseEvent& e) override
   {
      //Don't do mouse move in here, it really slows UI responsiveness in some scenarios. We do it when we render instead.
      //mSynth.MouseMoved(e.getPosition().x, e.getPosition().y);
   }
   
   void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override
   {
      if (!wheel.isInertial)
         mSynth.MouseScrolled(wheel.deltaX * 30, wheel.deltaY * 30);
   }
   
   bool keyPressed(const KeyPress& key) override
   {
      int keyCode = key.getKeyCode();
      if (isalpha(keyCode) && !key.getModifiers().isShiftDown())
         keyCode -= 'A' - 'a';
      if (key.isCurrentlyDown())
         mSynth.KeyPressed(keyCode);
      else
         mSynth.KeyReleased(keyCode);
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
   
   GlobalManagers mGlobalManagers;
   
   ModularSynth mSynth;
   
   NVGcontext* mVG;
   int64 mLastFpsUpdateTime;
   int mFrameCountAccum;
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
