#include "MutableResonator.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"
#include "FileStream.h"
#include "IAudioReceiver.h"
#include "UIControlMacros.h"
#include <cmath>

// Standard SVF bandpass filter: process one sample through a 2-pole resonant bandpass.
// This is the textbook Hal Chamberlin / Andrew Simper SVF topology.
// Returns the bandpass output.
static inline float ProcessSvfBandpass(float input, float freq, float q,
                                       float& ic1, float& ic2, float sampleRate)
{
   // Coefficient: g = tan(pi * f / sr), clamped for stability
   float w = freq / sampleRate;
   if (w > 0.49f)
      w = 0.49f;
   if (w < 0.0001f)
      w = 0.0001f;
   float g = tanf(M_PI * w);

   // Damping coefficient k = 1/Q
   float k = 1.0f / q;

   float a1 = 1.0f / (1.0f + g * (g + k));
   float a2 = g * a1;
   float a3 = g * a2;

   float v3 = input - ic2;
   float v1 = a1 * ic1 + a2 * v3;
   float v2 = ic2 + a2 * ic1 + a3 * v3;
   ic1 = 2.0f * v1 - ic1;
   ic2 = 2.0f * v2 - ic2;

   return v1; // bandpass output
}

// Compute the nth harmonic ratio with inharmonicity stretch.
// Based on the standard stiff-string/beam inharmonicity formula:
//   f_n = n * f0 * sqrt(1 + B * n^2)
// where B is the stiffness coefficient.
static float ComputeModeRatio(int n, float stiffness)
{
   float nf = (float)(n + 1);
   return nf * sqrtf(1.0f + stiffness * nf * nf);
}

MutableResonator::MutableResonator()
: IAudioProcessor(gBufferSize)
, mWriteBuffer(gBufferSize)
, mNoteInputBuffer(this)
{
   mModuleSaveData.SetString("target", "master");
   mWriteBuffer.SetNumActiveChannels(2);

   // Initialize all mode filter states
   for (auto& m : mModes)
   {
      m.ic1 = 0;
      m.ic2 = 0;
   }
}

MutableResonator::~MutableResonator()
{
}

void MutableResonator::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 5;

   // Column 1: Resonator Body
   float col1X = 5;
   float sliderW = 115;
   float sliderH = 15;

   mTuneSlider = new FloatSlider(this, "tune", col1X, y, sliderW, sliderH, &mTune, -24.0f, 24.0f);
   y += 17;
   mStructureSlider = new FloatSlider(this, "structure", col1X, y, sliderW, sliderH, &mStructure, 0.0f, 1.0f);
   y += 17;
   mBrightnessSlider = new FloatSlider(this, "brightness", col1X, y, sliderW, sliderH, &mBrightness, 0.0f, 1.0f);
   y += 17;
   mDampingSlider = new FloatSlider(this, "damping", col1X, y, sliderW, sliderH, &mDamping, 0.0f, 1.0f);
   y += 17;
   mPositionSlider = new FloatSlider(this, "position", col1X, y, sliderW, sliderH, &mPosition, 0.01f, 0.99f);

   // Column 2: Exciter
   float col2X = 130;
   y = 5;
   mExciterMixSlider = new FloatSlider(this, "noise mix", col2X, y, sliderW, sliderH, &mExciterMix, 0.0f, 1.0f);
   y += 17;
   mMalletHardnessSlider = new FloatSlider(this, "hardness", col2X, y, sliderW, sliderH, &mMalletHardness, 0.0f, 1.0f);
   y += 17;
   mInputGainSlider = new FloatSlider(this, "input gain", col2X, y, sliderW, sliderH, &mInputGain, 0.0f, 4.0f);

   // Column 3: Output
   float col3X = 255;
   y = 5;
   mSpreadSlider = new FloatSlider(this, "spread", col3X, y, sliderW, sliderH, &mSpread, 0.0f, 1.0f);
   y += 17;
   mVolumeSlider = new FloatSlider(this, "volume", col3X, y, sliderW, sliderH, &mVolume, 0.0f, 2.0f);
   y += 17;
   mStrikeButton = new ClickButton(this, "strike!", col3X, y);

   mWidth = 380;
   mHeight = 105;
}

