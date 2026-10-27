/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 BespokeSynth contributors

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

#pragma once

#include "juce_audio_devices/juce_audio_devices.h"
#include <jack/jack.h>
#include <cerrno>

class JackAudioApi
{
public:
   JackAudioApi()
   {
      if (!mLibrary.open("libjack.so.0") && !mLibrary.open("libjack.so"))
      {
         mError = "Unable to load the JACK library";
         return;
      }

      Load(clientOpen, "jack_client_open");
      Load(clientClose, "jack_client_close");
      Load(activate, "jack_activate");
      Load(deactivate, "jack_deactivate");
      Load(portRegister, "jack_port_register");
      Load(portName, "jack_port_name");
      Load(getPorts, "jack_get_ports");
      Load(free, "jack_free");
      Load(connect, "jack_connect");
      Load(portGetBuffer, "jack_port_get_buffer");
      Load(portGetLatencyRange, "jack_port_get_latency_range");
      Load(getSampleRate, "jack_get_sample_rate");
      Load(getBufferSize, "jack_get_buffer_size");
      Load(setProcessCallback, "jack_set_process_callback");
      Load(setSampleRateCallback, "jack_set_sample_rate_callback");
      Load(setBufferSizeCallback, "jack_set_buffer_size_callback");
      Load(setXrunCallback, "jack_set_xrun_callback");
      Load(onShutdown, "jack_on_shutdown");
   }

   const juce::String& GetError() const { return mError; }

   decltype(&::jack_client_open) clientOpen{ nullptr };
   decltype(&::jack_client_close) clientClose{ nullptr };
   decltype(&::jack_activate) activate{ nullptr };
   decltype(&::jack_deactivate) deactivate{ nullptr };
   decltype(&::jack_port_register) portRegister{ nullptr };
   decltype(&::jack_port_name) portName{ nullptr };
   decltype(&::jack_get_ports) getPorts{ nullptr };
   decltype(&::jack_free) free{ nullptr };
   decltype(&::jack_connect) connect{ nullptr };
   decltype(&::jack_port_get_buffer) portGetBuffer{ nullptr };
   decltype(&::jack_port_get_latency_range) portGetLatencyRange{ nullptr };
   decltype(&::jack_get_sample_rate) getSampleRate{ nullptr };
   decltype(&::jack_get_buffer_size) getBufferSize{ nullptr };
   decltype(&::jack_set_process_callback) setProcessCallback{ nullptr };
   decltype(&::jack_set_sample_rate_callback) setSampleRateCallback{ nullptr };
   decltype(&::jack_set_buffer_size_callback) setBufferSizeCallback{ nullptr };
   decltype(&::jack_set_xrun_callback) setXrunCallback{ nullptr };
   decltype(&::jack_on_shutdown) onShutdown{ nullptr };

private:
   template <typename Function>
   void Load(Function& function, const char* name)
   {
      function = reinterpret_cast<Function>(mLibrary.getFunction(name));
      if (function == nullptr && mError.isEmpty())
         mError = "Missing JACK function: " + juce::String(name);
   }

   juce::DynamicLibrary mLibrary;
   juce::String mError;
};

