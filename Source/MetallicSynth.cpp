#include "MetallicSynth.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "Scale.h"
#include "FileStream.h"
#include "IAudioReceiver.h"
#include <cmath>

MetallicSynth::MetallicSynth()
: mWriteBuffer(gBufferSize)
, mNoteInputBuffer(this)
{
   mModuleSaveData.SetString("target", "master");
   mWriteBuffer.SetNumActiveChannels(2);

   // Zero out all voice data
   for (auto& v : mVoices)
   {
      v.active = false;
      v.prevNoise = 0.0f;
      for (int i = 0; i < kNumModes; ++i)
      {
         v.modePhase[i] = 0;
         v.modeFreq[i] = 0;
         v.modeAmp[i] = 0;
         v.modeDecayRate[i] = 0.999f;
         v.modePanL[i] = 0.707f;
         v.modePanR[i] = 0.707f;
      }
   }
}

MetallicSynth::~MetallicSynth()
{
}

void MetallicSynth::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float y = 5;

   // Column 1: Object (Resonator)
   float col1X = 5;
   mMaterialSelector = new DropdownList(this, "object", col1X, y, (int*)&mMaterial, 110);
   mMaterialSelector->AddLabel("bell", kMaterial_Bell);
   mMaterialSelector->AddLabel("beam", kMaterial_Beam);
   mMaterialSelector->AddLabel("string", kMaterial_String);
   mMaterialSelector->AddLabel("membrane", kMaterial_Membrane);
   mMaterialSelector->AddLabel("plate", kMaterial_Plate);
   mMaterialSelector->AddLabel("tube", kMaterial_Tube);
   mMaterialSelector->AddLabel("marimba", kMaterial_Marimba);
   y += 18;
   mTuneSlider = new FloatSlider(this, "tune", col1X, y, 110, 15, &mTune, -24.0f, 24.0f);
   y += 17;
   mFineTuneSlider = new FloatSlider(this, "fine tune", col1X, y, 110, 15, &mFineTune, -100.0f, 100.0f);
   y += 17;
   mHitPositionSlider = new FloatSlider(this, "position", col1X, y, 110, 15, &mHitPosition, 0.0f, 1.0f);
   y += 17;
   mHarmonicsDecaySlider = new FloatSlider(this, "material", col1X, y, 110, 15, &mHarmonicsDecay, 0.0f, 1.0f);
   y += 17;
   mDecaySlider = new FloatSlider(this, "decay", col1X, y, 110, 15, &mDecay, 0.0f, 1.0f);
   y += 17;
   mStiffnessSlider = new FloatSlider(this, "stiffness", col1X, y, 110, 15, &mStiffness, -1.0f, 1.0f);

   // Column 2: Exciter
   float col2X = 120;
   y = 5;
   mMalletSlider = new FloatSlider(this, "mallet", col2X, y, 110, 15, &mMallet, 0.0f, 1.0f);
   y += 17;
   mNoiseSlider = new FloatSlider(this, "noise", col2X, y, 110, 15, &mNoise, 0.0f, 1.0f);
   y += 17;
   mHardnessSlider = new FloatSlider(this, "hardness", col2X, y, 110, 15, &mHardness, 0.0f, 1.0f);

   // Column 3: Output
   float col3X = 235;
   y = 5;
   mSpreadSlider = new FloatSlider(this, "spread", col3X, y, 110, 15, &mSpread, 0.0f, 1.0f);
   y += 17;
   mVolumeSlider = new FloatSlider(this, "volume", col3X, y, 110, 15, &mVolume, 0.0f, 2.0f);

   mWidth = 350;
   mHeight = 130;
}