void MutableResonator::UpdateFilterBank()
{
   // Map structure (0..1) to a stiffness coefficient.
   // 0 = perfectly harmonic (string/organ)
   // 0.3 = slightly inharmonic (marimba)
   // 0.7 = metallic (bell/plate)
   // 1.0 = very inharmonic (gong/cymbal)
   float stiffness = mStructure * mStructure * 0.08f;

   float baseFreq = mFrequency * powf(2.0f, mTune / 12.0f);

   // Brightness controls how fast higher modes lose energy.
   // bright=1: all modes full amplitude. bright=0: steep rolloff.
   float brightnessExponent = 1.0f + (1.0f - mBrightness) * 3.0f;

   // Damping controls Q. Low damping = high Q = long ring.
   // Map damping (0..1) to Q range: Q=200 (long ring) to Q=5 (short thud)
   float baseQ = 5.0f + (1.0f - mDamping) * 295.0f;

   for (int i = 0; i < kNumModes; ++i)
   {
      float ratio = ComputeModeRatio(i, stiffness);
      float freq = baseFreq * ratio;

      // Clamp to valid range
      if (freq > gSampleRate * 0.45f)
         freq = gSampleRate * 0.45f;
      if (freq < 20.0f)
         freq = 20.0f;
      mModes[i].freq = freq;

      // Position-based nodal filtering: excitation at a node of mode n
      // produces zero amplitude for that mode. sin((n+1) * pos * PI)
      float posAmp = fabsf(sinf((float)(i + 1) * mPosition * M_PI));
      posAmp = posAmp * posAmp; // Square for sharper nulls

      // Brightness rolloff: higher modes lose amplitude
      float brightAmp = powf(1.0f / (float)(i + 1), brightnessExponent - 1.0f);

      mModes[i].amp = posAmp * brightAmp;

      // Higher modes get lower Q (decay faster) — scaled by brightness
      float modeQ = baseQ / (1.0f + (float)i * (1.0f - mBrightness) * 0.5f);
      if (modeQ < 2.0f)
         modeQ = 2.0f;
      mModes[i].q = modeQ;

      // Stereo spread: alternate modes L/R with increasing width
      float pan = ((i % 2 == 0) ? -1.0f : 1.0f) * mSpread * ((float)(i + 1) / (float)kNumModes);
      mModes[i].panL = cosf((pan + 1.0f) * 0.25f * M_PI);
      mModes[i].panR = sinf((pan + 1.0f) * 0.25f * M_PI);
   }
}

void MutableResonator::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time))
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity <= 0)
      return;

   // Set resonator frequency from pitch
   mPitch = note.pitch;
   mFrequency = TheScale->PitchToFreq(mPitch);

   // Trigger internal exciter
   mExciterVelocity = note.velocity / 127.0f;
   mExciterEnv = mExciterVelocity;
   mExciterActive = true;
   mPrevNoise = 0;

   // Exciter decay rate: hardness controls how sharp/long the exciter impulse is
   // Hard mallet = very short click, Soft mallet = longer thump
   float exciterDurationMs = 0.5f + (1.0f - mMalletHardness) * 15.0f;
   float exciterDurationSamples = exciterDurationMs * 0.001f * gSampleRate;
   mExciterDecay = powf(0.001f, 1.0f / exciterDurationSamples);

   // Update the filter bank for the new pitch
   UpdateFilterBank();
}

