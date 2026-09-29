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
#include "AudioDeviceSettings.h"
#include "AudioIOBridge.h"
#include "SynthGlobals.h"
#include "Push2Control.h" //TODO(Ryan) remove
#include "SpaceMouseControl.h"
#include "UserPrefs.h"
#include <atomic>
#include <cmath>
#include <limits>

#ifdef JUCE_WINDOWS
#include <windows.h>
#endif

//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
class MainContentComponent : public OpenGLAppComponent,
                             public AudioIODeviceCallback,
                             public FileDragAndDropTarget,
                             private Timer
{
public:
   static constexpr const char* kAutoDevice = "auto";
   static constexpr const char* kNoneDevice = "none";

   class SeparateInputCallback : public AudioIODeviceCallback
   {
   public:
      explicit SeparateInputCallback(MainContentComponent& owner)
      : mOwner(owner)
      {}
      void audioDeviceAboutToStart(AudioIODevice* device) override
      {
         if (!mOwner.mAudioSwitchInProgress && mOwner.mSeparateInputActive && device != nullptr &&
             (device->getCurrentSampleRate() != mOwner.mConfiguredInputRate ||
              device->getCurrentBufferSizeSamples() != mOwner.mConfiguredInputBlockSize))
         {
            mOwner.mAudioFormatPending = true;
            mOwner.mAudioDeviceStopped = true;
         }
      }
      void audioDeviceStopped() override
      {
         if (!mOwner.mAudioSwitchInProgress)
         {
            mOwner.mAudioFormatPending = true;
            mOwner.mAudioDeviceStopped = true;
         }
      }
      void audioDeviceIOCallbackWithContext(const float* const* input, int inputChannels, float* const* output,
                                            int outputChannels, int frames, const AudioIODeviceCallbackContext&) override
      {
         if (!mOwner.mAudioSwitchInProgress && !mOwner.mAudioFormatPending &&
             inputChannels == mOwner.mSynth.GetNumInputChannels() && mOwner.mAudioIOBridge)
            mOwner.mAudioIOBridge->PushSeparateInput(input, inputChannels, frames);
         else if (!mOwner.mAudioSwitchInProgress && !mOwner.mAudioFormatPending &&
                  inputChannels != mOwner.mSynth.GetNumInputChannels())
         {
            mOwner.mAudioFormatPending = true;
            mOwner.mAudioDeviceStopped = true;
         }
         for (int ch = 0; ch < outputChannels; ++ch)
            if (output != nullptr && output[ch] != nullptr)
               juce::FloatVectorOperations::clear(output[ch], frames);
      }

   private:
      MainContentComponent& mOwner;
   };

   //==============================================================================
   MainContentComponent()
   : mLastFpsUpdateTime(0)
   , mFrameCountAccum(0)
   , mPixelRatio(1)
   , mSpaceMouseReader(mSynth)
   , mSeparateInputCallback(*this)
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
#if BESPOKE_LINUX //turning this on improves linux framerate, but seems to expose thread safety issues on windows/mac. see git PRs #349 and #396
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
      mGlobalManagers.mDeviceManager.getAvailableDeviceTypes(); //scans for device types ("Windows Audio", "DirectSound", etc)
      if (auto* type = mGlobalManagers.mDeviceManager.getCurrentDeviceTypeObject())
         mDefaultAudioDeviceType = type->getTypeName();
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

#if DEBUG
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

      if (mAudioDeviceStopped.exchange(false) && !mAudioSwitchInProgress)
      {
         ofLog() << "audio device stopped or changed format; reconnecting";
         mAudioDeviceConnectionState = AudioDeviceConnectionState::Disconnected;
      }

      if (mAudioDeviceConnectionState == AudioDeviceConnectionState::Disconnected &&
          Time::getMillisecondCounter() - mLastAudioReconnectAttempt >= 1000)
      {
         mLastAudioReconnectAttempt = Time::getMillisecondCounter();
         ApplyAudioDeviceSelection(mActiveAudioSelection);
      }
   }

   //==============================================================================
   void audioDeviceAboutToStart(AudioIODevice* device) override
   {
      // This function will be called when the audio device is started, or when
      // its settings (i.e. sample rate, block size, etc) are changed.

      // You can use this function to initialise any resources you might need,
      // but be careful - it will be called on the audio thread, not the GUI thread.

      if (!mAudioSwitchInProgress && device != nullptr && mConfiguredOutputRate > 0 &&
          (device->getCurrentSampleRate() != mConfiguredOutputRate ||
           device->getCurrentBufferSizeSamples() != mConfiguredOutputBlockSize))
      {
         mAudioFormatPending = true;
         mAudioDeviceStopped = true;
      }
   }

   void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                         int numInputChannels,
                                         float* const* outputChannelData,
                                         int numOutputChannels,
                                         int numSamples,
                                         const AudioIODeviceCallbackContext& context) override
   {
      ignoreUnused(context);
      const auto clearOutput = [&]()
      {
         for (int ch = 0; ch < numOutputChannels; ++ch)
            if (outputChannelData != nullptr && outputChannelData[ch] != nullptr)
               juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
      };
      if (mAudioSwitchInProgress || mAudioFormatPending)
      {
         clearOutput();
         return;
      }
      const int expectedInputChannels = mSeparateInputActive ? 0 : mSynth.GetNumInputChannels();
      const bool unexpectedChannels = numInputChannels != expectedInputChannels ||
                                      numOutputChannels != mSynth.GetNumOutputChannels();
      const bool unexpectedDirectBlock = mAudioIOBridge == nullptr && numSamples != mActiveBufferSize;
      if (unexpectedChannels || unexpectedDirectBlock)
      {
         mAudioFormatPending = true;
         mAudioDeviceStopped = true;
         clearOutput();
         return;
      }
      if (mAudioIOBridge)
         mAudioIOBridge->Process(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
      else
      {
         mSynth.AudioIn(inputChannelData, numSamples, numInputChannels);
         mSynth.AudioOut(outputChannelData, numSamples, numOutputChannels);
      }
   }

   void audioDeviceStopped() override
   {
      if (!mAudioSwitchInProgress)
      {
         mAudioFormatPending = true;
         mAudioDeviceStopped = true;
      }
   }

   void shutdownAudio()
   {
      mGlobalManagers.mDeviceManager.removeAudioCallback(this);
      mGlobalManagers.mDeviceManager.closeAudioDevice();
      mInputDeviceManager.removeAudioCallback(&mSeparateInputCallback);
      mInputDeviceManager.closeAudioDevice();
      mAudioIOBridge.reset();
   }

   void initialise() override
   {
#ifdef JUCE_WINDOWS
      // glewInit();
#endif

      for (int i = 0; i < (int)NanoVGRenderContext::Num; ++i)
      {
         gNanoVGRenderContexts[i] = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
         if (gNanoVGRenderContexts[i] == nullptr)
            printf("Could not init nanovg.\n");
      }
      gNanoVG = gNanoVGRenderContexts[(int)NanoVGRenderContext::Main];

      mSynth.LoadResources();
      Push2Control::CreateStaticFramebuffer();

      /*for (auto deviceType : mGlobalManagers.mDeviceManager.getAvailableDeviceTypes())
      {
         ofLog() << "inputs:";
         for (auto input : deviceType->getDeviceNames(true))
            ofLog() << input.toStdString();
         ofLog() << "outputs:";
         for (auto output : deviceType->getDeviceNames(false))
            ofLog() << output.toStdString();
      }*/

      mActiveAudioSelection = { UserPrefs.devicetype.Get(), UserPrefs.audio_input_device.Get(), UserPrefs.audio_output_device.Get() };
      mActiveSampleRate = UserPrefs.samplerate.Get();
      mActiveBufferSize = UserPrefs.buffersize.Get();
      mActiveOversampling = UserPrefs.oversampling.Get();
      mActiveMaxInputChannels = UserPrefs.max_input_channels.Get();
      mActiveMaxOutputChannels = UserPrefs.max_output_channels.Get();

      if (UserPrefs.devicetype.Get() != kAutoDevice)
         mGlobalManagers.mDeviceManager.setCurrentAudioDeviceType(UserPrefs.devicetype.Get(), true);

      SetGlobalSampleRateAndBufferSize(UserPrefs.samplerate.Get(), UserPrefs.buffersize.Get());

      mSynth.Setup(&mGlobalManagers.mDeviceManager, &mGlobalManagers.mAudioFormatManager, this, &openGLContext);
      mAudioSwitchInProgress = true;

#ifdef JUCE_WINDOWS
      CoInitializeEx(0, COINIT_MULTITHREADED);
#endif

      std::string inputDevice = GetInputDeviceName(mActiveAudioSelection);
      std::string outputDevice = mActiveAudioSelection.output;
      DeviceFormatPlan plan;
      String audioError = ChooseDeviceSettings(mActiveAudioSelection, plan);
      if (audioError.isEmpty())
         audioError = InitializeAudioDevice(mActiveAudioSelection, plan);

      if (audioError.isEmpty())
      {
         auto loadedSetup = mGlobalManagers.mDeviceManager.getAudioDeviceSetup();
         auto loadedInputSetup = plan.separateInput ? mInputDeviceManager.getAudioDeviceSetup() : loadedSetup;
         if (outputDevice != kAutoDevice && outputDevice != kNoneDevice &&
             loadedSetup.outputDeviceName.toStdString() != outputDevice)
         {
            mSynth.SetFatalError("error setting output device to '" + outputDevice + "', fix this in userprefs.json (use \"auto\" for default device)" +
                                 "\n\n\nvalid devices:\n" + GetAudioDevices());
         }
         else if (inputDevice != kAutoDevice && inputDevice != kNoneDevice &&
                  loadedInputSetup.inputDeviceName.toStdString() != inputDevice)
         {
            mSynth.SetFatalError("error setting input device to '" + inputDevice + "', fix this in userprefs.json (use \"auto\" for default device, or \"none\" for no device)" +
                                 "\n\n\nvalid devices:\n" + GetAudioDevices());
         }
         else if (plan.separateInput && (mInputDeviceManager.getCurrentAudioDeviceType().toStdString() != GetAudioDeviceTypeName(mActiveAudioSelection) ||
                                         mInputDeviceManager.getCurrentAudioDevice() == nullptr ||
                                         !mInputDeviceManager.getCurrentAudioDevice()->isOpen() ||
                                         loadedInputSetup.inputChannels.countNumberOfSetBits() == 0 ||
                                         loadedSetup.inputChannels.countNumberOfSetBits() != 0 ||
                                         loadedInputSetup.outputChannels.countNumberOfSetBits() != 0))
         {
            mSynth.SetFatalError("the separate audio input did not open with usable channels");
         }
         else if (mGlobalManagers.mDeviceManager.getCurrentAudioDevice() != nullptr &&
                  (loadedSetup.sampleRate <= 0 || loadedSetup.bufferSize <= 0))
         {
            mSynth.SetFatalError("audio device opened without a valid sample rate or buffer size");
         }
         else
         {
            ofLog() << "output: " << loadedSetup.outputDeviceName << "   input: " << loadedSetup.inputDeviceName;

            int numInputChannels = loadedInputSetup.inputChannels.countNumberOfSetBits();
            int numOutputChannels = loadedSetup.outputChannels.countNumberOfSetBits();

            mSynth.InitIOBuffers(numInputChannels, numOutputChannels);
            mAudioIOBridge = CreateAudioIOBridge(loadedSetup, plan.separateInput);
            mSeparateInputActive = plan.separateInput;
            mConfiguredOutputRate = loadedSetup.sampleRate;
            mConfiguredOutputBlockSize = loadedSetup.bufferSize;
            mConfiguredInputRate = loadedInputSetup.sampleRate;
            mConfiguredInputBlockSize = loadedInputSetup.bufferSize;
            ofLog() << "audio format: device " << loadedSetup.sampleRate << " Hz / " << loadedSetup.bufferSize
                    << " samples; input " << loadedInputSetup.sampleRate << " Hz / " << loadedInputSetup.bufferSize
                    << " samples; engine " << mActiveSampleRate << " Hz / " << mActiveBufferSize << " samples";
         }
      }
      else
      {
         if (audioError.startsWith("No such device"))
            audioError += "\n\nfix this in userprefs.json (you can use \"auto\" for the default device)";
         else
            audioError += juce::String("\n\nattempted to set output to: " + outputDevice + " and input to: " + inputDevice + "\n\ninitialization errors could potentially be fixed by changing buffer size, sample rate, or input/output devices in userprefs.json\nto use no input device, specify \"none\" for \"audio_input_device\"");
         mSynth.SetFatalError("error initializing audio device: " + audioError.toStdString() +
                              "\n\n\nvalid devices:\n" + GetAudioDevices());
      }

      mSynth.ResetLayout();

      if (!mSynth.HasFatalError())
      {
         mGlobalManagers.mDeviceManager.addAudioCallback(this);
         mAudioCallbackRegistered = true;
         if (mSeparateInputActive)
         {
            mInputDeviceManager.addAudioCallback(&mSeparateInputCallback);
            mInputCallbackRegistered = true;
         }

         mAudioDeviceConnectionState = AudioDeviceConnectionState::Connected;
      }
      mAudioDeviceStopped = false;
      mAudioSwitchInProgress = false;

      for (int i = 0; i < JUCEApplication::getCommandLineParameterArray().size(); ++i)
      {
         juce::String argument = JUCEApplication::getCommandLineParameterArray()[i];
         if (argument.endsWith(".bsk") || argument.endsWith(".bskt"))
         {
            mSynth.SetStartupSaveStateFile(argument.toStdString());
            TitleBar::sShowInitialHelpOverlay = false; //don't show initial help popup, a user who uses the command line arguments likely doesn't need it
            break;
         }
      }

      UserPrefs.LastTargetFramerate = UserPrefs.target_framerate.Get();
      startTimerHz(UserPrefs.target_framerate.Get());
   }

   std::string GetAudioDeviceTypeName(const AudioDeviceSelection& selection) const
   {
      return selection.type == kAutoDevice ? mDefaultAudioDeviceType.toStdString() : selection.type;
   }

   std::string GetInputDeviceName(const AudioDeviceSelection& selection)
   {
      std::string inputDevice = selection.input;
      for (auto* type : mGlobalManagers.mDeviceManager.getAvailableDeviceTypes())
         if (type->getTypeName().toStdString() == GetAudioDeviceTypeName(selection))
            if (!type->hasSeparateInputsAndOutputs())
               inputDevice = selection.output; //asio must have identical input and output
      return inputDevice;
   }

   struct DeviceFormatPlan
   {
      int outputRate{ 0 };
      int outputBlockSize{ 0 };
      int inputRate{ 0 };
      int inputBlockSize{ 0 };
      bool separateInput{ false };
   };

   String ChooseDeviceSettings(const AudioDeviceSelection& selection, DeviceFormatPlan& plan)
   {
      plan = { mActiveSampleRate, mActiveBufferSize, mActiveSampleRate, mActiveBufferSize, false };

      auto requestedType = GetAudioDeviceTypeName(selection);
      AudioIODeviceType* type = nullptr;
      for (auto* availableType : mGlobalManagers.mDeviceManager.getAvailableDeviceTypes())
         if (availableType->getTypeName().toStdString() == requestedType)
            type = availableType;
      if (type == nullptr)
         return "audio device type is not available: " + requestedType;

      type->scanForDevices();
      auto inputName = GetInputDeviceName(selection);
      auto outputName = selection.output;
      if (outputName != kAutoDevice && outputName != kNoneDevice && !type->getDeviceNames(false).contains(outputName))
         return "audio output device is not available: " + outputName;
      if (inputName != kAutoDevice && inputName != kNoneDevice && !type->getDeviceNames(true).contains(inputName))
         return "audio input device is not available: " + inputName;

      auto resolveName = [type](const std::string& requested, bool isInput) -> String
      {
         if (requested == kNoneDevice)
            return {};
         if (requested != kAutoDevice)
            return requested;
         auto names = type->getDeviceNames(isInput);
         int index = type->getDefaultDeviceIndex(isInput);
         return index >= 0 && index < names.size() ? names[index] : String();
      };
      String resolvedOutput = resolveName(outputName, false);
      String resolvedInput = type->hasSeparateInputsAndOutputs() ? resolveName(inputName, true) : resolvedOutput;
      if (inputName == kNoneDevice && outputName == kNoneDevice)
         return {};
      if (resolvedOutput.isEmpty() && resolvedInput.isEmpty())
         return "no audio input or output device is available";

      std::unique_ptr<AudioIODevice> candidate(type->createDevice(resolvedOutput, resolvedInput));
      if (candidate == nullptr)
         return "could not create the selected audio device";
      auto chooseRate = [this](const juce::Array<double>& rates) -> int
      {
         int selected = 0;
         double bestDistance = std::numeric_limits<double>::max();
         for (double rate : rates)
         {
            double distance = std::abs(rate - mActiveSampleRate);
            if (rate > 0 && distance < bestDistance)
            {
               selected = (int)rate;
               bestDistance = distance;
            }
         }
         return selected;
      };
      auto chooseBlock = [this](const juce::Array<int>& sizes) -> int
      {
         int selected = 0;
         int bestDistance = INT_MAX;
         for (int size : sizes)
         {
            int distance = std::abs(size - mActiveBufferSize);
            if (size > 0 && distance < bestDistance)
            {
               selected = size;
               bestDistance = distance;
            }
         }
         return selected;
      };

      auto rates = candidate->getAvailableSampleRates();
      auto blockSizes = candidate->getAvailableBufferSizes();
      if (!rates.isEmpty() && !blockSizes.isEmpty())
      {
         plan.outputRate = plan.inputRate = chooseRate(rates);
         plan.outputBlockSize = plan.inputBlockSize = chooseBlock(blockSizes);
         return {};
      }

      if (!type->hasSeparateInputsAndOutputs() || resolvedInput.isEmpty() || resolvedOutput.isEmpty() || resolvedInput == resolvedOutput)
         return "the selected input and output devices have no common hardware sample rate or buffer size";

      // JUCE's combined CoreAudio device requires one shared format. Open two
      // streams when distinct devices have no common format.
      std::unique_ptr<AudioIODevice> outputOnly(type->createDevice(resolvedOutput, {}));
      std::unique_ptr<AudioIODevice> inputOnly(type->createDevice({}, resolvedInput));
      if (outputOnly == nullptr || inputOnly == nullptr)
         return "could not create separate audio input and output devices";
      plan.outputRate = chooseRate(outputOnly->getAvailableSampleRates());
      plan.outputBlockSize = chooseBlock(outputOnly->getAvailableBufferSizes());
      plan.inputRate = chooseRate(inputOnly->getAvailableSampleRates());
      plan.inputBlockSize = chooseBlock(inputOnly->getAvailableBufferSizes());
      if (plan.outputRate <= 0 || plan.outputBlockSize <= 0 || plan.inputRate <= 0 || plan.inputBlockSize <= 0)
         return "one of the selected devices has no usable audio format";
      plan.separateInput = true;
      return {};
   }

   AudioDeviceManager::AudioDeviceSetup GetPreferredSetupOptions(const AudioDeviceSelection& selection, int deviceRate, int deviceBlockSize)
   {
      std::string inputDevice = GetInputDeviceName(selection);
      std::string outputDevice = selection.output;
      AudioDeviceManager::AudioDeviceSetup preferredSetupOptions;
      preferredSetupOptions.sampleRate = deviceRate;
      preferredSetupOptions.bufferSize = deviceBlockSize;
      if (outputDevice != kAutoDevice && outputDevice != kNoneDevice)
         preferredSetupOptions.outputDeviceName = outputDevice;
      if (inputDevice != kAutoDevice && inputDevice != kNoneDevice)
         preferredSetupOptions.inputDeviceName = inputDevice;
      return preferredSetupOptions;
   }

   std::unique_ptr<AudioIOBridge> CreateAudioIOBridge(const AudioDeviceManager::AudioDeviceSetup& setup, bool separateInput)
   {
      if (mGlobalManagers.mDeviceManager.getCurrentAudioDevice() == nullptr ||
          (!separateInput && setup.sampleRate == mActiveSampleRate && setup.bufferSize == mActiveBufferSize))
         return {};
      auto inputSetup = separateInput ? mInputDeviceManager.getAudioDeviceSetup() : setup;
      return std::make_unique<AudioIOBridge>(mSynth, mActiveSampleRate, mActiveBufferSize, (int)setup.sampleRate, setup.bufferSize,
                                             inputSetup.inputChannels.countNumberOfSetBits(), setup.outputChannels.countNumberOfSetBits(),
                                             separateInput ? (int)inputSetup.sampleRate : 0, separateInput ? inputSetup.bufferSize : 0);
   }

   String InitializeAudioDevice(const AudioDeviceSelection& selection, const DeviceFormatPlan& plan)
   {
      AudioDeviceSelection outputSelection = selection;
      if (plan.separateInput)
         outputSelection.input = kNoneDevice;
      std::string inputDevice = GetInputDeviceName(outputSelection);
      std::string outputDevice = selection.output;
      AudioDeviceManager::AudioDeviceSetup preferredSetupOptions = GetPreferredSetupOptions(outputSelection, plan.outputRate, plan.outputBlockSize);

      int inputChannels = mActiveMaxInputChannels;
      int outputChannels = mActiveMaxOutputChannels;

      if (inputDevice == kNoneDevice)
         inputChannels = 0;
      if (outputDevice == kNoneDevice)
         outputChannels = 0;

      mGlobalManagers.mDeviceManager.setCurrentAudioDeviceType(GetAudioDeviceTypeName(selection), false);

      String audioError = mGlobalManagers.mDeviceManager.initialise(inputChannels,
                                                                    outputChannels,
                                                                    nullptr,
                                                                    false,
                                                                    "",
                                                                    &preferredSetupOptions);

      if (audioError.isNotEmpty() || !plan.separateInput)
         return audioError;

      mInputDeviceManager.getAvailableDeviceTypes();
      mInputDeviceManager.setCurrentAudioDeviceType(GetAudioDeviceTypeName(selection), false);
      AudioDeviceManager::AudioDeviceSetup inputSetup;
      inputSetup.sampleRate = plan.inputRate;
      inputSetup.bufferSize = plan.inputBlockSize;
      if (selection.input != kAutoDevice)
         inputSetup.inputDeviceName = selection.input;
      return mInputDeviceManager.initialise(mActiveMaxInputChannels, 0, nullptr, false, "", &inputSetup);
   }

   AudioDeviceSelection GetActiveAudioDeviceSelection() const { return mActiveAudioSelection; }

   AudioEngineSettings GetActiveAudioEngineSettings() const
   {
      return { mActiveSampleRate, mActiveBufferSize, mActiveOversampling, mActiveMaxInputChannels, mActiveMaxOutputChannels };
   }

   AudioHardwareSettings GetActiveAudioHardwareSettings() const
   {
      return { mSynth.GetNumOutputChannels() > 0 ? (int)mConfiguredOutputRate.load() : 0,
               mSynth.GetNumOutputChannels() > 0 ? mConfiguredOutputBlockSize.load() : 0,
               mSynth.GetNumInputChannels() > 0 ? (int)mConfiguredInputRate.load() : 0,
               mSynth.GetNumInputChannels() > 0 ? mConfiguredInputBlockSize.load() : 0 };
   }

   AudioDeviceApplyResult ApplyAudioDeviceSelection(const AudioDeviceSelection& selection)
   {
      jassert(MessageManager::getInstance()->isThisTheMessageThread());

      auto& manager = mGlobalManagers.mDeviceManager;
      auto requestedType = GetAudioDeviceTypeName(selection);
      auto inputName = GetInputDeviceName(selection);
      auto outputName = selection.output;
      DeviceFormatPlan plan;
      String error = ChooseDeviceSettings(selection, plan);
      if (error.isNotEmpty())
         return { false, error.toStdString() };

      auto previousType = manager.getCurrentAudioDeviceType();
      auto previousSetup = manager.getAudioDeviceSetup();
      bool previousDeviceWasOpen = manager.getCurrentAudioDevice() != nullptr && manager.getCurrentAudioDevice()->isOpen();
      const bool previousSeparateInput = mSeparateInputActive;
      auto previousInputType = mInputDeviceManager.getCurrentAudioDeviceType();
      auto previousInputSetup = mInputDeviceManager.getAudioDeviceSetup();

      mAudioFormatPending = true;
      mAudioSwitchInProgress = true;
      if (mAudioCallbackRegistered)
      {
         manager.removeAudioCallback(this);
         mAudioCallbackRegistered = false;
      }
      if (mInputCallbackRegistered)
      {
         mInputDeviceManager.removeAudioCallback(&mSeparateInputCallback);
         mInputCallbackRegistered = false;
      }
      manager.closeAudioDevice();
      mInputDeviceManager.closeAudioDevice();

      error = InitializeAudioDevice(selection, plan);
      auto openedSetup = manager.getAudioDeviceSetup();
      auto openedInputSetup = plan.separateInput ? mInputDeviceManager.getAudioDeviceSetup() : openedSetup;
      auto* openedDevice = manager.getCurrentAudioDevice();
      auto* openedInputDevice = plan.separateInput ? mInputDeviceManager.getCurrentAudioDevice() : openedDevice;
      if (error.isEmpty())
      {
         if (manager.getCurrentAudioDeviceType().toStdString() != requestedType)
            error = "the requested audio device type was not opened";
         else if (outputName != kAutoDevice && outputName != kNoneDevice && openedSetup.outputDeviceName.toStdString() != outputName)
            error = "the requested audio output device was not opened";
         else if (inputName != kAutoDevice && inputName != kNoneDevice && openedInputSetup.inputDeviceName.toStdString() != inputName)
            error = "the requested audio input device was not opened";
         else if ((inputName != kNoneDevice || outputName != kNoneDevice) && openedDevice == nullptr)
            error = "no audio device was opened";
         else if (plan.separateInput && (openedSetup.inputChannels.countNumberOfSetBits() != 0 ||
                                         openedInputSetup.outputChannels.countNumberOfSetBits() != 0 ||
                                         openedInputSetup.inputChannels.countNumberOfSetBits() == 0))
            error = "the separate input or output opened with unexpected channels";
         else if (plan.separateInput && (mInputDeviceManager.getCurrentAudioDeviceType().toStdString() != requestedType ||
                                         openedInputDevice == nullptr || !openedInputDevice->isOpen() ||
                                         openedInputSetup.sampleRate <= 0 || openedInputSetup.bufferSize <= 0 ||
                                         openedInputDevice->getCurrentSampleRate() != openedInputSetup.sampleRate ||
                                         openedInputDevice->getCurrentBufferSizeSamples() != openedInputSetup.bufferSize ||
                                         openedInputSetup.inputChannels.countNumberOfSetBits() != openedInputDevice->getActiveInputChannels().countNumberOfSetBits()))
            error = "the separate audio input did not open with a valid format";
         else if (openedDevice != nullptr && (!openedDevice->isOpen() || openedSetup.sampleRate <= 0 || openedSetup.bufferSize <= 0 ||
                                              openedDevice->getCurrentSampleRate() != openedSetup.sampleRate ||
                                              openedDevice->getCurrentBufferSizeSamples() != openedSetup.bufferSize))
            error = "the selected device did not open with a valid audio format";
         else if (openedDevice != nullptr &&
                  (openedSetup.inputChannels.countNumberOfSetBits() != openedDevice->getActiveInputChannels().countNumberOfSetBits() ||
                   openedSetup.outputChannels.countNumberOfSetBits() != openedDevice->getActiveOutputChannels().countNumberOfSetBits()))
            error = "the selected device opened with an unexpected channel count";
      }

      if (error.isEmpty())
      {
         mSynth.InitIOBuffers(openedInputSetup.inputChannels.countNumberOfSetBits(), openedSetup.outputChannels.countNumberOfSetBits());
         mAudioIOBridge = CreateAudioIOBridge(openedSetup, plan.separateInput);
         ofLog() << "audio format: device " << openedSetup.sampleRate << " Hz / " << openedSetup.bufferSize
                 << " samples; input " << openedInputSetup.sampleRate << " Hz / " << openedInputSetup.bufferSize
                 << " samples; engine " << mActiveSampleRate << " Hz / " << mActiveBufferSize << " samples";
         if (openedDevice != nullptr)
         {
            manager.addAudioCallback(this);
            mAudioCallbackRegistered = true;
         }
         if (plan.separateInput)
         {
            mInputDeviceManager.addAudioCallback(&mSeparateInputCallback);
            mInputCallbackRegistered = true;
         }
         mSeparateInputActive = plan.separateInput;
         mConfiguredOutputRate = openedSetup.sampleRate;
         mConfiguredOutputBlockSize = openedSetup.bufferSize;
         mConfiguredInputRate = openedInputSetup.sampleRate;
         mConfiguredInputBlockSize = openedInputSetup.bufferSize;
         mActiveAudioSelection = selection;
         mAudioDeviceConnectionState = AudioDeviceConnectionState::Connected;
      }
      else
      {
         manager.closeAudioDevice();
         mInputDeviceManager.closeAudioDevice();
         if (previousDeviceWasOpen)
         {
            manager.setCurrentAudioDeviceType(previousType, false);
            int previousInputChannels = previousSeparateInput || GetInputDeviceName(mActiveAudioSelection) == kNoneDevice ? 0 : mActiveMaxInputChannels;
            int previousOutputChannels = mActiveAudioSelection.output == kNoneDevice ? 0 : mActiveMaxOutputChannels;
            String rollbackError = manager.initialise(previousInputChannels,
                                                      previousOutputChannels,
                                                      nullptr,
                                                      false,
                                                      "",
                                                      &previousSetup);
            if (rollbackError.isEmpty() && previousSeparateInput)
            {
               mInputDeviceManager.setCurrentAudioDeviceType(previousInputType, false);
               rollbackError = mInputDeviceManager.initialise(mActiveMaxInputChannels, 0, nullptr, false, "", &previousInputSetup);
            }
            auto restoredSetup = manager.getAudioDeviceSetup();
            auto restoredInputSetup = mInputDeviceManager.getAudioDeviceSetup();
            if (rollbackError.isEmpty() && manager.getCurrentAudioDevice() != nullptr && manager.getCurrentAudioDevice()->isOpen() &&
                manager.getCurrentAudioDeviceType() == previousType &&
                restoredSetup.inputDeviceName == previousSetup.inputDeviceName && restoredSetup.outputDeviceName == previousSetup.outputDeviceName &&
                restoredSetup.inputChannels == previousSetup.inputChannels && restoredSetup.outputChannels == previousSetup.outputChannels &&
                restoredSetup.sampleRate == previousSetup.sampleRate && restoredSetup.bufferSize == previousSetup.bufferSize &&
                (!previousSeparateInput || (mInputDeviceManager.getCurrentAudioDevice() != nullptr && mInputDeviceManager.getCurrentAudioDevice()->isOpen() &&
                                            restoredInputSetup.inputDeviceName == previousInputSetup.inputDeviceName &&
                                            restoredInputSetup.inputChannels == previousInputSetup.inputChannels &&
                                            restoredInputSetup.sampleRate == previousInputSetup.sampleRate &&
                                            restoredInputSetup.bufferSize == previousInputSetup.bufferSize)))
            {
               mSynth.InitIOBuffers(previousSeparateInput ? restoredInputSetup.inputChannels.countNumberOfSetBits() : restoredSetup.inputChannels.countNumberOfSetBits(),
                                    restoredSetup.outputChannels.countNumberOfSetBits());
               mAudioIOBridge = CreateAudioIOBridge(restoredSetup, previousSeparateInput);
               manager.addAudioCallback(this);
               mAudioCallbackRegistered = true;
               if (previousSeparateInput)
               {
                  mInputDeviceManager.addAudioCallback(&mSeparateInputCallback);
                  mInputCallbackRegistered = true;
               }
               mSeparateInputActive = previousSeparateInput;
               mConfiguredOutputRate = restoredSetup.sampleRate;
               mConfiguredOutputBlockSize = restoredSetup.bufferSize;
               mConfiguredInputRate = previousSeparateInput ? restoredInputSetup.sampleRate : restoredSetup.sampleRate;
               mConfiguredInputBlockSize = previousSeparateInput ? restoredInputSetup.bufferSize : restoredSetup.bufferSize;
               mAudioDeviceConnectionState = AudioDeviceConnectionState::Connected;
            }
            else
            {
               manager.closeAudioDevice();
               mInputDeviceManager.closeAudioDevice();
               mAudioIOBridge.reset();
               mSeparateInputActive = false;
               mConfiguredOutputRate = mConfiguredInputRate = 0;
               mConfiguredOutputBlockSize = mConfiguredInputBlockSize = 0;
               mAudioDeviceConnectionState = AudioDeviceConnectionState::Disconnected;
               if (rollbackError.isEmpty())
                  rollbackError = "the previous audio setup could not be restored exactly";
               error += "; the previous device could not be restored: " + rollbackError;
            }
         }
         else
         {
            manager.setCurrentAudioDeviceType(previousType, false);
            mAudioIOBridge.reset();
            mSeparateInputActive = false;
            mConfiguredOutputRate = mConfiguredInputRate = 0;
            mConfiguredOutputBlockSize = mConfiguredInputBlockSize = 0;
            mAudioDeviceConnectionState = GetInputDeviceName(mActiveAudioSelection) == kNoneDevice && mActiveAudioSelection.output == kNoneDevice
                                          ? AudioDeviceConnectionState::Connected
                                          : AudioDeviceConnectionState::Disconnected;
         }
      }

      mAudioDeviceStopped = false;
      mAudioFormatPending = mAudioDeviceConnectionState != AudioDeviceConnectionState::Connected;
      mAudioSwitchInProgress = false;
      return { error.isEmpty(), error.toStdString() };
   }

   void shutdown() override
   {
      for (int i = 0; i < (int)NanoVGRenderContext::Num; ++i)
         nvgDeleteGLES2(gNanoVGRenderContexts[i]);
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

      if (UserPrefs.LastTargetFramerate != UserPrefs.target_framerate.Get())
      {
         stopTimer();
         UserPrefs.LastTargetFramerate = UserPrefs.target_framerate.Get();
         startTimerHz(UserPrefs.target_framerate.Get());
      }

      juce::Point<int> mouse = Desktop::getMousePosition();
      mouse -= mScreenPosition;
      mSynth.MouseMoved(mouse.x, mouse.y);

      float width = getWidth();
      float height = getHeight();

      static float kMotionTrails = .4f;

      ofVec3f bgColor(ModularSynth::sBackgroundR, ModularSynth::sBackgroundG, ModularSynth::sBackgroundB);
      glViewport(0, 0, width * mPixelRatio, height * mPixelRatio);
      glClearColor(bgColor.x, bgColor.y, bgColor.z, 0);
      if (UserPrefs.motion_trails.Get() <= 0)
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      nvgBeginFrame(gNanoVG, width, height, mPixelRatio);

      if (UserPrefs.motion_trails.Get() > 0)
      {
         ofSetColor(bgColor.x * 255, bgColor.y * 255, bgColor.z * 255, (1 - (UserPrefs.motion_trails.Get() * kMotionTrails) * (ofGetFrameRate() / 60.0f)) * 255);
         ofFill();
         ofRect(0, 0, width, height);
      }

      static float sSpacing = -.3f;
      for (int i = 0; i < (int)NanoVGRenderContext::Num; ++i)
      {
         nvgLineCap(gNanoVGRenderContexts[i], NVG_ROUND);
         nvgLineJoin(gNanoVGRenderContexts[i], NVG_ROUND);
         nvgTextLetterSpacing(gNanoVGRenderContexts[i], sSpacing);
      }

      mSynth.Draw();

      nvgEndFrame(gNanoVG);

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
         mSynth.MouseScrolled(wheel.deltaX * scale, wheel.deltaY * scale * invert, wheel.isSmooth, wheel.isReversed, true);
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
      if (keyCode < 32 || key.getModifiers().isAltDown())
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
      for (auto& file : files)
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
         String typeName(types[i]->getTypeName()); // This will be things like "DirectSound", "CoreAudio", etc.
         types[i]->scanForDevices(); // This must be called before getting the list of devices

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
   juce::AudioDeviceManager mInputDeviceManager;

   ModularSynth mSynth;

   int64 mLastFpsUpdateTime;
   int mFrameCountAccum;
   std::list<int> mPressedKeys;
   double mPixelRatio;
   juce::Point<int> mScreenPosition;
   juce::Point<int> mDesiredInitialPosition;
   SpaceMouseMessageWindow mSpaceMouseReader;
   SeparateInputCallback mSeparateInputCallback;

   enum class AudioDeviceConnectionState
   {
      None,
      Connected,
      Disconnected
   };
   AudioDeviceConnectionState mAudioDeviceConnectionState{ AudioDeviceConnectionState::None };
   AudioDeviceSelection mActiveAudioSelection;
   String mDefaultAudioDeviceType;
   int mActiveSampleRate{ 0 };
   int mActiveBufferSize{ 0 };
   int mActiveOversampling{ 0 };
   int mActiveMaxInputChannels{ 0 };
   int mActiveMaxOutputChannels{ 0 };
   std::unique_ptr<AudioIOBridge> mAudioIOBridge;
   std::atomic<bool> mSeparateInputActive{ false };
   std::atomic<double> mConfiguredOutputRate{ 0 };
   std::atomic<double> mConfiguredInputRate{ 0 };
   std::atomic<int> mConfiguredOutputBlockSize{ 0 };
   std::atomic<int> mConfiguredInputBlockSize{ 0 };
   bool mAudioCallbackRegistered{ false };
   bool mInputCallbackRegistered{ false };
   std::atomic<bool> mAudioSwitchInProgress{ false };
   std::atomic<bool> mAudioDeviceStopped{ false };
   std::atomic<bool> mAudioFormatPending{ false };
   uint32 mLastAudioReconnectAttempt{ 0 };

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }

// This function is called when opening the app with a bsk file.
void SetStartupSaveStateFile(const juce::String& bskFilePath, Component* component)
{
   auto* mainComponent = dynamic_cast<MainContentComponent*>(component);
   if (mainComponent == nullptr)
      ofLog() << "Non main component sent to SetStartupSaveStateFile";
   else
      mainComponent->SetStartupSaveStateFile(bskFilePath);
}

AudioDeviceSelection GetActiveAudioDeviceSelection(juce::Component* component)
{
   if (auto* mainComponent = dynamic_cast<MainContentComponent*>(component))
      return mainComponent->GetActiveAudioDeviceSelection();
   return {};
}

std::string GetResolvedAudioDeviceTypeName(juce::Component* component, const AudioDeviceSelection& selection)
{
   if (auto* mainComponent = dynamic_cast<MainContentComponent*>(component))
      return mainComponent->GetAudioDeviceTypeName(selection);
   return selection.type;
}

AudioEngineSettings GetActiveAudioEngineSettings(juce::Component* component)
{
   if (auto* mainComponent = dynamic_cast<MainContentComponent*>(component))
      return mainComponent->GetActiveAudioEngineSettings();
   return {};
}

AudioHardwareSettings GetActiveAudioHardwareSettings(juce::Component* component)
{
   if (auto* mainComponent = dynamic_cast<MainContentComponent*>(component))
      return mainComponent->GetActiveAudioHardwareSettings();
   return {};
}

AudioDeviceApplyResult ApplyAudioDeviceSelection(juce::Component* component, const AudioDeviceSelection& selection)
{
   if (auto* mainComponent = dynamic_cast<MainContentComponent*>(component))
      return mainComponent->ApplyAudioDeviceSelection(selection);
   return { false, "could not find Bespoke's main component" };
}