void MetallicSynth::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time))
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity <= 0)
      return; // Let bodies ring out naturally

   // Find free voice or steal the one whose note-on happened longest ago. exciterTime can't be
   // used for this: it freezes once the mallet-attack window ends, so a long-decaying old voice
   // and a brand-new voice can compare as "equally old" or even backwards.
   int voiceIdx = 0;
   uint64_t oldestSeq = UINT64_MAX;
   for (int i = 0; i < kNumVoices; ++i)
   {
      if (!mVoices[i].active)
      {
         voiceIdx = i;
         break;
      }
      if (mVoices[i].noteOnSeq < oldestSeq)
      {
         oldestSeq = mVoices[i].noteOnSeq;
         voiceIdx = i;
      }
   }

   Voice& v = mVoices[voiceIdx];
   v.active = true;
   v.pitch = note.pitch;
   v.velocity = note.velocity / 127.0f;
   v.exciterTime = 0;
   v.noteOnSeq = ++mNoteOnCounter;
   v.prevNoise = 0.0f; // Reset filter state

   // 12 Modes for richer synthesis
   static const float ratiosBell[kNumModes] = { 1.0f, 2.0f, 3.0f, 4.2f, 5.4f, 6.8f, 8.1f, 9.5f, 11.2f, 13.0f, 15.1f, 17.5f };
   static const float ratiosBeam[kNumModes] = { 1.0f, 2.76f, 5.4f, 8.9f, 13.3f, 18.6f, 24.8f, 31.8f, 39.8f, 48.6f, 58.4f, 69.0f };
   static const float ratiosString[kNumModes] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f };
   static const float ratiosMembrane[kNumModes] = { 1.0f, 1.58f, 2.0f, 2.24f, 2.55f, 2.91f, 3.14f, 3.5f, 3.6f, 4.06f, 4.15f, 4.62f };
   static const float ratiosPlate[kNumModes] = { 1.0f, 1.62f, 2.36f, 3.16f, 4.02f, 5.0f, 6.05f, 7.15f, 8.35f, 9.6f, 10.9f, 12.3f };
   static const float ratiosTube[kNumModes] = { 1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f, 13.0f, 15.0f, 17.0f, 19.0f, 21.0f, 23.0f };
   static const float ratiosMarimba[kNumModes] = { 1.0f, 3.99f, 10.6f, 17.3f, 25.1f, 34.0f, 44.0f, 55.0f, 67.2f, 80.5f, 94.9f, 110.0f };

   const float* ratios = ratiosBell;
   switch (mMaterial)
   {
      case kMaterial_Beam: ratios = ratiosBeam; break;
      case kMaterial_String: ratios = ratiosString; break;
      case kMaterial_Membrane: ratios = ratiosMembrane; break;
      case kMaterial_Plate: ratios = ratiosPlate; break;
      case kMaterial_Tube: ratios = ratiosTube; break;
      case kMaterial_Marimba: ratios = ratiosMarimba; break;
      default: ratios = ratiosBell; break;
   }

   float pitchOffset = mTune + mFineTune / 100.0f;
   float baseFreq = TheScale->PitchToFreq(v.pitch + pitchOffset);

   // Decay rate per sample. decay=0 → fast fade, decay=1 → long ring
   float ringTime = 0.05f + mDecay * 4.95f; // 50ms to 5 seconds
   float decayPerSec = 1.0f / ringTime;

   // Mallet softness lowpasses the initial amplitudes
   float malletMuffle = powf(1.0f - mMallet, 2.0f);

   for (int i = 0; i < kNumModes; ++i)
   {
      float stretch = 1.0f + mStiffness * 0.5f * i;
      v.modeFreq[i] = baseFreq * ratios[i] * stretch;

      // Clamp frequency to nyquist
      if (v.modeFreq[i] > gSampleRate * 0.45f)
         v.modeFreq[i] = gSampleRate * 0.45f;
      if (v.modeFreq[i] < 20.0f)
         v.modeFreq[i] = 20.0f;

      v.modePhase[i] = 0.0;

      // Hit position amplitude weighting (nodal filtering)
      // Nodes are at sin(modeIndex * position * PI) == 0
      float hitAmp = fabsf(sinf((i + 1) * mHitPosition * PI)) + 0.05f;

      // Initial amplitude: velocity scaled, higher modes quieter
      float amp = v.velocity * 0.3f * hitAmp / (float)(i + 1);

      // Mallet softness heavily muffles high frequencies
      amp *= powf(malletMuffle, (float)i * 0.5f);
      v.modeAmp[i] = amp;

      // Higher modes decay faster based on Material Damping
      // mHarmonicsDecay=0 (all decay same), =1 (highs die very fast)
      float harmDamping = powf((float)(i + 1), mHarmonicsDecay);
      float modeDecayTime = decayPerSec / harmDamping;

      v.modeDecayRate[i] = 1.0f - 1.0f / (modeDecayTime * gSampleRate);
      if (v.modeDecayRate[i] < 0.9f)
         v.modeDecayRate[i] = 0.9f;
      if (v.modeDecayRate[i] > 0.99999f)
         v.modeDecayRate[i] = 0.99999f;

      // Stereo Spread
      // Alternate modes left/right, scaled by spread
      float pan = (i % 2 == 0) ? -1.0f : 1.0f;
      pan *= mSpread;
      // Convert pan [-1, 1] to equal power gains
      v.modePanL[i] = cosf((pan + 1.0f) * 0.25f * PI);
      v.modePanR[i] = sinf((pan + 1.0f) * 0.25f * PI);
   }
}