void MutableResonator::Process(double time)
{
   PROFILER(MutableResonator);

   mNoteInputBuffer.Process(time);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   // Update filter parameters every buffer (cheap, happens once per callback)
   UpdateFilterBank();

   int bufferSize = target->GetBuffer()->BufferSize();
   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   float* outL = mWriteBuffer.GetChannel(0);
   float* outR = mWriteBuffer.GetChannel(1);

   // Get external audio input (if patched)
   float* inputBuf = GetBuffer()->GetChannel(0);

   // One-pole filter coefficient for exciter noise hardness
   float noiseAlpha = 0.05f + mMalletHardness * 0.9f;

   for (int i = 0; i < bufferSize; ++i)
   {
      // Build exciter signal
      float exciter = 0.0f;

      // Internal exciter (from note trigger)
      if (mExciterActive)
      {
         // Click component (decaying impulse)
         float click = mExciterEnv * (1.0f - mExciterMix);

         // Noise component (filtered by hardness)
         float rawNoise = ofRandom(-1.0f, 1.0f);
         float filteredNoise = mPrevNoise + noiseAlpha * (rawNoise - mPrevNoise);
         mPrevNoise = filteredNoise;
         float noise = filteredNoise * mExciterEnv * mExciterMix;

         exciter = (click + noise) * 2.0f;

         mExciterEnv *= mExciterDecay;
         if (mExciterEnv < 0.0001f)
         {
            mExciterActive = false;
            mExciterEnv = 0;
         }
      }

      // Mix in external audio input as exciter
      float externalInput = inputBuf[i] * mInputGain;
      float totalInput = exciter + externalInput;

      // Feed through ALL modes in parallel and sum
      float sampleL = 0.0f;
      float sampleR = 0.0f;

      for (int m = 0; m < kNumModes; ++m)
      {
         if (mModes[m].amp < 0.00001f)
            continue;

         float bp = ProcessSvfBandpass(totalInput, mModes[m].freq, mModes[m].q,
                                       mModes[m].ic1, mModes[m].ic2, gSampleRate);

         float weighted = bp * mModes[m].amp;
         sampleL += weighted * mModes[m].panL;
         sampleR += weighted * mModes[m].panR;
      }

      outL[i] = sampleL * mVolume;
      outR[i] = sampleR * mVolume;
   }

   // Soft clipping to prevent harsh digital clipping
   for (int i = 0; i < bufferSize; ++i)
   {
      outL[i] = tanhf(outL[i]);
      outR[i] = tanhf(outR[i]);
   }

   // Clear input buffer after reading
   GetBuffer()->Reset();

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), bufferSize);
   }
}

void MutableResonator::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   // Column labels
   ofSetColor(180, 180, 180, 120);
   DrawTextNormal("BODY", 40, -2);
   DrawTextNormal("EXCITER", 155, -2);
   DrawTextNormal("OUTPUT", 285, -2);

   mTuneSlider->Draw();
   mStructureSlider->Draw();
   mBrightnessSlider->Draw();
   mDampingSlider->Draw();
   mPositionSlider->Draw();

   mExciterMixSlider->Draw();
   mMalletHardnessSlider->Draw();
   mInputGainSlider->Draw();

   mSpreadSlider->Draw();
   mVolumeSlider->Draw();
   mStrikeButton->Draw();
}

void MutableResonator::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void MutableResonator::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void MutableResonator::ButtonClicked(ClickButton* button, double time)
{
   if (button == mStrikeButton)
   {
      // Manual strike: trigger exciter at current frequency
      mExciterVelocity = 0.8f;
      mExciterEnv = mExciterVelocity;
      mExciterActive = true;
      mPrevNoise = 0;

      float exciterDurationMs = 0.5f + (1.0f - mMalletHardness) * 15.0f;
      float exciterDurationSamples = exciterDurationMs * 0.001f * gSampleRate;
      mExciterDecay = powf(0.001f, 1.0f / exciterDurationSamples);
   }
}

void MutableResonator::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void MutableResonator::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void MutableResonator::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);
}

void MutableResonator::LoadState(FileStreamIn& in, int rev)
{
   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   IDrawableModule::LoadState(in, rev);
}