class JackAudioDevice final : public juce::AudioIODevice,
                              private juce::AsyncUpdater
{
public:
   JackAudioDevice(std::shared_ptr<JackAudioApi> api,
                   const juce::String& inputName, const juce::String& outputName,
                   int numInputs, int numOutputs, bool autoConnect)
   : AudioIODevice(outputName.isEmpty() ? inputName : outputName, "JACK")
   , mInputName(inputName)
   , mOutputName(outputName)
   , mApi(std::move(api))
   , mAutoConnect(autoConnect)
   {
      mInputChannels.setRange(0, inputName.isEmpty() ? 0 : numInputs, true);
      mOutputChannels.setRange(0, outputName.isEmpty() ? 0 : numOutputs, true);
      mInputPointers.resize(mInputChannels.countNumberOfSetBits());
      mOutputPointers.resize(mOutputChannels.countNumberOfSetBits());
      mLastError = mApi->GetError();
      if (mLastError.isNotEmpty())
         return;

      jack_status_t status{};
      mClient = mApi->clientOpen("BespokeSynth", JackNoStartServer, &status);
      if (mClient == nullptr)
      {
         mLastError = "Unable to open JACK client (status " + juce::String(static_cast<int>(status)) + ")";
         return;
      }

      if (!RegisterPorts(mInputPorts, "in_", mInputPointers.size(), JackPortIsInput) ||
          !RegisterPorts(mOutputPorts, "out_", mOutputPointers.size(), JackPortIsOutput))
         return;

      mSampleRate = mApi->getSampleRate(mClient);
      mBufferSize = mApi->getBufferSize(mClient);
      if (mApi->setProcessCallback(mClient, ProcessCallback, this) != 0 ||
          mApi->setSampleRateCallback(mClient, ConfigurationCallback, this) != 0 ||
          mApi->setBufferSizeCallback(mClient, ConfigurationCallback, this) != 0 ||
          mApi->setXrunCallback(mClient, XrunCallback, this) != 0)
      {
         mLastError = "Unable to install JACK audio callbacks";
         return;
      }
      mApi->onShutdown(mClient, ShutdownCallback, this);
   }

   ~JackAudioDevice() override
   {
      close();
      if (mClient != nullptr)
      {
         if (mApi->clientClose(mClient) != 0)
            juce::Logger::writeToLog("Unable to close the JACK client");
      }
      cancelPendingUpdate();
   }

   const juce::String& GetInputName() const { return mInputName; }
   const juce::String& GetOutputName() const { return mOutputName; }
   juce::StringArray getInputChannelNames() override { return GetChannelNames("in_", mInputPointers.size()); }
   juce::StringArray getOutputChannelNames() override { return GetChannelNames("out_", mOutputPointers.size()); }
   juce::Array<double> getAvailableSampleRates() override { return { getCurrentSampleRate() }; }
   juce::Array<int> getAvailableBufferSizes() override { return { getCurrentBufferSizeSamples() }; }
   int getDefaultBufferSize() override { return getCurrentBufferSizeSamples(); }
   int getCurrentBufferSizeSamples() override { return static_cast<int>(mBufferSize); }
   double getCurrentSampleRate() override { return mSampleRate; }
   int getCurrentBitDepth() override { return 32; }
   juce::String getLastError() override { return mLastError; }
   bool isOpen() override { return mOpen && !mServerStopped; }
   bool isPlaying() override
   {
      const juce::ScopedLock lock(mCallbackLock);
      return isOpen() && mCallback != nullptr;
   }
   juce::BigInteger getActiveInputChannels() const override { return mInputChannels; }
   juce::BigInteger getActiveOutputChannels() const override { return mOutputChannels; }
   int getInputLatencyInSamples() override { return GetLatency(mInputPorts, JackCaptureLatency); }
   int getOutputLatencyInSamples() override { return GetLatency(mOutputPorts, JackPlaybackLatency); }
   int getXRunCount() const noexcept override { return mXruns.load(); }

   juce::String open(const juce::BigInteger&, const juce::BigInteger&, double sampleRate, int bufferSizeSamples) override
   {
      if (mLastError.isNotEmpty())
         return mLastError;
      close();
      if (mServerStopped)
         return mLastError = "The JACK server has shut down";
      if (sampleRate != getCurrentSampleRate() || bufferSizeSamples != getCurrentBufferSizeSamples())
         return mLastError = "JACK sample rate and buffer size must match the server";
      if (mApi->activate(mClient) != 0)
         return mLastError = "Unable to activate the JACK client";
      mOpen = true;
      mXruns = 0;
      if (mAutoConnect)
      {
         mLastError = ConnectPorts(mInputPorts, mInputName, true);
         if (mLastError.isEmpty())
            mLastError = ConnectPorts(mOutputPorts, mOutputName, false);
         if (mLastError.isNotEmpty())
         {
            close();
            return mLastError;
         }
      }
      return {};
   }

   void close() override
   {
      stop();
      if (mOpen && !mServerStopped && mApi->deactivate(mClient) != 0)
      {
         mLastError = "Unable to deactivate the JACK client";
         juce::Logger::writeToLog(mLastError);
      }
      mOpen = false;
   }

   void start(juce::AudioIODeviceCallback* callback) override
   {
      stop();
      if (callback != nullptr && isOpen())
      {
         callback->audioDeviceAboutToStart(this);
         const juce::ScopedLock lock(mCallbackLock);
         mCallback = callback;
      }
   }

   void stop() override
   {
      juce::AudioIODeviceCallback* callback;
      {
         const juce::ScopedLock lock(mCallbackLock);
         callback = mCallback;
         mCallback = nullptr;
      }
      if (callback != nullptr)
         callback->audioDeviceStopped();
   }

private:
   juce::String ConnectPorts(const std::vector<jack_port_t*>& ports, const juce::String& clientName, bool inputs)
   {
      if (ports.empty())
         return {};

      const auto freePorts = [this](const char** names)
      {
         mApi->free(names);
      };
      std::unique_ptr<const char*, decltype(freePorts)> names(
      mApi->getPorts(mClient, nullptr, JACK_DEFAULT_AUDIO_TYPE, inputs ? JackPortIsOutput : JackPortIsInput), freePorts);
      size_t channel = 0;
      if (names != nullptr)
      {
         for (size_t i = 0; names.get()[i] != nullptr && channel < ports.size(); ++i)
         {
            const juce::String externalPort(names.get()[i]);
            if (externalPort.upToFirstOccurrenceOf(":", false, false) != clientName)
               continue;

            const char* localPort = mApi->portName(ports[channel++]);
            const char* source = inputs ? names.get()[i] : localPort;
            const char* destination = inputs ? localPort : names.get()[i];
            const int error = mApi->connect(mClient, source, destination);
            if (error != 0 && error != EEXIST)
               return "Unable to connect JACK ports " + juce::String(source) + " -> " + juce::String(destination) +
                      " (error " + juce::String(error) + ")";
         }
      }
      if (channel == 0)
         return "No audio ports available on JACK device " + clientName;
      return {};
   }

   static juce::StringArray GetChannelNames(const char* prefix, size_t count)
   {
      juce::StringArray names;
      for (size_t i = 0; i < count; ++i)
         names.add(juce::String(prefix) + juce::String(static_cast<int>(i + 1)));
      return names;
   }

   bool RegisterPorts(std::vector<jack_port_t*>& ports, const char* prefix, size_t count, unsigned long flags)
   {
      for (size_t i = 0; i < count; ++i)
      {
         const auto name = juce::String(prefix) + juce::String(static_cast<int>(i + 1));
         auto* port = mApi->portRegister(mClient, name.toRawUTF8(), JACK_DEFAULT_AUDIO_TYPE, flags, 0);
         if (port == nullptr)
         {
            mLastError = "Unable to register JACK port " + name;
            return false;
         }
         ports.push_back(port);
      }
      return true;
   }

   int GetLatency(const std::vector<jack_port_t*>& ports, jack_latency_callback_mode_t mode)
   {
      if (mServerStopped)
         return 0;
      jack_nframes_t latency = 0;
      for (auto* port : ports)
      {
         jack_latency_range_t range{};
         mApi->portGetLatencyRange(port, mode, &range);
         latency = juce::jmax(latency, range.max);
      }
      return static_cast<int>(latency);
   }

   static int ProcessCallback(jack_nframes_t samples, void* context)
   {
      auto& device = *static_cast<JackAudioDevice*>(context);
      const juce::ScopedLock lock(device.mCallbackLock);
      if (device.mServerStopped)
         return 0;
      if (samples != device.mBufferSize)
      {
         device.mConfigurationChanged = true;
         device.triggerAsyncUpdate();
      }
      bool buffersAvailable = true;
      for (size_t i = 0; i < device.mOutputPorts.size(); ++i)
      {
         device.mOutputPointers[i] = static_cast<float*>(device.mApi->portGetBuffer(device.mOutputPorts[i], samples));
         if (device.mOutputPointers[i] != nullptr)
            juce::FloatVectorOperations::clear(device.mOutputPointers[i], static_cast<int>(samples));
         else
            buffersAvailable = false;
      }
      for (size_t i = 0; i < device.mInputPorts.size(); ++i)
      {
         device.mInputPointers[i] = static_cast<const float*>(device.mApi->portGetBuffer(device.mInputPorts[i], samples));
         buffersAvailable = buffersAvailable && device.mInputPointers[i] != nullptr;
      }
      if (!buffersAvailable)
      {
         device.mBufferUnavailable = true;
         device.triggerAsyncUpdate();
         return 0;
      }

      // JACK supplies silent buffers for unconnected inputs. Process every port in physical order.
      if (device.mCallback != nullptr && !device.mConfigurationChanged && !device.mBufferUnavailable)
         device.mCallback->audioDeviceIOCallbackWithContext(device.mInputPointers.data(), static_cast<int>(device.mInputPointers.size()),
                                                            device.mOutputPointers.data(), static_cast<int>(device.mOutputPointers.size()),
                                                            static_cast<int>(samples), {});
      return 0;
   }

   static int ConfigurationCallback(jack_nframes_t, void* context)
   {
      auto& device = *static_cast<JackAudioDevice*>(context);
      if (device.mApi->getSampleRate(device.mClient) != device.mSampleRate ||
          device.mApi->getBufferSize(device.mClient) != device.mBufferSize)
      {
         device.mConfigurationChanged = true;
         device.triggerAsyncUpdate();
      }
      return 0;
   }

   static int XrunCallback(void* context)
   {
      ++static_cast<JackAudioDevice*>(context)->mXruns;
      return 0;
   }

   static void ShutdownCallback(void* context)
   {
      auto& device = *static_cast<JackAudioDevice*>(context);
      device.mServerStopped = true;
      device.triggerAsyncUpdate();
   }

   void handleAsyncUpdate() override
   {
      if (mServerStopped)
         mLastError = "The JACK server has shut down";
      else if (mBufferUnavailable)
         mLastError = "Unable to access JACK audio port buffers";
      else
         mLastError = "JACK sample rate or buffer size changed; restart BespokeSynth";
      {
         const juce::ScopedLock lock(mCallbackLock);
         if (mCallback != nullptr)
            mCallback->audioDeviceError(mLastError);
      }
      juce::Logger::writeToLog(mLastError);
      close();
   }

   juce::String mInputName, mOutputName;
   std::shared_ptr<JackAudioApi> mApi;
   bool mAutoConnect;
   jack_client_t* mClient{ nullptr };
   juce::String mLastError;
   jack_nframes_t mSampleRate{ 0 }, mBufferSize{ 0 };
   std::atomic<bool> mOpen{ false }, mServerStopped{ false }, mConfigurationChanged{ false };
   std::atomic<bool> mBufferUnavailable{ false };
   std::atomic<int> mXruns{ 0 };
   juce::CriticalSection mCallbackLock;
   juce::AudioIODeviceCallback* mCallback{ nullptr };
   juce::BigInteger mInputChannels, mOutputChannels;
   std::vector<jack_port_t*> mInputPorts, mOutputPorts;
   std::vector<const float*> mInputPointers;
   std::vector<float*> mOutputPointers;
};

