#include "Radar.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include <cmath>

Radar::Radar()
{
}

Radar::~Radar()
{
}

void Radar::Init()
{
   IDrawableModule::Init();
}

void Radar::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   FLOATSLIDER(mRadarSizeSlider, "size", &mRadarSize, 20, 200);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mRadarDurationSlider, "duration", &mRadarDuration, 100, 5000);
   UIBLOCK_NEWLINE();
   INTSLIDER(mNumRadarsSlider, "no of radars", &mNumRadars, 1, 5);
   UIBLOCK_NEWLINE();
   INTSLIDER(mNumNotesSlider, "no of notes", &mNumNotes, 1, 50);
   UIBLOCK_NEWLINE();
   INTSLIDER(mLowsSlider, "low", &mLows, 0, 127);
   UIBLOCK_NEWLINE();
   INTSLIDER(mHighsSlider, "high", &mHighs, 0, 127);
   UIBLOCK_NEWLINE();

   TEXTENTRY_NUM(mSeedEntry, "seed", 4, &mSeed, 0, 9999);
   BUTTON(mPrevSeedButton, "<");
   BUTTON(mReseedButton, "*");
   BUTTON(mNextSeedButton, ">");

   mPrevSeedButton->PositionTo(mSeedEntry, kAnchor_Right);
   mReseedButton->PositionTo(mPrevSeedButton, kAnchor_Right);
   mNextSeedButton->PositionTo(mReseedButton, kAnchor_Right);

   ENDUIBLOCK(mWidth, mHeight);

   mArenaX = mWidth + 15;
   mArenaY = 10;
   mArenaW = 220;
   mArenaH = 220;

   mWidth += mArenaW + 30;
   mHeight = MAX(mHeight, mArenaH + 20);

   RecalculateTriggers();
}

void Radar::RecalculateTriggers()
{
   mTriggers.clear();
   int minPitch = MIN(mLows, mHighs);
   int maxPitch = MAX(mLows, mHighs);

   float maxRadius = MIN(mRadarSize, mArenaW / 2.0f - 10.0f); // Restrict to box

   for (int i = 0; i < mNumNotes; ++i)
   {
      TriggerPoint tp;
      tp.angle = DeterministicRandomFloat01(mSeed, i * 3) * 6.28318530718f; // TWO_PI
      tp.distance = DeterministicRandomFloat01(mSeed, i * 3 + 1) * maxRadius;

      float randVal = DeterministicRandomFloat01(mSeed, i * 3 + 2);
      tp.pitch = minPitch + (int)(randVal * (maxPitch - minPitch + 1));
      tp.pitch = ofClamp(tp.pitch, 0, 127);

      tp.phase = tp.distance / MAX(0.001f, mRadarSize);
      tp.flashAmount = 0.0f;
      mTriggers.push_back(tp);
   }
}

void Radar::Poll()
{
   if (!IsEnabled())
      return;

   double time = gTime;
   if (mLastTime == 0)
      mLastTime = time;
   double dt = (time - mLastTime) / 1000.0;
   mLastTime = time;

   // Avoid huge jumps
   dt = MIN(dt, 0.1);

   float dPhase = dt / (mRadarDuration / 1000.0f);
   mPhase += dPhase;

   for (auto& tp : mTriggers)
   {
      // Decay flash
      tp.flashAmount = MAX(0.0f, tp.flashAmount - (float)dt * 2.0f);

      // Check against all active radars
      for (int i = 0; i < mNumRadars; ++i)
      {
         // The radar phase progresses backwards from the outer edge inward if we think about it,
         // actually we want radar circles expanding outward.
         // mPhase goes 0->1. The radius goes 0->1.
         // Let radar phases be staggered.
         float offset = i / (float)mNumRadars;
         float radarOldPhase = fmod(mPhase - dPhase + offset, 1.0f);
         float radarNewPhase = fmod(mPhase + offset, 1.0f);

         bool wrapped = radarNewPhase < radarOldPhase;

         // Trigger if the radar crossed the point's phase
         bool triggered = false;
         if (!wrapped && tp.phase >= radarOldPhase && tp.phase <= radarNewPhase)
         {
            triggered = true;
         }
         else if (wrapped && (tp.phase >= radarOldPhase || tp.phase <= radarNewPhase))
         {
            triggered = true;
         }

         if (triggered)
         {
            tp.flashAmount = 1.0f;

            double playTime = NextBufferTime(false);
            PlayNoteOutput(NoteMessage(playTime, tp.pitch, 100));
            PlayNoteOutput(NoteMessage(playTime + 100, tp.pitch, 0)); // 100ms duration
         }
      }
   }

   if (mPhase >= 1.0f)
   {
      mPhase = fmod(mPhase, 1.0f);
   }
}

