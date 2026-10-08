#include "StreamingAudioResampler.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <utility>
#include <vector>

namespace
{
   std::atomic<int> gAllocations{ 0 };
   constexpr double kPi = 3.14159265358979323846;

   struct SineSource
   {
      double sampleRate;
      double frequency;
      int channels;
      uint64_t position{ 0 };

      static void Read(void* context, float* output, int frames)
      {
         auto& source = *static_cast<SineSource*>(context);
         for (int frame = 0; frame < frames; ++frame)
         {
            const float value = (float)std::sin(2.0 * kPi * source.frequency * source.position++ / source.sampleRate);
            for (int channel = 0; channel < source.channels; ++channel)
               output[frame * source.channels + channel] = channel == 0 ? value : 0.0f;
         }
      }
   };

   bool Expect(bool condition, const char* message)
   {
      if (!condition)
         std::cerr << message << '\n';
      return condition;
   }

   std::vector<float> ConvertSine(int inputRate, int outputRate, int frequency, int channels = 1, int callbackSize = 64)
   {
      SineSource source{ (double)inputRate, (double)frequency, channels };
      StreamingAudioResampler resampler(channels, &SineSource::Read, &source);
      std::vector<std::vector<float>> samples(channels, std::vector<float>(outputRate));
      std::vector<float*> output(channels);
      for (int offset = 0; offset < outputRate; offset += callbackSize)
      {
         for (int channel = 0; channel < channels; ++channel)
            output[channel] = samples[channel].data() + offset;
         if (!resampler.Process(output.data(), std::min(callbackSize, outputRate - offset), (double)outputRate / inputRate))
            return {};
      }
      for (int channel = 1; channel < channels; ++channel)
         for (float value : samples[channel])
            if (std::abs(value) > 1e-6f)
               return {};
      return samples[0];
   }

   double ToneAmplitude(const std::vector<float>& samples, int sampleRate, int frequency)
   {
      const int start = sampleRate / 4;
      double sine = 0;
      double cosine = 0;
      for (int frame = start; frame < (int)samples.size(); ++frame)
      {
         const double phase = 2.0 * kPi * frequency * frame / sampleRate;
         sine += samples[frame] * std::sin(phase);
         cosine += samples[frame] * std::cos(phase);
      }
      return 2.0 * std::hypot(sine, cosine) / (samples.size() - start);
   }
}

void* operator new(std::size_t size)
{
   ++gAllocations;
   if (void* memory = std::malloc(size))
      return memory;
   throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

int main()
{
   bool passed = true;
   const auto highTone = ConvertSine(48000, 24000, 18000);
   passed &= Expect(!highTone.empty() && ToneAmplitude(highTone, 24000, 6000) < 0.000316,
                    "48 to 24 kHz conversion aliases an 18 kHz tone above -70 dB");
   const auto lowTone = ConvertSine(48000, 24000, 1000);
   passed &= Expect(!lowTone.empty() && ToneAmplitude(lowTone, 24000, 1000) > 0.9,
                    "48 to 24 kHz conversion attenuates a 1 kHz tone");
   const auto unevenCallbacks = ConvertSine(48000, 24000, 1000, 1, 257);
   double maximumDifference = 0;
   if (unevenCallbacks.size() == lowTone.size())
      for (size_t index = 0; index < lowTone.size(); ++index)
         maximumDifference = std::max(maximumDifference, (double)std::abs(unevenCallbacks[index] - lowTone[index]));
   passed &= Expect(unevenCallbacks.size() == lowTone.size() && maximumDifference < 1e-6,
                    "conversion depends on callback length");
   const auto downsampled441 = ConvertSine(44100, 24000, 17000);
   passed &= Expect(!downsampled441.empty() && ToneAmplitude(downsampled441, 24000, 7000) < 0.000316,
                    "44.1 to 24 kHz conversion aliases a 17 kHz tone above -70 dB");
   const auto downsampled96 = ConvertSine(96000, 44100, 30000);
   passed &= Expect(!downsampled96.empty() && ToneAmplitude(downsampled96, 44100, 14100) < 0.000316,
                    "96 to 44.1 kHz conversion aliases a 30 kHz tone above -70 dB");
   const auto upsampled = ConvertSine(24000, 48000, 1000, 2);
   passed &= Expect(!upsampled.empty() && ToneAmplitude(upsampled, 48000, 23000) < 0.000316,
                    "24 to 48 kHz conversion leaves an image above -70 dB or leaks into channel two");
   const auto sixChannels = ConvertSine(48000, 24000, 1000, 6);
   passed &= Expect(!sixChannels.empty() && ToneAmplitude(sixChannels, 24000, 1000) > 0.9,
                    "six channel conversion lost the signal or leaked into another channel");
   const auto sixteenChannels = ConvertSine(48000, 24000, 1000, 16);
   passed &= Expect(!sixteenChannels.empty() && ToneAmplitude(sixteenChannels, 24000, 1000) > 0.9,
                    "sixteen channel conversion lost the signal or leaked into another channel");
   for (const auto [inputRate, outputRate] : { std::pair{ 44100, 48000 }, std::pair{ 48000, 44100 } })
   {
      const auto samples = ConvertSine(inputRate, outputRate, 1000);
      passed &= Expect(!samples.empty() && ToneAmplitude(samples, outputRate, 1000) > 0.9,
                       "44.1/48 kHz conversion loses a 1 kHz tone");
   }

   SineSource source{ 192000.0, 1000.0, 2 };
   StreamingAudioResampler driftResampler(2, &SineSource::Read, &source);
   std::vector<float> left(2048);
   std::vector<float> right(2048);
   float* output[] = { left.data(), right.data() };
   const int allocationBaseline = gAllocations.load();
   for (int callback = 0; callback < 100; ++callback)
   {
      const double correction = callback % 2 == 0 ? 0.002 : -0.002;
      passed &= Expect(driftResampler.Process(output, 2048, 24000.0 / (192000.0 * (1.0 + correction))),
                       "192 to 24 kHz drift conversion stalled");
   }
   passed &= Expect(gAllocations == allocationBaseline, "conversion allocated memory during Process");
   return passed ? 0 : 1;
}