void MetallicSynth::Process(double time)
{
   PROFILER(MetallicSynth);

   mNoteInputBuffer.Process(time);

   IAudioReceiver* target = GetTarget();
   if (!mEnabled || target == nullptr)
      return;

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   mWriteBuffer.SetNumActiveChannels(2);
   mWriteBuffer.Clear();

   float* outL = mWriteBuffer.GetChannel(0);
   float* outR = mWriteBuffer.GetChannel(1);

   for (int vIdx = 0; vIdx < kNumVoices; ++vIdx)
   {
      Voice& v = mVoices[vIdx];
      if (!v.active)
         continue;

      bool anyAudible = false;

      // Mallet duration in samples (longer mallet = softer attack)
      double malletSamples = (0.001 + mMallet * 0.05) * gSampleRate;

      // Noise filter coefficient (hardness)
      // mHardness=0 -> lowpass noise (soft), mHardness=1 -> highpass noise (hard)
      float alpha = 0.1f + mHardness * 0.8f;

      for (int i = 0; i < bufferSize; ++i)
      {
         // Exciter generation
         float exciter = 0.0f;
         if (v.exciterTime < malletSamples)
         {
            float env = 1.0f - (float)(v.exciterTime / malletSamples);

            // Generate raw noise
            float rawNoise = ofRandom(-1.0f, 1.0f);

            // Apply one-pole filter for hardness
            float filteredNoise = v.prevNoise + alpha * (rawNoise - v.prevNoise);
            v.prevNoise = filteredNoise;

            // Exciter = mix of pure impulse envelope and noise
            exciter = (env * (1.0f - mNoise) + filteredNoise * env * mNoise) * v.velocity;

            v.exciterTime++;
         }

         float sampleL = 0.0f;
         float sampleR = 0.0f;

         for (int m = 0; m < kNumModes; ++m)
         {
            if (v.modeAmp[m] < 0.00001f)
               continue;

            // Base sine wave
            float outVal = sinf((float)(v.modePhase[m] * 2.0 * PI)) * v.modeAmp[m];

            // Continuous Spread Panning
            float pan = (m % 2 == 0) ? -1.0f : 1.0f;
            pan *= mSpread;
            float panL = cosf((pan + 1.0f) * 0.25f * PI);
            float panR = sinf((pan + 1.0f) * 0.25f * PI);

            // Panning
            sampleL += outVal * panL;
            sampleR += outVal * panR;

            // Advance phase
            v.modePhase[m] += v.modeFreq[m] / gSampleRate;
            if (v.modePhase[m] > 1.0)
               v.modePhase[m] -= 1.0;

            // Exponential decay
            v.modeAmp[m] *= v.modeDecayRate[m];

            if (v.modeAmp[m] > 0.0001f)
               anyAudible = true;
         }

         // Add the exciter directly to the output so the strike is audible
         sampleL += exciter * 0.5f;
         sampleR += exciter * 0.5f;

         outL[i] += sampleL * mVolume;
         outR[i] += sampleR * mVolume;
      }

      if (!anyAudible)
         v.active = false;
   }

   // Soft clipping
   for (int i = 0; i < bufferSize; ++i)
   {
      outL[i] = tanhf(outL[i]);
      outR[i] = tanhf(outR[i]);
   }

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), bufferSize, ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), bufferSize);
   }
}

void MetallicSynth::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mMaterialSelector->Draw();
   mTuneSlider->Draw();
   mFineTuneSlider->Draw();
   mHitPositionSlider->Draw();
   mHarmonicsDecaySlider->Draw();
   mDecaySlider->Draw();
   mStiffnessSlider->Draw();

   mMalletSlider->Draw();
   mNoiseSlider->Draw();
   mHardnessSlider->Draw();

   mSpreadSlider->Draw();
   mVolumeSlider->Draw();
}

void MetallicSynth::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void MetallicSynth::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
}

void MetallicSynth::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void MetallicSynth::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));
}

void MetallicSynth::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
   IDrawableModule::SaveState(out);
}

void MetallicSynth::LoadState(FileStreamIn& in, int rev)
{
   if (ModularSynth::sLoadingFileSaveStateRev < 423)
      in >> rev;
   IDrawableModule::LoadState(in, rev);
}