void Radar::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mRadarSizeSlider->Draw();
   mRadarDurationSlider->Draw();
   mNumRadarsSlider->Draw();
   mNumNotesSlider->Draw();
   mLowsSlider->Draw();
   mHighsSlider->Draw();

   mSeedEntry->Draw();
   mPrevSeedButton->Draw();
   mReseedButton->Draw();
   mNextSeedButton->Draw();

   // Draw Arena Box
   ofPushStyle();
   ofSetColor(255, 255, 255, 255);
   ofNoFill();
   ofRect(mArenaX, mArenaY, mArenaW, mArenaH, 15);

   float centerX = mArenaX + mArenaW / 2.0f;
   float centerY = mArenaY + mArenaH / 2.0f;

   // Draw Trigger Points
   for (const auto& tp : mTriggers)
   {
      float px = centerX + cosf(tp.angle) * tp.distance;
      float py = centerY + sinf(tp.angle) * tp.distance;

      ofFill();
      // Interpolate color from green to white based on flash
      ofSetColor(100 + 155 * tp.flashAmount, 200 + 55 * tp.flashAmount, 100 + 155 * tp.flashAmount, 255);

      // Draw bigger dot (radius 4 normally, expands to 6 when flashed)
      float radius = 4.0f + 2.0f * tp.flashAmount;
      ofCircle(px, py, radius);
   }

   // Draw Radar Circles
   ofSetColor(60, 150, 190, 255);
   ofNoFill();

   float maxRadiusToDraw = MIN(mRadarSize, mArenaW / 2.0f);
   for (int i = 0; i < mNumRadars; ++i)
   {
      float radarPhase = fmod(mPhase + i / (float)mNumRadars, 1.0f);
      float currentRadius = radarPhase * mRadarSize;

      if (currentRadius <= maxRadiusToDraw)
      {
         ofCircle(centerX, centerY, currentRadius);
      }
   }

   ofPopStyle();
}

void Radar::Resize(float w, float h)
{
}

void Radar::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   if (slider == mRadarSizeSlider)
   {
      RecalculateTriggers();
   }
}

void Radar::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumNotesSlider || slider == mLowsSlider || slider == mHighsSlider)
   {
      RecalculateTriggers();
   }
}

void Radar::TextEntryComplete(TextEntry* entry)
{
   if (entry == mSeedEntry)
   {
      RecalculateTriggers();
   }
}

void Radar::ButtonClicked(ClickButton* button, double time)
{
   if (button == mReseedButton)
   {
      Reseed();
   }
   else if (button == mPrevSeedButton)
   {
      if (mSeed > 0)
      {
         --mSeed;
         RecalculateTriggers();
      }
   }
   else if (button == mNextSeedButton)
   {
      if (mSeed < 9999)
      {
         ++mSeed;
         RecalculateTriggers();
      }
   }
}

void Radar::Reseed()
{
   mSeed = gRandom() % 10000;
   RecalculateTriggers();
}

void Radar::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   SetUpFromSaveData();
}

void Radar::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void Radar::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
}

void Radar::LoadState(FileStreamIn& in, int rev)
{
   //note: nothing to read here - the generic module loader already consumes the rev int that
   //SaveState() wrote (via LoadModuleSaveStateRev()) before calling LoadState(). Reading it again
   //here desyncs the file stream and corrupts every module loaded after this one.
}
