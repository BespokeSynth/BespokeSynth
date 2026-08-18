#include "MacroBars.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include <cmath>
#include <set>

MacroBars::MacroBars()
{
}

MacroBars::~MacroBars()
{
   for (auto fader : mFaders)
      delete fader;
}

void MacroBars::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mNumFadersSlider = new IntSlider(this, "count", kMarginX, kMarginY - 20, 100, 15, &mNumFaders, 1, kMaxFaders);

   for (int i = 0; i < kMaxFaders; ++i)
   {
      Fader* fader = new Fader(this, i);
      mFaders.push_back(fader);

      float cx = kMarginX + i * (kFaderWidth + kFaderSpacing);
      float cy = kMarginY + 20 + kFaderHeight + 15;

      fader->GetCableSource()->SetManualPosition(cx + kFaderWidth / 2.0f, cy);
   }
}

void MacroBars::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < kMaxFaders; ++i)
   {
      mFaders[i]->GetCableSource()->SetEnabled(i < mNumFaders);
   }

   mNumFadersSlider->Draw();

   for (int i = 0; i < mNumFaders; ++i)
   {
      float cx = kMarginX + i * (kFaderWidth + kFaderSpacing);
      float cy = kMarginY + 20;

      ofPushStyle();

      // Fader track background
      ofSetColor(0, 0, 0, 150);
      ofFill();
      ofRect(cx, cy, kFaderWidth, kFaderHeight);

      // Fader outline
      ofSetColor(100, 100, 100);
      ofNoFill();
      ofSetLineWidth(2);
      ofRect(cx, cy, kFaderWidth, kFaderHeight);

      // Hover overlay
      if (mHoveredFader == i || mDraggingFader == i)
      {
         ofSetColor(255, 255, 255, 30);
         ofFill();
         ofRect(cx, cy, kFaderWidth, kFaderHeight);
      }

      // Value fill (from bottom to top)
      ofSetColor(mDraggingFader == i ? 200 : 150, mDraggingFader == i ? 200 : 150, 200);
      ofFill();
      float fillHeight = mFaders[i]->mValue * kFaderHeight;
      ofRect(cx, cy + kFaderHeight - fillHeight, kFaderWidth, fillHeight);

      // Fader thumb
      ofSetColor(255, 255, 255);
      ofRect(cx - 2, cy + kFaderHeight - fillHeight - 4, kFaderWidth + 4, 8);

      ofPopStyle();
   }
}

void MacroBars::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   for (int i = 0; i < mNumFaders; ++i)
   {
      float cx = kMarginX + i * (kFaderWidth + kFaderSpacing);
      float cy = kMarginY + 20;

      if (x >= cx && x <= cx + kFaderWidth && y >= cy && y <= cy + kFaderHeight)
      {
         mDraggingFader = i;
         MouseMoved(x, y); // update immediately
         break;
      }
   }
}

bool MacroBars::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   mHoveredFader = -1;
   for (int i = 0; i < mNumFaders; ++i)
   {
      float cx = kMarginX + i * (kFaderWidth + kFaderSpacing);
      float cy = kMarginY + 20;

      if (x >= cx && x <= cx + kFaderWidth && y >= cy && y <= cy + kFaderHeight)
      {
         mHoveredFader = i;
         break;
      }
   }

   if (mDraggingFader >= 0)
   {
      float cy = kMarginY + 20;
      float localY = ofClamp(y - cy, 0, kFaderHeight);
      float val = 1.0f - (localY / (float)kFaderHeight); // inverted Y axis for UI

      mFaders[mDraggingFader]->mValue = val;
      mFaders[mDraggingFader]->UpdateControl();

      return true;
   }

   return false;
}

void MacroBars::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mDraggingFader = -1;
}

void MacroBars::SaveLayout(ofxJSONElement& moduleInfo)
{
   for (int i = 0; i < kMaxFaders; ++i)
   {
      moduleInfo["fader" + ofToString(i)] = mFaders[i]->mValue;
   }
}