class JackAudioDeviceType final : public juce::AudioIODeviceType
{
public:
   JackAudioDeviceType(std::unique_ptr<juce::AudioIODeviceType> type, int numInputs, int numOutputs, bool autoConnect)
   : AudioIODeviceType("JACK")
   , mType(std::move(type))
   , mApi(std::make_shared<JackAudioApi>())
   , mNumInputs(numInputs)
   , mNumOutputs(numOutputs)
   , mAutoConnect(autoConnect)
   {
   }

   void scanForDevices() override { mType->scanForDevices(); }
   juce::StringArray getDeviceNames(bool wantInputNames = false) const override
   {
      auto names = mType->getDeviceNames(wantInputNames);
      if (!mAutoConnect)
      {
         names.removeString("BespokeSynth");
         names.insert(0, "BespokeSynth");
      }
      return names;
   }
   int getDefaultDeviceIndex(bool forInput) const override { return mAutoConnect ? mType->getDefaultDeviceIndex(forInput) : 0; }
   bool hasSeparateInputsAndOutputs() const override { return true; }

   int getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const override
   {
      if (auto* jackDevice = dynamic_cast<JackAudioDevice*>(device))
         return getDeviceNames(asInput).indexOf(asInput ? jackDevice->GetInputName() : jackDevice->GetOutputName());
      return -1;
   }

   juce::AudioIODevice* createDevice(const juce::String& outputDeviceName, const juce::String& inputDeviceName) override
   {
      if ((outputDeviceName.isEmpty() && inputDeviceName.isEmpty()) ||
          (outputDeviceName.isNotEmpty() && !getDeviceNames().contains(outputDeviceName)) ||
          (inputDeviceName.isNotEmpty() && !getDeviceNames(true).contains(inputDeviceName)))
         return nullptr;

      return new JackAudioDevice(mApi, inputDeviceName, outputDeviceName, mNumInputs, mNumOutputs, mAutoConnect);
   }

private:
   std::unique_ptr<juce::AudioIODeviceType> mType;
   std::shared_ptr<JackAudioApi> mApi;
   int mNumInputs, mNumOutputs;
   bool mAutoConnect;
};
