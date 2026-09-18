#include "MacroXY.h"
#include "OpenFrameworksPort.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Slider.h"
#include <cmath>
#include <set>

MacroXY::MacroXY()
{
}

MacroXY::~MacroXY()
{
   delete mAxisX;
   delete mAxisY;
}

void MacroXY::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mAxisX = new Axis(this, 0, "X");
   mAxisY = new Axis(this, 1, "Y");

   // Position the cable sources to the right of the pad (after the text label)
   mAxisX->GetCableSource()->SetManualPosition(kMarginX + kPadSize + 30, kMarginY + 20);
   mAxisY->GetCableSource()->SetManualPosition(kMarginX + kPadSize + 30, kMarginY + kPadSize - 20);
}

void MacroXY::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();

   // Pad background
   ofSetColor(0, 0, 0, mDragging ? 200 : 150);
   ofFill();
   ofRect(kMarginX, kMarginY, kPadSize, kPadSize);

   // Pad outline
   ofSetColor(100, 100, 100);
   ofNoFill();
   ofSetLineWidth(2);
   ofRect(kMarginX, kMarginY, kPadSize, kPadSize);

   // Crosshairs
   ofSetColor(100, 100, 100, 100);
   ofSetLineWidth(1);
   float cx = kMarginX + mAxisX->mValue * kPadSize;
   float cy = kMarginY + (1.0f - mAxisY->mValue) * kPadSize;
   ofLine(kMarginX, cy, kMarginX + kPadSize, cy);
   ofLine(cx, kMarginY, cx, kMarginY + kPadSize);

   // Cursor
   ofSetColor(255, 255, 255);
   ofFill();
   ofCircle(cx, cy, 5);

   // Labels for axes next to cable sources
   DrawTextNormal("X", kMarginX + kPadSize + 5, kMarginY + 24);
   DrawTextNormal("Y", kMarginX + kPadSize + 5, kMarginY + kPadSize - 16);

   ofPopStyle();
}

void MacroXY::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (right)
      return;

   if (x >= kMarginX && x <= kMarginX + kPadSize && y >= kMarginY && y <= kMarginY + kPadSize)
   {
      mDragging = true;
      MouseMoved(x, y); // update immediately
   }
}

bool MacroXY::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);

   if (mDragging)
   {
      float localX = ofClamp(x - kMarginX, 0, kPadSize);
      float localY = ofClamp(y - kMarginY, 0, kPadSize);

      float valX = localX / (float)kPadSize;
      float valY = 1.0f - (localY / (float)kPadSize); // inverted Y axis for UI

      mAxisX->mValue = valX;
      mAxisY->mValue = valY;

      mAxisX->UpdateControl();
      mAxisY->UpdateControl();

      return true;
   }

   return false;
}

void MacroXY::MouseReleased()
{
   IDrawableModule::MouseReleased();
   mDragging = false;
}

void MacroXY::SaveLayout(ofxJSONElement& moduleInfo)
{
   moduleInfo["x"] = mAxisX->mValue;
   moduleInfo["y"] = mAxisY->mValue;
}

void MacroXY::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();

   if (moduleInfo.isMember("x"))
   {
      mAxisX->mValue = moduleInfo["x"].asFloat();
      mAxisX->UpdateControl();
   }
   if (moduleInfo.isMember("y"))
   {
      mAxisY->mValue = moduleInfo["y"].asFloat();
      mAxisY->UpdateControl();
   }
}

void MacroXY::SetUpFromSaveData()
{
}

void MacroXY::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   Axis* axes[] = { mAxisX, mAxisY };
   for (auto axis : axes)
   {
      if (axis->GetCableSource() != cableSource)
         continue;

      std::set<IUIControl*> usedByOthers;
      for (auto other : axes)
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

      axis->UpdateControl();
      break;
   }
}

void MacroXY::GetModuleDimensions(float& width, float& height)
{
   width = kMarginX + kPadSize + 50;
   height = kMarginY + kPadSize + kMarginY;
}

// ---------------- Axis implementation ----------------

MacroXY::Axis::Axis(MacroXY* owner, int index, const std::string& label)
: mOwner(owner)
, mIndex(index)
, mLabel(label)
{
   mTargetCableSource = new PatchCableSource(mOwner, kConnectionType_Modulator);
   mTargetCableSource->SetModulatorOwner(this);
   mTargetCableSource->SetOverrideCableDir(ofVec2f(1, 0), PatchCableSource::Side::kRight);
   mTargetCableSource->SetDefaultPatchBehavior(kDefaultPatchBehavior_Add);
   mOwner->AddPatchCableSource(mTargetCableSource);
}

MacroXY::Axis::~Axis()
{
   mTargetCableSource->ClearPatchCables();
   mOwner->RemovePatchCableSource(mTargetCableSource);
}

void MacroXY::Axis::UpdateControl()
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
      const auto& cables = mTargetCableSource->GetPatchCables();
      for (size_t i = 0; i < cables.size() && i < mTargets.size(); ++i)
      {
         IUIControl* target = dynamic_cast<IUIControl*>(cables[i]->GetTarget());
         mTargets[i].mUIControlTarget = target;
         mTargets[i].mSliderTarget = dynamic_cast<FloatSlider*>(target);
      }
   }

   TheSynth->RemoveExtraPoller(this);
   TheSynth->AddExtraPoller(this);
}

float MacroXY::Axis::Value(int samplesIn)
{
   return mValue;
}

void MacroXY::Axis::Poll()
{
   if (Active())
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
