#include "NoiseSynth.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "IAudioReceiver.h"
#include "Scale.h"

NoiseSynthVoice::NoiseSynthVoice()
{
   mHil.assign(kHilbertLen, 0.0f);
   for (int k = 0; k < kHilbertLen; ++k)
   {
      int m = k - kHilbertMid;
      if (m != 0 && (m % 2) != 0) // only odd symmetric terms
      {
         float win = 0.54f - 0.46f * cosf(FTWO_PI * k / (kHilbertLen - 1)); // Hamming
         mHil[k] = win * 2.0f / (PI * m);
      }
   }
   mDelayLine.assign(256, 0.0f);
}

void NoiseSynthVoice::PlayNote(double time, int pitch, int velocity, const ADSR& masterAdsr)
{
   mNoteOn = true;
   mPitch = pitch;
   float normVelocity = velocity / 127.0f;

   mAdsr.Start(time, normVelocity, masterAdsr);
}

void NoiseSynthVoice::Process(float* outL, float* outR, int numSamples, double time, double sampleIncrementMs, float freqShift, float cutoff, float res, int filterType, int noiseColor, float volume)
{
   if (!IsActive(time))
      return;

   // Update filter tracking pitch
   float pitchFreq = TheScale->PitchToFreq(mPitch);
   float cutoffFreq = pitchFreq * powf(2.0f, (cutoff - 0.5f) * 10.0f);
   cutoffFreq = ofClamp(cutoffFreq, 20.0f, 20000.0f);
   mFilter.SetFilterType(filterType == 0 ? FilterType::kFilterType_Lowpass : (filterType == 1 ? FilterType::kFilterType_Highpass : FilterType::kFilterType_Bandpass));
   mFilter.SetFilterParams(cutoffFreq, ofClamp(res * 10.0f, 0.1f, 10.0f));

   float phaseInc = freqShift * FTWO_PI / gSampleRate;

   for (int i = 0; i < numSamples; ++i)
   {
      if (mAdsr.IsDone(time))
      {
         mNoteOn = false;
         break;
      }

      float adsrVal = mAdsr.Value(time);
      time += sampleIncrementMs;
      float rawNoise = ofRandom(-1.0f, 1.0f);
      float noiseVal = 0.0f;

      if (noiseColor == 1) // Pink
      {
         // Simple pink filter approx
         mNoiseState1 = mNoiseState1 + 0.1f * (rawNoise - mNoiseState1);
         noiseVal = mNoiseState1 * 2.0f;
      }
      else if (noiseColor == 2) // Brown
      {
         mNoiseState1 = mNoiseState1 + 0.02f * (rawNoise - mNoiseState1);
         noiseVal = mNoiseState1 * 3.0f;
      }
      else // White
      {
         noiseVal = rawNoise;
      }

      // Filter
      noiseVal = mFilter.Filter(noiseVal);

      // Envelope and Volume
      noiseVal *= adsrVal * volume;

      // Frequency Shift (Hilbert)
      if (freqShift != 0.0f)
      {
         mDelayLine[mDelayPos] = noiseVal;

         float hilbertOut = 0.0f;
         for (int k = 0; k < kHilbertLen; ++k)
         {
            int idx = (mDelayPos - k + mDelayLine.size()) % mDelayLine.size();
            hilbertOut += mDelayLine[idx] * mHil[k];
         }

         int delayIdx = (mDelayPos - kHilbertMid + mDelayLine.size()) % mDelayLine.size();
         float delayedIn = mDelayLine[delayIdx];

         mPhase += phaseInc;
         if (mPhase > FTWO_PI)
            mPhase -= FTWO_PI;
         if (mPhase < 0.0)
            mPhase += FTWO_PI;

         float shifted = delayedIn * cosf(mPhase) - hilbertOut * sinf(mPhase);
         noiseVal = shifted;

         mDelayPos = (mDelayPos + 1) % mDelayLine.size();
      }

      outL[i] += noiseVal;
      if (outL != outR)
         outR[i] += noiseVal;
   }
}