void MacroBars::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();

   for (int i = 0; i < kMaxFaders; ++i)
   {
      if (moduleInfo.isMember("fader" + ofToString(i)))
      {
         mFaders[i]->mValue = moduleInfo["fader" + ofToString(i)].asFloat();
         mFaders[i]->UpdateControl();
      }
   }
}

void MacroBars::SetUpFromSaveData()
{
}

void MacroBars::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   for (auto fader : mFaders)
   {
      if (fader->GetCableSource() != cableSource)
         continue;

      std::set<IUIControl*> usedByOthers;
      for (auto other : mFaders)
      {
         if (other->GetCableSource() == cableSource)
            continue;
         for (auto* cable : other->GetCableSource()->GetPatchCables())
         {
            IUIControl* t = dynamic_cast<IUIControl*>(cable->GetTarget());
            if (t != nullptr)
               usedByOthers.insert(t);
         }
      }

      auto& cables = cableSource->GetPatchCables();
      for (int i = (int)cables.size() - 1; i >= 0; --i)
      {
         IUIControl* t = dynamic_cast<IUIControl*>(cables[i]->GetTarget());
         if (t != nullptr && usedByOthers.count(t))
         {
            cableSource->RemovePatchCable(cables[i]);
            i = (int)cableSource->GetPatchCables().size();
         }
      }

      fader->UpdateControl();
      break;
   }
}

void MacroBars::GetModuleDimensions(float& width, float& height)
{
   width = kMarginX + mNumFaders * (kFaderWidth + kFaderSpacing) + kMarginX;
   width = MAX(width, 110.0f); // Make sure it's wide enough for the "count" slider
   height = kMarginY + 20 + kFaderHeight + 40;
}

// ---------------- Fader implementation ----------------

MacroBars::Fader::Fader(MacroBars* owner, int index)
: mOwner(owner)
, mIndex(index)
{
   mTargetCableSource = new PatchCableSource(mOwner, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   mTargetCableSource->SetOverrideCableDir(ofVec2f(0, 1), PatchCableSource::Side::kBottom);
   mTargetCableSource->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
   mOwner->AddPatchCableSource(mTargetCableSource);
}

MacroBars::Fader::~Fader()
{
   mTargetCableSource->ClearPatchCables();
   mOwner->RemovePatchCableSource(mTargetCableSource);
}

void MacroBars::Fader::UpdateControl()
{
   for (auto& t : mTargets)
   {
      if (t.mSliderTarget != nullptr && t.mSliderTarget->GetModulator() == this)
         t.mSliderTarget->SetModulator(nullptr);
      t.mUIControlTarget = nullptr;
      t.mSliderTarget = nullptr;
   }

   if (mTargetCableSource != nullptr)
   {
      // Only process patch cables if this fader is currently active/visible
      if (mIndex < mOwner->mNumFaders)
      {
         const auto& cables = mTargetCableSource->GetPatchCables();
         for (size_t i = 0; i < cables.size() && i < mTargets.size(); ++i)
         {
            IUIControl* target = dynamic_cast<IUIControl*>(cables[i]->GetTarget());
            mTargets[i].mUIControlTarget = target;
            mTargets[i].mSliderTarget = dynamic_cast<FloatSlider*>(target);
         }
      }
   }

   TheSynth->RemoveExtraPoller(this);
   TheSynth->AddExtraPoller(this);
}

float MacroBars::Fader::Value(int samplesIn)
{
   return mValue;
}

void MacroBars::Fader::Poll()
{
   if (Active() && mIndex < mOwner->mNumFaders)
   {
      mLastPollValue = mValue;
      const float kBlendRate = -9.65784f;
      float blend = exp2(kBlendRate / ofGetFrameRate());
      mSmoothedValue = mSmoothedValue * blend + mLastPollValue * (1 - blend);

      for (int i = 0; i < GetNumTargets(); ++i)
      {
         IUIControl* target = mTargets[i].mUIControlTarget;
         if (target == nullptr)
            continue;

         target->SetFromMidiCC(mValue, NextBufferTime(false), SetValueMethod::Modulator);
      }
   }
}
