#include "BouncingBalls.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModularSynth.h"
#include "UIControlMacros.h"
#include <cmath>

BouncingBalls::BouncingBalls()
{
}

BouncingBalls::~BouncingBalls()
{
}

void BouncingBalls::Init()
{
   IDrawableModule::Init();
}

void BouncingBalls::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   INTSLIDER(mNumBallsSlider, "no of balls", &mNumBalls, 1, 30);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mSpeedSlider, "speed", &mSpeed, 10, 500);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mSizeSlider, "size of ball", &mSize, 2, 30);
   UIBLOCK_NEWLINE();
   CHECKBOX(mSymmetryCheckbox, "symmetry", &mSymmetry);
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
   mArenaW = 200;
   mArenaH = 200;

   mWidth += mArenaW + 30;
   mHeight = MAX(mHeight, mArenaH + 20);

   RespawnBalls();
}

void BouncingBalls::RespawnBalls()
{
   mBalls.clear();
   int minPitch = MIN(mLows, mHighs);
   int maxPitch = MAX(mLows, mHighs);

   for (int i = 0; i < mNumBalls; ++i)
   {
      Ball b;

      float randVal = DeterministicRandomFloat01(mSeed, i * 4);
      b.pitch = minPitch + (int)(randVal * (maxPitch - minPitch + 1));
      b.pitch = ofClamp(b.pitch, 0, 127);

      if (mSymmetry && i >= mNumBalls / 2)
      {
         // Mirror horizontally
         Ball ref = mBalls[i - mNumBalls / 2];
         b.x = mArenaW - ref.x;
         b.y = ref.y;
         b.dx = -ref.dx;
         b.dy = ref.dy;
         b.pitch = ref.pitch; // Symmetric balls can share the same pitch
      }
      else
      {
         b.x = mSize + DeterministicRandomFloat01(mSeed, i * 4 + 1) * (mArenaW - 2 * mSize);
         b.y = mSize + DeterministicRandomFloat01(mSeed, i * 4 + 2) * (mArenaH - 2 * mSize);
         b.dx = DeterministicRandomFloat01(mSeed, i * 4 + 3) * 2.0f - 1.0f;
         b.dy = DeterministicRandomFloat01(mSeed, i * 4 + 4) * 2.0f - 1.0f;
         float mag = sqrtf(b.dx * b.dx + b.dy * b.dy);
         if (mag > 0.001f)
         {
            b.dx /= mag;
            b.dy /= mag;
         }
         else
         {
            b.dx = 1.0f;
            b.dy = 0.0f;
         }
      }
      mBalls.push_back(b);
   }
}

void BouncingBalls::TriggerNote(int pitch)
{
   if (!IsEnabled())
      return;

   double playTime = NextBufferTime(false);
   PlayNoteOutput(NoteMessage(playTime, pitch, 100));
   PlayNoteOutput(NoteMessage(playTime + 100, pitch, 0));
}

void BouncingBalls::Poll()
{
   if (!IsEnabled())
      return;

   double time = gTime; // gTime is in ms
   if (mLastTime == 0)
      mLastTime = time;
   double dt = (time - mLastTime) / 1000.0;
   mLastTime = time;

   // Clamp dt to avoid huge jumps
   dt = MIN(dt, 0.1);

   for (auto& b : mBalls)
   {
      b.x += b.dx * mSpeed * dt;
      b.y += b.dy * mSpeed * dt;

      bool hit = false;

      // Keep inside bounds
      float maxRadius = mSize;

      if (b.x < maxRadius)
      {
         b.x = maxRadius;
         b.dx *= -1;
         hit = true;
      }
      else if (b.x > mArenaW - maxRadius)
      {
         b.x = mArenaW - maxRadius;
         b.dx *= -1;
         hit = true;
      }

      if (b.y < maxRadius)
      {
         b.y = maxRadius;
         b.dy *= -1;
         hit = true;
      }
      else if (b.y > mArenaH - maxRadius)
      {
         b.y = mArenaH - maxRadius;
         b.dy *= -1;
         hit = true;
      }

      if (hit)
      {
         TriggerNote(b.pitch);
      }
   }
}

void BouncingBalls::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mNumBallsSlider->Draw();
   mSpeedSlider->Draw();
   mSizeSlider->Draw();
   mSymmetryCheckbox->Draw();
   mLowsSlider->Draw();
   mHighsSlider->Draw();

   mSeedEntry->Draw();
   mPrevSeedButton->Draw();
   mReseedButton->Draw();
   mNextSeedButton->Draw();

   // Draw Arena Box
   ofPushStyle();
   ofSetColor(255, 255, 255, 255);
   ofRect(mArenaX, mArenaY, mArenaW, mArenaH, 15);

   // Draw Balls
   ofSetColor(60, 150, 190, 255);
   ofFill();
   for (const auto& b : mBalls)
   {
      ofCircle(mArenaX + b.x, mArenaY + b.y, mSize);
   }
   ofPopStyle();
}


void BouncingBalls::Resize(float w, float h)
{
}

void BouncingBalls::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
   // Optional: if size changes, we might need to clamp balls inside bounds,
   // but Poll() will push them back naturally.
}

void BouncingBalls::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mNumBallsSlider || slider == mLowsSlider || slider == mHighsSlider)
   {
      RespawnBalls();
   }
}

void BouncingBalls::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mSymmetryCheckbox)
   {
      RespawnBalls();
   }
}

void BouncingBalls::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);

   SetUpFromSaveData();
}

void BouncingBalls::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
}

void BouncingBalls::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();
}

void BouncingBalls::LoadState(FileStreamIn& in, int rev)
{
   //note: nothing to read here - the generic module loader already consumes the rev int that
   //SaveState() wrote (via LoadModuleSaveStateRev()) before calling LoadState(). Reading it again
   //here desyncs the file stream and corrupts every module loaded after this one.
}

void BouncingBalls::TextEntryComplete(TextEntry* entry)
{
   if (entry == mSeedEntry)
   {
      RespawnBalls();
   }
}

void BouncingBalls::ButtonClicked(ClickButton* button, double time)
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
         RespawnBalls();
      }
   }
   else if (button == mNextSeedButton)
   {
      if (mSeed < 9999)
      {
         ++mSeed;
         RespawnBalls();
      }
   }
}

void BouncingBalls::Reseed()
{
   mSeed = gRandom() % 10000;
   RespawnBalls();
}