// ----------------------------------------------------

NoiseSynth::NoiseSynth()
: mNoteInputBuffer(this)
{
}

NoiseSynth::~NoiseSynth()
{
}

void NoiseSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   int y = 20;

   mNoiseColorDropdown = new DropdownList(this, "Color", 10, y, &mNoiseColor);
   mNoiseColorDropdown->AddLabel("White", 0);
   mNoiseColorDropdown->AddLabel("Pink", 1);
   mNoiseColorDropdown->AddLabel("Brown", 2);
   y += 20;

   mFilterTypeDropdown = new DropdownList(this, "Filter", 10, y, &mFilterType);
   mFilterTypeDropdown->AddLabel("Lowpass", 0);
   mFilterTypeDropdown->AddLabel("Highpass", 1);
   mFilterTypeDropdown->AddLabel("Bandpass", 2);
   y += 20;

   mCutoffSlider = new FloatSlider(this, "cutoff", 10, y, 100, 15, &mCutoff, 0.0f, 1.0f);
   y += 20;

   mResonanceSlider = new FloatSlider(this, "resonance", 10, y, 100, 15, &mResonance, 0.0f, 1.0f);
   y += 20;

   mFreqShiftSlider = new FloatSlider(this, "freq shift", 10, y, 100, 15, &mFreqShift, -1000.0f, 1000.0f);
   mFreqShiftSlider->SetMode(FloatSlider::kSquare);
   y += 20;

   mAdsrDisplay = new ADSRDisplay(this, "adsr", 10, y, 100, 50, &mMasterAdsr);
}

void NoiseSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mNoiseColorDropdown->Draw();
   mFilterTypeDropdown->Draw();
   mCutoffSlider->Draw();
   mResonanceSlider->Draw();
   mFreqShiftSlider->Draw();
   mAdsrDisplay->Draw();
}

void NoiseSynth::Process(double time)
{
   PROFILER(NoiseSynth);
   mNoteInputBuffer.Process(time);

   IAudioReceiver* target = GetTarget();
   if (target == nullptr)
      return;

   int bufferSize = target->GetBuffer()->BufferSize();
   float* outL = target->GetBuffer()->GetChannel(0);
   float* outR = target->GetBuffer()->GetChannel(1);
   bool isStereo = target->GetBuffer()->NumActiveChannels() > 1;

   Clear(outL, bufferSize);
   if (isStereo)
      Clear(outR, bufferSize);

   for (auto& voice : mVoices)
   {
      if (voice.IsActive(time))
      {
         voice.Process(outL, isStereo ? outR : outL, bufferSize, time, gInvSampleRateMs, mFreqShift, mCutoff, mResonance, mFilterType, mNoiseColor, mVolume);
      }
   }
}

void NoiseSynth::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time))
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   int pitch = note.pitch;
   int velocity = note.velocity;
   double time = note.time;

   mNoteHistory.AddEvent(time, velocity > 0, pitch);

   if (velocity > 0)
   {
      // Find free voice
      for (auto& voice : mVoices)
      {
         if (!voice.IsActive(time))
         {
            voice.PlayNote(time, pitch, velocity, mMasterAdsr);
            return;
         }
      }

      // If no free voice, steal the oldest (just steal first for simplicity here)
      mVoices[0].PlayNote(time, pitch, velocity, mMasterAdsr);
   }
   else
   {
      // Note off
      for (auto& voice : mVoices)
      {
         if (voice.IsActive(time) && voice.mPitch == pitch)
         {
            // Note off
            voice.mNoteOn = false;
            voice.mAdsr.Stop(time);
         }
      }
   }
}

void NoiseSynth::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void NoiseSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void NoiseSynth::SetUpFromSaveData()
{
}

void NoiseSynth::GetModuleDimensions(float& width, float& height)
{
   width = 120;
   height = 190;
}
