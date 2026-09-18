#include "LimiterEffect.h"
#include "SynthGlobals.h"
#include "Profiler.h"
#include "OpenFrameworksPort.h"

namespace
{
   static double lin2dB(double lin)
   {
      static const double LOG_2_DB = 8.6858896380650365530225783783321; // 20 / ln( 10 )
      return log(lin) * LOG_2_DB;
   }

   static double dB2lin(double dB)
   {
      static const double DB_2_LOG = 0.11512925464970228420089957273422; // ln( 10 ) / 20
      return exp(dB * DB_2_LOG);
   }
}

LimiterEffect::LimiterEffect()
{
   SetRelease(mRelease);
}

void LimiterEffect::CreateUIControls()
{
   IDrawableModule::CreateUIControls();
   mThresholdSlider = new FloatSlider(this, "threshold", 5, 20, 110, 15, &mThreshold, -40.0f, 0.0f);
   mReleaseSlider = new FloatSlider(this, "release", 5, 40, 110, 15, &mRelease, 1.0f, 1000.0f);
   mInGainSlider = new FloatSlider(this, "in gain", 5, 60, 110, 15, &mInGain, -20.0f, 20.0f);
   mOutGainSlider = new FloatSlider(this, "out gain", 5, 80, 110, 15, &mOutGain, -20.0f, 20.0f);

   mWidth = 140; // slightly wider for meter
   mHeight = 100;
}

void LimiterEffect::SetRelease(float releaseMs)
{
   // exp(-1 / (release_sec * sampleRate))
   double dt = releaseMs / 1000.0;
   mReleaseCoef = exp(-1.0 / (dt * gSampleRate));
}

void LimiterEffect::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mReleaseSlider)
      SetRelease(mRelease);
}

void LimiterEffect::ProcessAudio(double time, ChannelBuffer* buffer)
{
   PROFILER(LimiterEffect);

   ComputeSliders(buffer->BufferSize());

   if (!mEnabled)
      return;

   int bufferSize = buffer->BufferSize();
   int numChannels = buffer->NumActiveChannels();
   float inGainLinear = dB2lin(mInGain);
   float outGainLinear = dB2lin(mOutGain);
   float threshLinear = dB2lin(mThreshold);

   double minGR = 1.0;

   for (int i = 0; i < bufferSize; ++i)
   {
      double maxAbs = 0.0;
      for (int ch = 0; ch < numChannels; ++ch)
      {
         float sample = buffer->GetChannel(ch)[i] * inGainLinear;
         maxAbs = std::max(maxAbs, (double)std::abs(sample));
      }

      if (maxAbs > mEnvelope)
         mEnvelope = maxAbs; // instant attack
      else
         mEnvelope = maxAbs + mReleaseCoef * (mEnvelope - maxAbs);

      float gain = 1.0f;
      if (mEnvelope > threshLinear && mEnvelope > 0.0)
         gain = threshLinear / mEnvelope;

      minGR = std::min(minGR, (double)gain);

      for (int ch = 0; ch < numChannels; ++ch)
      {
         float in = buffer->GetChannel(ch)[i] * inGainLinear;
         buffer->GetChannel(ch)[i] = in * gain * outGainLinear;
      }
   }

   // smooth the UI meter slightly to avoid flickering
   mGainReduction = mGainReduction * 0.9 + minGR * 0.1;
}

void LimiterEffect::DrawModule()
{
   mThresholdSlider->Draw();
   mReleaseSlider->Draw();
   mInGainSlider->Draw();
   mOutGainSlider->Draw();

   ofPushStyle();

   int meterX = 122;
   int meterY = 20;
   int meterW = 10;
   int meterH = 75;

   // draw meter background
   ofSetColor(30, 30, 30);
   ofRect(meterX, meterY, meterW, meterH);

   float grDb = lin2dB((float)mGainReduction);
   if (grDb < -20.0f)
      grDb = -20.0f;
   if (grDb > 0.0f)
      grDb = 0.0f;

   float fillPct = -grDb / 20.0f;

   ofSetColor(100, 200, 100);
   ofRect(meterX, meterY, meterW, meterH * fillPct);

   ofPopStyle();
}
